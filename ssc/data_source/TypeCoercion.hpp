/*
 * TypeCoercion.hpp
 *
 * \date 17 juin 2013, 14:50:54
 * \author cec
 */

#ifndef TYPECOERCION_HPP_
#define TYPECOERCION_HPP_


#include "ssc/data_source/CoercionException.hpp"
#include <cstddef>
#include <vector>
#include <list>

namespace ssc
{
    namespace data_source
    {
        template<bool B, class T = void>
        struct enable_if {};

        template<class T>
        struct enable_if<true, T> { typedef T type; };

        template<class T> struct is_arithmetic { const static bool value = false; };
        #define DECLARE_ARITHMETIC_TYPE(t) template<> struct is_arithmetic<t> { const static bool value = true; }

        DECLARE_ARITHMETIC_TYPE(bool);
        DECLARE_ARITHMETIC_TYPE(char);
        DECLARE_ARITHMETIC_TYPE(wchar_t);
        DECLARE_ARITHMETIC_TYPE(short);
        DECLARE_ARITHMETIC_TYPE(int);
        DECLARE_ARITHMETIC_TYPE(long);
        DECLARE_ARITHMETIC_TYPE(float);
        DECLARE_ARITHMETIC_TYPE(double);
        DECLARE_ARITHMETIC_TYPE(size_t);





        /** \author: cec
         *  \ingroup data_source
         *  \brief Converts type to list of doubles
         *  \snippet data_source/unit_tests/src/TypeCoercionTest.cpp example
        */

        template <class T>
        typename enable_if<is_arithmetic<T>::value,void>::type coerce(std::list<double>& ret, const T thing_to_convert)
        {
            ret.push_back((double)thing_to_convert);
        }

        template <typename T> void coerce(std::list<double>& ret, const std::vector<T>& thing_to_convert)
        {
            for (typename std::vector<T>::const_iterator it = thing_to_convert.begin() ; it != thing_to_convert.end() ; ++it)
            {
                coerce(ret, *it);
            }
        }

        void throw_if_list_is_empty(const std::list<double>& l);

        template <class T>
        typename enable_if<is_arithmetic<T>::value,void>::type decoerce(std::list<double>& ret, T& thing_to_convert)
        {
            if (ret.empty())
            {
                THROW(__PRETTY_FUNCTION__, CoercionException, "List is empty.");
            }
            thing_to_convert = (T)ret.front();
            ret.pop_front();
        }

        void decoerce(std::list<double>& ret, bool& thing_to_convert);
        void decoerce(std::list<double>& ret, std::vector<bool>& thing_to_convert);

        template <typename T> void decoerce(std::list<double>& ret, std::vector<T>& thing_to_convert)
        {
            if (ret.size() < thing_to_convert.size())
            {
                THROW(__PRETTY_FUNCTION__, CoercionException, "List containing new values should contain at least as many elements as vector to update");
            }
            for (typename std::vector<T>::iterator it = thing_to_convert.begin() ; it != thing_to_convert.end() ; ++it)
            {
                decoerce(ret, *it);
            }
        }
    }
}

#endif /* TYPECOERCION_HPP_ */
