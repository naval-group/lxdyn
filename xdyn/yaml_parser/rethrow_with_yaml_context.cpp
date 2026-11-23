/*
 * rethrow_with_yaml_context.cpp
 */

#include "rethrow_with_yaml_context.hpp"

#include <yaml-cpp/exceptions.h>

#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/input/YamlModel.hpp"

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
