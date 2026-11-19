/*
 * ModelTable.hpp
 *
 * A fixed table mapping each model name a YAML file may use to the factory that builds the model.
 */

#ifndef MODELTABLE_HPP_
#define MODELTABLE_HPP_

#include <array>
#include <string>
#include <vector>

#include "xdyn/external_data_structures/YamlModel.hpp"

template <typename Factory>
struct ModelEntry {
    std::string name;
    Factory factory;
};

/** \brief The factory registered under `name`, nullptr when no entry has that name */
template <typename Factory, std::size_t N>
Factory find_model(const std::array<ModelEntry<Factory>, N>& table, const std::string& name)
{
    for (const auto& entry : table) {
        if (entry.name == name) {
            return entry.factory;
        }
    }
    return nullptr;
}

template <typename Factory, std::size_t N>
std::vector<std::string> model_names(const std::array<ModelEntry<Factory>, N>& table)
{
    std::vector<std::string> names;
    names.reserve(N);
    for (const auto& entry : table) {
        names.push_back(entry.name);
    }
    return names;
}

/** \brief Suggests the closest of `known_names` */
[[noreturn]] void throw_unknown_model(
    const YamlModel& yaml,
    const std::vector<std::string>& known_names);

/** \brief For names with no YAML line of their own; `kind` is e.g. "output format" */
[[noreturn]] void throw_unknown_name(
    const std::string& kind,
    const std::string& name,
    const std::vector<std::string>& known_names);

/** \brief Call from a catch (...): rethrows an input error with the model's name and YAML line */
[[noreturn]] void rethrow_with_yaml_context(const YamlModel& yaml);

#endif /* MODELTABLE_HPP_ */
