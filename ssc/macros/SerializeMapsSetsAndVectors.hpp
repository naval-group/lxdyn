/*
 * SerializeMapsSetsAndVectors.hpp
 *
 * \date Mar 4, 2015
 * \author cec
 */

#ifdef _GLIBCXX_VECTOR
#ifndef SERIALIZEVECTORS_HPP_
#define SERIALIZEVECTORS_HPP_
#include <ostream>
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
#endif
#endif

#ifdef _GLIBCXX_MAP
#ifndef SERIALIZEMAPS_HPP_
#define SERIALIZEMAPS_HPP_
#include <ostream>
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
#endif
#endif

#ifdef _STL_PAIR_H
#ifndef SERIALIZEPAIRS_HPP_
#define SERIALIZEPAIRS_HPP_
#include <ostream>
template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::pair<T,U>& m)
{
    os << "(" << m.first << "," << m.second << ")";
    return os;
}
#endif
#endif

#ifdef _GLIBCXX_SET
#ifndef SERIALIZESET_HPP_
#define SERIALIZESET_HPP_
#include <ostream>
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
#endif

