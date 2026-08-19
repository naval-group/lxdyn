/*
 * external_data_structures_parsers.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: cady
 */

#include "external_data_structures_parsers.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/yaml_parser/parse_unit_value.hpp"
#include "yaml_compat.h"
#include <iostream>

size_t try_to_parse_positive_integer(const YAML::Node& node, const std::string& key)
{
    int x;
    try
    {
        node[key] >> x;
    }
    catch(YAML::Exception& e)
    {
        std::stringstream ss;
        ss << "Error trying to parse key '" << key << "' (" << e.msg << ")";
        throw YAML::Exception(node.Mark(), ss.str());
    }
    if (x < 0)
    {
        std::stringstream ss;
        ss << "Expected a positive integer for key '" << key << "', but got " << x;
        throw YAML::Exception(node.Mark(), ss.str());
    }
    return (size_t)x;
}

void parse_point_with_name(const YAML::Node& node, YamlPoint& p, const std::string& name)
{
    YamlCoordinates c;
    node >> c;
    p = c;
    node["frame"] >> p.frame;
    p.name = name;
}

void inline search_and_replace(std::string& source, const std::string& find, const std::string& replace)
{
    const size_t fLen = find.size();
    const size_t rLen = replace.size();
    for (size_t pos=0; (pos=source.find(find, pos))!=std::string::npos; pos+=rLen)
    {
        source.replace(pos, fLen, replace);
    }
}

void operator >> (const YAML::Node& node, YamlRotation& g)
{
    std::vector<std::string> s;
    node >> s;
    if (s.size() != 3)
    {
        std::stringstream ss;
        ss << "Could not parse rotations: there should be exactly three elements in the 'rotations convention' list, but " << s.size() << " were detected.";
        THROW(__PRETTY_FUNCTION__, InvalidInputException, ss.str());
    }
    bool detected_axis = false;
    bool detected_angle = false;
    std::vector<std::string> ret;
    for (size_t i = 0 ; i < 3 ; ++i)
    {
        std::string si = s.at(i);
        if (si.size() > 7)
        {
            std::stringstream ss;
            ss << "Could not parse rotations: element nb " << i << " (starting from 0) in vector was " << si;
            THROW(__PRETTY_FUNCTION__, InvalidInputException, ss.str());
        }
        const std::string si_with_no_apostrophs = si.substr(0,si.find_first_of('\''));
        if ((si_with_no_apostrophs == "x") or (si_with_no_apostrophs == "y") or (si_with_no_apostrophs == "z"))
        {
            detected_axis = true;
        }
        if ((si_with_no_apostrophs == "phi") or (si_with_no_apostrophs == "theta") or (si_with_no_apostrophs == "psi"))
        {
            detected_angle = true;
            search_and_replace(si, "phi", "x");
            search_and_replace(si, "theta", "y");
            search_and_replace(si, "psi", "z");
        }
        if (not(detected_axis or detected_angle))
        {
            std::stringstream ss;
            ss << "Could not parse rotations: could not detect convention for element nb " << i << " (starting from 0) in vector, which was " << si;
            THROW(__PRETTY_FUNCTION__, InvalidInputException, ss.str());
        }
        g.convention.push_back(si);
    }
    if (detected_axis) g.order_by = "axis";
    if (detected_angle) g.order_by = "angle";
}

void operator >> (const YAML::Node& node, YamlBody& b)
{
    node["name"] >> b.name;
    try_to_parse(node, "mesh", b.mesh);
    try_to_parse(node, "external forces", b.external_forces);
    // operator>>(YAML::Node, std::vector<T>) clears the vector before parsing, so we cannot just do try_to_parse(node, "controlled forces", b.external_forces)
    std::vector<YamlModel> controlled_forces;
    try_to_parse(node, "controlled forces", controlled_forces);
    b.external_forces.insert(b.external_forces.end(), controlled_forces.begin(), controlled_forces.end());
    try
    {
        node["position of body frame relative to mesh"]             >> b.position_of_body_frame_relative_to_mesh;
        node["initial position of body frame relative to NED"]      >> b.initial_position_of_body_frame_relative_to_NED_projected_in_NED;
        node["initial velocity of body frame relative to NED"]      >> b.initial_velocity_of_body_frame_relative_to_NED_projected_in_body;
        node["dynamics"]                                            >> b.dynamics;
        if (node["filtered states"])
        {
            node["filtered states"] >> b.filtered_states;
        }
    }
    catch (const InvalidInputException& e)
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Error parsing 'body' section (body '" << b.name << "'): " << e.get_message());
    }
    try_to_parse(node, "blocked dof", b.blocked_dof);
}

