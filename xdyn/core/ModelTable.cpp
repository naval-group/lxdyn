/*
 * ModelTable.cpp
 */

#include "ModelTable.hpp"

#include "closest_string.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "yaml-cpp/exceptions.h"

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

void rethrow_with_yaml_context(const YamlModel& yaml)
{
    const std::string context = "Invalid input data for model '" + yaml.model + "'.";
    try {
        throw;
    } catch (const InvalidInputException& exception) {
        THROW(
            __PRETTY_FUNCTION__,
            InvalidInputException,
            context << std::endl
                    << exception.get_message() << std::endl
                    << "Model containing error is defined line "
                    << yaml.index_of_first_line_in_global_yaml << " of the YAML file."
                    << std::endl);
    } catch (const YAML::Exception& exception) {
        THROW(
            __PRETTY_FUNCTION__,
            InvalidInputException,
            context << std::endl
                    << "Model containing error is defined line "
                    << yaml.index_of_first_line_in_global_yaml << " of the YAML file." << std::endl
                    << "The error was: " << exception.msg);
    }
}
