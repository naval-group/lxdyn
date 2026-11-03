/*
 * almost_equal.hpp
 *
 * \date 26 nov. 2013, 08:40:25
 * \author cec
 */

#ifndef ALMOST_EQUAL_HPP_
#define ALMOST_EQUAL_HPP_

#include <string>
#include <vector>

template <class T>
bool almost_equal(const T& , const T&, const double eps=0)
{
    (void) eps;
    return false;
}

template <> bool almost_equal(const double& t1, const double& t2, const double eps);
template <> bool almost_equal(const float& t1, const float& t2, const double eps);
template <> bool almost_equal(const int& t1, const int& t2, const double eps);
template <> bool almost_equal(const size_t& t1, const size_t& t2, const double eps);
template <> bool almost_equal(const std::string& t1, const std::string& t2, const double eps);
template <> bool almost_equal(const std::vector<double>& t1, const std::vector<double>& t2, const double eps);

#endif /* ALMOST_EQUAL_HPP_ */