void operator >> (const YAML::Node& node, YamlModel& m)
{
    node["model"] >> m.model;
    YAML::Emitter out;
    out << node;
    m.yaml = out.c_str();
    const int i = node.Mark().line;
    m.index_of_first_line_in_global_yaml = i > 0 ? 1+(size_t)i : 0;
}

void operator >> (const YAML::Node& node, YamlPosition& p)
{
    node >> p.angle;
    node >> p.coordinates;
    node["frame"] >> p.frame;
}

void operator >> (const YAML::Node& node, YamlAngle& a)
{
    xdyn::yaml_parser::parse_uv(node["phi"], a.phi);
    xdyn::yaml_parser::parse_uv(node["theta"], a.theta);
    xdyn::yaml_parser::parse_uv(node["psi"], a.psi);
}

void operator >> (const YAML::Node& node, YamlCoordinates& c)
{
    xdyn::yaml_parser::parse_uv(node["x"], c.x);
    xdyn::yaml_parser::parse_uv(node["y"], c.y);
    xdyn::yaml_parser::parse_uv(node["z"], c.z);
}

void operator >> (const YAML::Node& node, YamlSpeed& s)
{
    node["frame"] >> s.frame;
    xdyn::yaml_parser::parse_uv(node["u"], s.u);
    xdyn::yaml_parser::parse_uv(node["v"], s.v);
    xdyn::yaml_parser::parse_uv(node["w"], s.w);
    xdyn::yaml_parser::parse_uv(node["p"], s.p);
    xdyn::yaml_parser::parse_uv(node["q"], s.q);
    xdyn::yaml_parser::parse_uv(node["r"], s.r);
}

void operator >> (const YAML::Node& node, YamlDynamics& d)
{
    parse_point_with_name(node["centre of inertia"], d.centre_of_inertia, "centre of inertia");
    const std::string rb_buoyancy("rigid body inertia matrix at the center of buoyancy projected in the body frame");
    const std::string rb_gravity("rigid body inertia matrix at the center of gravity and projected in the body frame");
    const std::string am_buoyancy("added mass matrix at the center of buoyancy projected in the body frame");
    const std::string am_gravity("added mass matrix at the center of gravity and projected in the body frame");
    if (node[rb_buoyancy] && node[rb_gravity])
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "One can not define both keys '" << rb_buoyancy <<"' and '" << rb_gravity << "'.");
    }
    if (node[am_buoyancy] && node[am_gravity])
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "One can not define both keys '" << am_buoyancy <<"' and '" << am_gravity << "'.");
    }
    if (node[rb_gravity])
    {
        try
        {
            parse_YamlDynamics6x6Matrix(node[rb_gravity], d.rigid_body_inertia, false, d.centre_of_inertia.frame);
        }
        catch(const InvalidInputException& e)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "In node '" << rb_gravity << "': " << e.get_message());
        }
    }
    else
    {
        try
        {
            parse_YamlDynamics6x6Matrix(node[rb_buoyancy], d.rigid_body_inertia, false, d.centre_of_inertia.frame);
        }
        catch(const InvalidInputException& e)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "In node '" << rb_buoyancy << "': " << e.get_message());
        }
        std::cout << "Using key '" << rb_buoyancy << "' is not recommended, as it is considered as the matrix expressed at center of gravity." << std::endl;
        std::cout << "Please replace key '" << rb_buoyancy << "' with key '" << rb_gravity << "'." <<std::endl;
    }
    if (node[am_gravity])
    {
        try
        {
            parse_YamlDynamics6x6Matrix(node[am_gravity], d.added_mass, false, d.centre_of_inertia.frame);
        }
        catch(const InvalidInputException& e)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "In node '" << am_gravity << "': " << e.get_message());
        }
    }
    else
    {
        try
        {
            parse_YamlDynamics6x6Matrix(node[am_buoyancy], d.added_mass, false, d.centre_of_inertia.frame);
        }
        catch(const InvalidInputException& e)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "In node '" << am_buoyancy << "': " << e.get_message());
        }
        std::cout << "Using key '" << am_buoyancy << "' is not recommended, as it is considered as the matrix expressed at center of gravity." << std::endl;
        std::cout << "Please replace key '" << am_buoyancy << "' with key '" << am_gravity << "'." <<std::endl;
    }
    node["hydrodynamic forces calculation point in body frame"] >> d.hydrodynamic_forces_calculation_point_in_body_frame;
}

