/*
 * closest_string.hpp
 */

#ifndef CLOSEST_STRING_HPP_
#define CLOSEST_STRING_HPP_

#include <string>
#include <vector>

/** \brief By Levenshtein distance; "" when `strings_to_test` is empty */
std::string closest_string(
    const std::string& string_to_match,
    const std::vector<std::string>& strings_to_test);

#endif /* CLOSEST_STRING_HPP_ */
