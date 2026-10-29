#include "model.hpp"

#include "xdyn/observers_and_api/ConfBuilder.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <map>
#include <random>
#include <set>
#include <sstream>

namespace
{
    const char* const state_names[13]
        = { "x", "y", "z", "u", "v", "w", "p", "q", "r", "qr", "qi", "qj", "qk" };
    const char* const state_docs[13] = { "position along north (NED)",
                                         "position along east (NED)",
                                         "position down (NED)",
                                         "surge velocity (body, FRD)",
                                         "sway velocity (body, FRD)",
                                         "heave velocity (body, FRD)",
                                         "roll rate (body)",
                                         "pitch rate (body)",
                                         "yaw rate (body)",
                                         "quaternion, real part",
                                         "quaternion, i",
                                         "quaternion, j",
                                         "quaternion, k" };
    const char* const state_units[13]
        = { "m", "m", "m", "m/s", "m/s", "m/s", "rad/s", "rad/s", "rad/s", "", "", "", "" };
    const char* const state_display_units[13]
        = { "", "", "", "", "", "", "deg/s", "deg/s", "deg/s", "", "", "", "" };

    struct Unit
    {
        const char* unit;
        const char* display_unit;
    };

    // The commands force models declare, in SI units. The YAML's own `unit:` is not reliable
    // enough to go by, and a maneuvering model's commands are whatever its author calls them:
    // those get no unit.
    Unit command_unit(const std::string& command)
    {
        static const std::map<std::string, Unit> units
            = { { "rpm", { "rad/s", "rpm" } }, { "P/D", { "", "" } },
                { "beta", { "rad", "deg" } },  { "psi_co", { "rad", "deg" } },
                { "x_co", { "m", "" } },       { "y_co", { "m", "" } } };
        const auto it = units.find(command);
        return it == units.end() ? Unit{ "", "" } : it->second;
    }

    // One part of a structured name: what is not [A-Za-z0-9_] becomes _, and a part cannot start
    // with a digit.
    std::string name_part(const std::string& text)
    {
        std::string out;
        for (const char c : text)
            out += std::isalnum(static_cast<unsigned char>(c)) or c == '_' ? c : '_';
        if (out.empty() or std::isdigit(static_cast<unsigned char>(out[0])))
            out = "_" + out;
        return out;
    }

    std::string escaped(const std::string& text)
    {
        std::string out;
        for (const char c : text)
        {
            switch (c)
            {
                case '&':
                    out += "&amp;";
                    break;
                case '<':
                    out += "&lt;";
                    break;
                case '>':
                    out += "&gt;";
                    break;
                case '"':
                    out += "&quot;";
                    break;
                default:
                    out += c;
            }
        }
        return out;
    }

    // Enough digits to read back as the same double.
    std::string number(const double value)
    {
        std::ostringstream s;
        s << std::setprecision(17) << value;
        return s.str();
    }

    // An importer checks it against the one it passes to fmi2Instantiate: unique per FMU built.
    std::string random_guid()
    {
        std::random_device device;
        std::mt19937_64 random((static_cast<std::uint64_t>(device()) << 32)
                               ^ static_cast<std::uint64_t>(device()));
        std::ostringstream s;
        s << std::hex << std::setfill('0') << '{' << std::setw(8) << (random() & 0xffffffff) << '-'
          << std::setw(4) << (random() & 0xffff) << '-' << std::setw(4)
          << ((random() & 0x0fff) | 0x4000) << '-' << std::setw(4) << ((random() & 0x3fff) | 0x8000)
          << '-' << std::setw(12) << (random() & 0xffffffffffff) << '}';
        return s.str();
    }

    struct Variables
    {
        std::ostringstream xml;
        std::vector<size_t> outputs; // 1-based indices, for <ModelStructure>
        std::set<std::string> names;
        size_t count = 0;

        void real(const std::string& name, const unsigned vr, const std::string& causality,
                  const std::string& variability, const std::string& initial, const double start,
                  const std::string& unit, const std::string& display_unit,
                  const std::string& description)
        {
            std::ostringstream type;
            type << "Real start=\"" << number(start) << '"';
            if (not(unit.empty()))
                type << " unit=\"" << unit << '"';
            if (not(display_unit.empty()))
                type << " displayUnit=\"" << display_unit << '"';
            add(name, vr, causality, variability, initial, type.str(), description);
        }