void operator >> (const YAML::Node& node, YamlPoint& p)
{
    std::string name;
    node["name"] >> name;
    parse_point_with_name(node, p,name);
}

void parse_YamlDynamics6x6Matrix(const YAML::Node& node, YamlDynamics6x6Matrix& m, const bool parse_frame, const std::string& frame_name)
{
    if (node["from precal"])
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "You used YAML key 'from precal' but this key is deprecated: you should use 'from hdb' instead.");
    }
    if (node["from hdb"])
    {
        if (node["row 1"] || node["row 2"] || node["row 3"]
            || node["row 4"] || node["row 5"] || node["row 6"]
            || node["convention z down"])
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException,
                    "cannot specify both an HDB filename & a matrix (both keys 'from hdb' and "
                    "one of 'row 1', 'row 2', 'row 3', 'row 4', 'row 5' or 'row 6' were found in "
                    "the YAML file).");
        }
        if (node["from raodb"])
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException,
                    "cannot specify both an HDB filename & a PRECAL_R filename (both keys 'from "
                    "hdb' and 'from raodb' were found in the YAML file).");
        }
        m.read_from_file = true;
        node["from hdb"] >> m.hdb_filename;
    }
    else if (node["from raodb"])
    {
        m.read_from_file = true;
        node["from raodb"] >> m.precal_filename;
    }
    else
    {
        try
        {
            if (parse_frame)
            {
                node["frame"] >> m.frame;
            }
            else
            {
                m.frame = frame_name;
            }
            m.row_convention_xdyn_with_z_down = true;
            if (node["convention z down"])
            {
                node["convention z down"] >> m.row_convention_xdyn_with_z_down;
            }
            node["row 1"] >> m.row_1;
            node["row 2"] >> m.row_2;
            node["row 3"] >> m.row_3;
            node["row 4"] >> m.row_4;
            node["row 5"] >> m.row_5;
            node["row 6"] >> m.row_6;
        }
        catch (const YAML::Exception& e)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException,
                    "Unable to parse matrix in the YAML file. The problem was: '"
                        << e.msg << "' and it was detected line " << e.mark.line + 1 << " column "
                        << e.mark.column + 1 << " of the concatenated YAML file.");
        }
    }
}

void operator >> (const YAML::Node& node, YamlEnvironmentalConstants& f)
{
    xdyn::yaml_parser::parse_uv(node["g"], f.g);
    xdyn::yaml_parser::parse_uv(node["rho"], f.rho);
    if(node["air rho"])
    {
        double rho_air;
        xdyn::yaml_parser::parse_uv(node["air rho"], rho_air);
        f.rho_air = rho_air;
    }
    xdyn::yaml_parser::parse_uv(node["nu"], f.nu);
}

void operator >> (const YAML::Node& node, BlockableState& g);
void operator >> (const YAML::Node& node, BlockableState& g)
{
    std::string t;
    node >> t;
    if      (t == "u") g = BlockableState::U;
    else if (t == "v") g = BlockableState::V;
    else if (t == "w") g = BlockableState::W;
    else if (t == "p") g = BlockableState::P;
    else if (t == "q") g = BlockableState::Q;
    else if (t == "r") g = BlockableState::R;
    else if (t == "x") g = BlockableState::X;
    else if (t == "y") g = BlockableState::Y;
    else if (t == "z") g = BlockableState::Z;
    else if (t == "phi") g = BlockableState::PHI;
    else if (t == "theta") g = BlockableState::THETA;
    else if (t == "psi") g = BlockableState::PSI;
    else
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Unrecognized state: '" << t << "'. Has to be one of 'u', 'v', 'w', 'p', 'q', 'r', 'x', 'y', 'z', 'phi', 'theta', 'psi'.");
    }
}

