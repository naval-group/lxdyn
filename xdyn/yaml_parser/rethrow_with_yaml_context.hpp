/*
 * rethrow_with_yaml_context.hpp
 */

#ifndef RETHROW_WITH_YAML_CONTEXT_HPP_
#define RETHROW_WITH_YAML_CONTEXT_HPP_

struct YamlModel;

/** \brief Call from a catch (...): rethrows an input error with the model's name and YAML line */
[[noreturn]] void rethrow_with_yaml_context(const YamlModel& yaml);

#endif /* RETHROW_WITH_YAML_CONTEXT_HPP_ */
