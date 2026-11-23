/*
 * ModelTable.cpp
 */

#include "ModelTable.hpp"

#include "closest_string.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"

void throw_unknown_model(const YamlModel& yaml, const std::vector<std::string>& known_names)
{
    THROW(
        __PRETTY_FUNCTION__,
        InvalidInputException,
        "Simulator does not know model '"
            << yaml.model << "', defined line " << yaml.index_of_first_line_in_global_yaml
            << " of the YAML file. Did you mean '" << closest_string(yaml.model, known_names)
            << "'?");
}

void throw_unknown_name(
    const std::string& kind,
    const std::string& name,
    const std::vector<std::string>& known_names)
{
    THROW(
        __PRETTY_FUNCTION__,
        InvalidInputException,
        "Simulator does not know " << kind << " '" << name << "'. Did you mean '"
                                   << closest_string(name, known_names) << "'?");
}