void operator >> (const YAML::Node& node, InterpolationType& g);
void operator >> (const YAML::Node& node, InterpolationType& g)
{
    std::string t;
    node >> t;
    if      (t == "piecewise constant") g = InterpolationType::PIECEWISE_CONSTANT;
    else if (t == "linear")             g = InterpolationType::LINEAR;
    else if (t == "spline")             g = InterpolationType::SPLINE;
    else
    {
        THROW(__PRETTY_FUNCTION__, InvalidInputException, "Unrecognized interpolation type: '" << t << "'. Has to be one of 'piecewise constant', 'linear', 'spline'");
    }
}

void operator >> (const YAML::Node& node, YamlCSVDOF& g);
void operator >> (const YAML::Node& node, YamlCSVDOF& g)
{
    node["filename"]      >> g.filename;
    node["interpolation"] >> g.interpolation;
    node["state"]         >> g.state;
    node["t"]             >> g.t;
    node["value"]         >> g.value;
}

void operator >> (const YAML::Node& node, YamlDOF<std::vector<double> >& g);
void operator >> (const YAML::Node& node, YamlDOF<std::vector<double> >& g)
{
    node["interpolation"] >> g.interpolation;
    node["state"]         >> g.state;
    node["t"]             >> g.t;
    node["value"]         >> g.value;
    if (node["how applied"]) {
        node["how applied"] >> g.howApplied;
        if (g.howApplied.size() != 6) {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "The forcing force must have 6 components (Fx, Fy, Fz, Mx, My, Mz)");
        }
    }
    else {
        g.howApplied.resize(6);
        for (std::size_t i=0; i<6; ++i) {g.howApplied[i] = 0.;}
        switch(g.state)
        {
            case BlockableState::X:
                g.howApplied[0] = 1.;
                break;
            case BlockableState::Y:
                g.howApplied[1] = 1.;
                break;
            case BlockableState::Z:
                g.howApplied[2] = 1.;
                break;
            case BlockableState::PHI:
                g.howApplied[3] = 1.;
                break;
            case BlockableState::THETA:
                g.howApplied[4] = 1.;
                break;
            case BlockableState::PSI:
                g.howApplied[5] = 1.;
                break;
            default:
                break;
        }
    }
}

void operator >> (const YAML::Node& node, YamlBlockedDOF& b)
{
    if (node["from CSV"])  node["from CSV"]  >> b.from_csv;
    if (node["from YAML"]) node["from YAML"] >> b.from_yaml;
}

YamlBlockedDOF parse(const std::string& yaml)
{
    YAML::Node node = YAML::Load(yaml);
    YamlBlockedDOF ret;
    if (node.size())
    {
        if (node["blocked dof"])
        {
            node["blocked dof"] >> ret;
        }
    }
    return ret;
}

std::string node_to_string(const YAML::Node& node);
std::string node_to_string(const YAML::Node& node)
{
    YAML::Emitter out;
    out << node;
    return out.c_str();
}

#define PARSE_FILTERED_STATE(state) \
        if (node[#state])\
        {\
            p.state = node_to_string(node[#state]);\
        }

void operator >> (const YAML::Node& node, YamlFilteredStates& p)
{
    PARSE_FILTERED_STATE(x);
    PARSE_FILTERED_STATE(y);
    PARSE_FILTERED_STATE(z);
    PARSE_FILTERED_STATE(u);
    PARSE_FILTERED_STATE(v);
    PARSE_FILTERED_STATE(w);
    PARSE_FILTERED_STATE(p);
    PARSE_FILTERED_STATE(q);
    PARSE_FILTERED_STATE(r);
    PARSE_FILTERED_STATE(phi);
    PARSE_FILTERED_STATE(theta);
    PARSE_FILTERED_STATE(psi);
}

std::vector<double> extract_vector_of_doubles(const YAML::Node& node, const std::string& key)
{
    std::vector<double> ret;
    try
    {
        node[key] >> ret;
    }
    catch (const YAML::Exception& e)
    {
        YAML::Emitter emitter;
        emitter << node;
        const std::string msg
            = std::string("Unable to parse '")
              + key + "': expected a list of values (e.g. [1,2,3]). The offending YAML was:\n"
              + emitter.c_str();
        THROW(__PRETTY_FUNCTION__, InvalidInputException, msg);
    }
    return ret;
}
