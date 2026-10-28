#include "model.hpp"

#include "xdyn/observers_and_api/ConfBuilder.hpp"

#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>

namespace
{
    const char* const state_names[13]
        = { "x", "y", "z", "u", "v", "w", "p", "q", "r", "qr", "qi", "qj", "qk" };
    const char* const state_docs[13] = { "position along north (NED), m",
                                         "position along east (NED), m",
                                         "position down (NED), m",
                                         "surge velocity (body, FRD), m/s",
                                         "sway velocity (body, FRD), m/s",
                                         "heave velocity (body, FRD), m/s",
                                         "roll rate (body), rad/s",
                                         "pitch rate (body), rad/s",
                                         "yaw rate (body), rad/s",
                                         "quaternion, real part",
                                         "quaternion, i",
                                         "quaternion, j",
                                         "quaternion, k" };
    const char* const state_units[13]
        = { "m", "m", "m", "m/s", "m/s", "m/s", "rad/s", "rad/s", "rad/s", "", "", "", "" };

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
        size_t count = 0;

        void real(const std::string& name, const unsigned vr, const std::string& causality,
                  const std::string& variability, const std::string& initial, const double start,
                  const std::string& unit, const std::string& description)
        {
            ++count;
            if (causality == "output")
                outputs.push_back(count);
            xml << "    <ScalarVariable name=\"" << escaped(name) << "\" valueReference=\"" << vr
                << "\" causality=\"" << causality << "\" variability=\"" << variability << '"';
            if (not(initial.empty()))
                xml << " initial=\"" << initial << '"';
            xml << " description=\"" << escaped(description) << "\">\n      <Real start=\""
                << number(start) << '"';
            if (not(unit.empty()))
                xml << " unit=\"" << unit << '"';
            xml << "/>\n    </ScalarVariable>\n";
        }

        void other(const std::string& name, const unsigned vr, const std::string& causality,
                   const std::string& variability, const std::string& initial,
                   const std::string& type, const std::string& start,
                   const std::string& description)
        {
            ++count;
            xml << "    <ScalarVariable name=\"" << name << "\" valueReference=\"" << vr
                << "\" causality=\"" << causality << "\" variability=\"" << variability << '"';
            if (not(initial.empty()))
                xml << " initial=\"" << initial << '"';
            xml << " description=\"" << escaped(description) << "\">\n      <" << type
                << " start=\"" << escaped(start) << "\"/>\n    </ScalarVariable>\n";
        }
    };
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
    model.commands = builder.sim.get_command_names();
    for (const auto& name : model.commands)
    {
        double value = 0;
        // A command the YAML declares without a value: the master sets it before the first step.
        try
        {
            value = builder.sim.get_input_value(name);
        }
        catch (const std::exception&)
        {
        }
        model.command_values.push_back(value);
    }
    for (size_t i = 0; i < 13; ++i)
        model.state[i] = builder.sim.state.at(i);
    return model;
}

std::string xdyn_fmu::model_description(const Model& model, const std::string& model_name,
                                        const double dt, const std::string& solver)
{
    Variables v;
    for (unsigned i = 0; i < 13; ++i)
        v.real(state_names[i], VR_STATE + i, "output", "continuous", "exact", model.state[i],
               state_units[i], model.body + ": " + state_docs[i]);
    const char* const euler[3] = { "phi", "theta", "psi" };
    const char* const euler_docs[3] = { "roll", "pitch", "heading" };
    for (unsigned i = 0; i < 3; ++i)
        v.real(euler[i], VR_EULER + i, "output", "continuous", "exact", 0, "rad",
               model.body + ": " + euler_docs[i] + ", rad");
    v.real("xdyn_dt", VR_DT, "parameter", "fixed", "exact", dt, "s",
           "xdyn's integration step inside a communication step, s");
    v.other("solver", VR_SOLVER, "parameter", "fixed", "exact", "String", solver,
            "xdyn's solver: euler, rk4 or rkck");
    v.other("reset_state", VR_RESET, "input", "discrete", "", "Boolean", "false",
            "When true, the next step starts from reset_x ... reset_qk instead of the current "
            "state");
    for (unsigned i = 0; i < 13; ++i)
        v.real(std::string("reset_") + state_names[i], VR_RESET_STATE + i, "input", "discrete", "",
               model.state[i], state_units[i],
               std::string("state imposed by reset_state: ") + state_docs[i]);
    for (unsigned i = 0; i < model.commands.size(); ++i)
        v.real(model.commands[i], VR_COMMAND + i, "input", "continuous", "",
               model.command_values[i], "", "xdyn command (SI units; rpm in rad/s)");

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<fmiModelDescription fmiVersion=\"2.0\" modelName=\"" << escaped(model_name)
        << "\" guid=\"" << random_guid() << "\"\n"
        << "  description=\"xdyn ship dynamics as a Co-Simulation FMU\" generationTool=\"xdyn\"\n"
        << "  variableNamingConvention=\"flat\" numberOfEventIndicators=\"0\">\n"
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
        << "    <Unit name=\"rad\"><BaseUnit rad=\"1\"/></Unit>\n"
        << "    <Unit name=\"rad/s\"><BaseUnit s=\"-1\" rad=\"1\"/></Unit>\n"
        << "    <Unit name=\"s\"><BaseUnit s=\"1\"/></Unit>\n"
        << "  </UnitDefinitions>\n"
        << "  <DefaultExperiment startTime=\"0\" stepSize=\"" << number(dt) << "\"/>\n"
        << "  <ModelVariables>\n"
        << v.xml.str() << "  </ModelVariables>\n"
        << "  <ModelStructure>\n    <Outputs>\n";
    for (const size_t index : v.outputs)
        xml << "      <Unknown index=\"" << index << "\"/>\n";
    xml << "    </Outputs>\n  </ModelStructure>\n</fmiModelDescription>\n";
    return xml.str();
}