        void add(const std::string& name, const unsigned vr, const std::string& causality,
                 const std::string& variability, const std::string& initial,
                 const std::string& type, const std::string& description)
        {
            if (not(names.insert(name).second))
                throw std::invalid_argument("two variables would be called " + name
                                            + ": rename a body, force model or command");
            ++count;
            if (causality == "output")
                outputs.push_back(count);
            xml << "    <ScalarVariable name=\"" << escaped(name) << "\" valueReference=\"" << vr
                << "\" causality=\"" << causality << "\" variability=\"" << variability << '"';
            if (not(initial.empty()))
                xml << " initial=\"" << initial << '"';
            xml << " description=\"" << escaped(description) << "\">\n      <" << type
                << "/>\n    </ScalarVariable>\n";
        }
    };
}

const std::vector<std::string>& xdyn_fmu::solvers()
{
    static const std::vector<std::string> names = { "euler", "rk4", "rkck" };
    return names;
}

std::mutex& xdyn_fmu::assets_root_mutex()
{
    static std::mutex mutex;
    return mutex;
}

xdyn_fmu::Model xdyn_fmu::read_model(const std::string& yaml, const std::string& assets_root)
{
    ConfBuilder builder = with_assets_root(assets_root, [&] { return ConfBuilder(yaml); });
    Model model;
    model.body = builder.sim.get_bodies().at(0)->get_name();
    // As the YAML writes it: xdyn turns [psi, theta', phi''] into axes, ordered by angle.
    const YamlRotation& rot = builder.sim.get_env().rot;
    const std::map<char, std::string> angles = { { 'x', "phi" }, { 'y', "theta" }, { 'z', "psi" } };
    for (size_t i = 0; i < rot.convention.size(); ++i)
    {
        std::string item = rot.convention[i];
        if (rot.order_by == "angle" and not(item.empty()) and angles.count(item[0]))
            item = angles.at(item[0]) + item.substr(1);
        model.rotations += (i ? ", " : "[") + item;
    }
    model.rotations += "]";
    for (const auto& body_forces : builder.sim.get_forces())
        for (const auto& force : body_forces.second)
            for (const auto& command : force->get_command_names())
            {
                if (command.empty())
                    continue;
                Command c;
                c.xdyn_name = force->get_name() + "(" + command + ")";
                c.name = name_part(body_forces.first) + "." + name_part(force->get_name()) + "."
                         + name_part(command);
                // A command the YAML declares without a value: the master sets it before the
                // first step.
                try
                {
                    c.value = builder.sim.get_input_value(c.xdyn_name);
                }
                catch (const std::exception&)
                {
                }
                model.commands.push_back(c);
            }
    for (size_t i = 0; i < 13; ++i)
        model.state[i] = builder.sim.state.at(i);
    return model;
}

