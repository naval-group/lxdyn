/*
 * TypeCoercion.cpp
 *
 * \date 18 juin 2013, 16:49:02
 * \author cec
 */

#include "ssc/data_source/TypeCoercion.hpp"
#include "ssc/macros/tr1_macros.hpp"

using namespace ssc::data_source;

void ssc::data_source::decoerce(std::list<double>& ret, bool& thing_to_convert)
{
    if (ret.empty())
    {
        THROW(__PRETTY_FUNCTION__, CoercionException, "List is empty.");
    }
    thing_to_convert = (bool)ret.front();
    ret.pop_front();
}

void ssc::data_source::decoerce(std::list<double>& ret, std::vector<bool>& thing_to_convert)
{
    if (ret.size() < thing_to_convert.size())
    {
        THROW(__PRETTY_FUNCTION__, CoercionException, "List containing new values should contain at least as many elements as vector to update");
    }
    const size_t n = thing_to_convert.size();
    for (size_t i = 0 ; i < n ; ++i)
    {
        bool b;
        decoerce(ret, b);
        thing_to_convert.at(i) = b;
    }
}



