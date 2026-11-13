/*
 * yaml_compat.h
 *
 * Compatibility shim for yaml-cpp 0.3 -> 0.8 migration.
 * Restores the operator>> extraction pattern used throughout the codebase.
 *
 * In yaml-cpp 0.3, node >> value was built-in for basic types and users
 * defined operator>> for custom types. In yaml-cpp 0.8, the extraction
 * operator was removed in favor of node.as<T>().
 *
 * This header provides:
 * 1. A template operator>> for basic types (delegates to node.as<T>())
 * 2. A template operator>> for std::vector<T> (iterates using operator>>)
 *
 * User-defined operator>> for custom types (non-template) take precedence
 * over these templates due to C++ overload resolution rules.
 */

#ifndef YAML_COMPAT_H_
#define YAML_COMPAT_H_

#include <yaml-cpp/yaml.h>

#include <string>
#include <type_traits>
#include <vector>

namespace yaml_compat_detail {

// Trait to detect std::vector
template <typename T>
struct is_vector : std::false_type {};
template <typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {};

}  // namespace yaml_compat_detail

// Restore operator>> for scalar types that have YAML::convert<T> support.
// This template has lower priority than any non-template operator>> overload,
// so user-defined operator>>(const YAML::Node&, CustomType&) still works.
template <typename T>
inline typename std::enable_if<!yaml_compat_detail::is_vector<T>::value, void>::type operator>>(
    const YAML::Node& node,
    T& value)
{
    value = node.as<T>();
}

// Restore operator>> for std::vector<T>.
// Iterates over the YAML sequence and extracts each element using operator>>,
// which dispatches to either user-defined operator>> (for custom types) or
// the template above (for basic types).
template <typename T>
inline void operator>>(const YAML::Node& node, std::vector<T>& vec)
{
    vec.clear();
    if (!node.IsDefined() || node.IsNull())
        return;
    vec.reserve(node.size());
    for (std::size_t i = 0; i < node.size(); ++i) {
        T val;
        node[i] >> val;
        vec.push_back(std::move(val));
    }
}

#endif /* YAML_COMPAT_H_ */