std::string xdyn_fmu::model_description(const Model& model, const std::string& model_name,
                                        const double dt, const std::string& solver)
{
    const std::string body = name_part(model.body);
    const auto solver_index = std::find(solvers().begin(), solvers().end(), solver);
    if (solver_index == solvers().end())
        throw std::invalid_argument("unknown solver '" + solver + "'");
    Variables v;
    for (unsigned i = 0; i < 13; ++i)
        v.real(body + "." + state_names[i], VR_STATE + i, "output", "continuous", "exact",
               model.state[i], state_units[i], state_display_units[i],
               std::string(state_names[i]) + "(" + model.body + "): " + state_docs[i]);
    const char* const euler[3] = { "phi", "theta", "psi" };
    const char* const euler_docs[3] = { "roll", "pitch", "heading" };
    for (unsigned i = 0; i < 3; ++i)
        v.real(body + "." + euler[i], VR_EULER + i, "output", "continuous", "exact", 0, "rad",
               "deg",
               std::string(euler[i]) + "(" + model.body + "): " + euler_docs[i]
                   + ", Euler angle in the rotations convention " + model.rotations);
    v.real("xdyn.dt", VR_DT, "parameter", "fixed", "exact", dt, "s", "",
           "xdyn's integration step inside a communication step");
    v.add("xdyn.solver", VR_SOLVER, "parameter", "fixed", "exact",
          "Enumeration declaredType=\"xdyn.Solver\" start=\""
              + std::to_string(solver_index - solvers().begin() + 1) + '"',
          "xdyn's solver");
    v.add(body + ".reset_state", VR_RESET, "input", "discrete", "", "Boolean start=\"false\"",
          "Going from false to true makes the next step start from " + body + ".reset_x ... " + body
              + ".reset_qk instead of the current state");
    for (unsigned i = 0; i < 13; ++i)
        v.real(body + ".reset_" + state_names[i], VR_RESET_STATE + i, "input", "discrete", "",
               model.state[i], state_units[i], state_display_units[i],
               std::string("state imposed by reset_state: ") + state_docs[i]);
    for (unsigned i = 0; i < model.commands.size(); ++i)
    {
        const Command& c = model.commands[i];
        const std::string command = c.xdyn_name.substr(c.xdyn_name.rfind('(') + 1);
        const Unit unit = command_unit(command.substr(0, command.size() - 1));
        v.real(c.name, VR_COMMAND + i, "input", "continuous", "", c.value, unit.unit,
               unit.display_unit, c.xdyn_name);
    }

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<fmiModelDescription fmiVersion=\"2.0\" modelName=\"" << escaped(model_name)
        << "\" guid=\"" << random_guid() << "\"\n"
        << "  description=\"xdyn ship dynamics as a Co-Simulation FMU\" generationTool=\"xdyn\"\n"
        << "  variableNamingConvention=\"structured\" numberOfEventIndicators=\"0\">\n"
        << "  <CoSimulation modelIdentifier=\"xdyn_fmu\" "
           "canHandleVariableCommunicationStepSize=\"true\"\n"
        << "    canInterpolateInputs=\"false\" maxOutputDerivativeOrder=\"0\" "
           "canRunAsynchronuously=\"false\"\n"
        << "    canBeInstantiatedOnlyOncePerProcess=\"false\" "
           "canNotUseMemoryManagementFunctions=\"true\"\n"
        << "    canGetAndSetFMUstate=\"false\" canSerializeFMUstate=\"false\" "
           "providesDirectionalDerivative=\"false\"/>\n"
        << "  <UnitDefinitions>\n"
        << "    <Unit name=\"m\"><BaseUnit m=\"1\"/></Unit>\n"
        << "    <Unit name=\"m/s\"><BaseUnit m=\"1\" s=\"-1\"/></Unit>\n"
        << "    <Unit name=\"rad\"><BaseUnit rad=\"1\"/>\n"
        << "      <DisplayUnit name=\"deg\" factor=\"57.295779513082323\"/></Unit>\n"
        << "    <Unit name=\"rad/s\"><BaseUnit s=\"-1\" rad=\"1\"/>\n"
        << "      <DisplayUnit name=\"deg/s\" factor=\"57.295779513082323\"/>\n"
        << "      <DisplayUnit name=\"rpm\" factor=\"9.5492965855137202\"/></Unit>\n"
        << "    <Unit name=\"s\"><BaseUnit s=\"1\"/></Unit>\n"
        << "  </UnitDefinitions>\n"
        << "  <TypeDefinitions>\n"
        << "    <SimpleType name=\"xdyn.Solver\">\n      <Enumeration>\n";
    for (size_t k = 0; k < solvers().size(); ++k)
        xml << "        <Item name=\"" << solvers()[k] << "\" value=\"" << k + 1 << "\"/>\n";
    xml << "      </Enumeration>\n    </SimpleType>\n  </TypeDefinitions>\n"
        << "  <DefaultExperiment startTime=\"0\" stepSize=\"" << number(dt) << "\"/>\n"
        << "  <ModelVariables>\n"
        << v.xml.str() << "  </ModelVariables>\n"
        << "  <ModelStructure>\n    <Outputs>\n";
    // The outputs are the states at the end of a step: no input reaches them within the step.
    for (const size_t index : v.outputs)
        xml << "      <Unknown index=\"" << index << "\" dependencies=\"\"/>\n";
    xml << "    </Outputs>\n  </ModelStructure>\n</fmiModelDescription>\n";
    return xml.str();
}
