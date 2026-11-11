/*
 * SerializeMapsSetsAndVectors.hpp
 *
 * \date Mar 4, 2015
 * \author cec
 */

#ifndef SERIALIZEMAPSSETSANDVECTORS_HPP_
#define SERIALIZEMAPSSETSANDVECTORS_HPP_

#include <map>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& m)
{
    os << "[";
    if (not(m.empty())) os << m.front();
    for (size_t i = 1 ; i < m.size() ; ++i)
    {
        os << ", " << m.at(i);
    }
    os << "]";
    return os;
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::map<T,U>& m)
{
    os << "{";
    if (not(m.empty())) os << m.begin()->first << ": " << m.begin()->second;
    for (auto it = m.begin() ; it != m.end() ; ++it)
    {
        if (it!=m.begin()) os << ", " << it->first << ": " << it->second;
    }
    os << "}";
    return os;
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::pair<T,U>& m)
{
    os << "(" << m.first << "," << m.second << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& m)
{
    os << "{";
    if (not(m.empty())) os << *m.begin();
    for (auto it = m.begin() ; it != m.end() ; ++it)
    {
        if (it!=m.begin()) os << ", " << *it;
    }
    os << "}";
    return os;
}

#endif
