/*
 * SignalContainerTypeLists.hpp
 *
 * \date 18 juin 2013, 10:39:54
 * \author cec
 */

#ifndef SIGNALCONTAINERTYPELISTS_HPP_
#define SIGNALCONTAINERTYPELISTS_HPP_

#include "ssc/macros/tr1_macros.hpp"
#include TR1INC(unordered_map)

#include <boost/any.hpp>
#include <string>
#include <list>


//#include <map>

namespace ssc
{
    namespace data_source
    {
        typedef std::string SignalName;
        typedef std::string TypeName;
        class PhysicalQuantity;

        class TypedSignalName
        {
            public:
                TypedSignalName();
                TypedSignalName(const TypedSignalName& rhs) = default;
                TypedSignalName(const SignalName& signal_name, const TypeName& type_name);
                bool operator<(const TypedSignalName& rhs) const;
                bool operator==(const TypedSignalName& rhs) const;
                bool operator!=(const TypedSignalName& rhs) const;
                SignalName get_signal_name() const;
                TypeName get_type_name() const;
                TypedSignalName& operator=(const TypedSignalName& rhs);
            private:
                SignalName _signal_name;
                TypeName _type_name;
        };
    }
}

namespace std
{
#if TR1NAMESPACE
    namespace tr1
    {
#endif
        template <>
        struct hash<ssc::data_source::TypedSignalName>
        {
            std::size_t operator() (const ssc::data_source::TypedSignalName& c) const
            {
               return TR1(hash)<std::string>()(c.get_signal_name()+c.get_type_name());
            }
        };
#if TR1NAMESPACE
    }
#endif
}

namespace ssc
{
    namespace data_source
    {
        ::std::ostream& operator<<(::std::ostream& os, const TypedSignalName& s);

        typedef TypedSignalName TypedModuleName;

        class OwnHash
        {
        public:
            std::size_t operator() (const TypedSignalName& c) const
            {
               return TR1(hash)<std::string>()(c.get_signal_name()+c.get_type_name());
            }
        };


        typedef TR1(unordered_map)<TypedSignalName, boost::any, OwnHash> Signals;
        //typedef std::map<TypedSignalName, boost::any> Signals;
        typedef Signals::const_iterator ConstSignalIterator;

        #define DECLARE_TYPE_ACCESSORS(t) template <> ConvertibleTypesIterator begin<t>(const ConvertibleTypes& l);\
                                          template <> ConvertibleTypesIterator end<t>(const ConvertibleTypes& l);

        #define DEFINE_TYPE_ACCESSORS(t) template <> ConvertibleTypesIterator begin<t>(const ConvertibleTypes& l)\
                                          {\
                                              return l.iter_ ## t .begin();\
                                          }\
                                            \
                                          template <> ConvertibleTypesIterator end<t>(const ConvertibleTypes& l)\
                                          {\
                                              return l.iter_ ## t .end();\
                                          }



        /** \author: cec
         *  \ingroup data_source
         *  \details This structure defines lists of values that are convertible
         *  to doubles. It is used e.g. when calling an integrator (we then need
         *  a list of doubles containing all states of the system). We maintain
         *  two such structures: one for scalar values & one for vectors.
        */
        struct ConvertibleTypes
        {
            ConvertibleTypes();
            void clear();
            void erase(const TypedSignalName& name);
            void erase(std::list<ConstSignalIterator>& l, const TypedSignalName& name);
            std::list<ConstSignalIterator> iter_bool;
            std::list<ConstSignalIterator> iter_char;
            std::list<ConstSignalIterator> iter_wchar_t;
            std::list<ConstSignalIterator> iter_short;
            std::list<ConstSignalIterator> iter_int;
            std::list<ConstSignalIterator> iter_size_t;
            std::list<ConstSignalIterator> iter_long;
            std::list<ConstSignalIterator> iter_float;
            std::list<ConstSignalIterator> iter_double;
            std::list<ConstSignalIterator> iter_phys_qty;
            std::list<ConstSignalIterator> bin;
        };

        typedef std::list<ConstSignalIterator>::const_iterator ConvertibleTypesIterator;

        template <typename T> ConvertibleTypesIterator begin(const ConvertibleTypes& l)
        {
            return l.bin.begin();
        }

        template <typename T> ConvertibleTypesIterator end(const ConvertibleTypes& l)
        {
            return l.bin.end();
        }

        DECLARE_TYPE_ACCESSORS(bool)
        DECLARE_TYPE_ACCESSORS(char)
        DECLARE_TYPE_ACCESSORS(wchar_t)
        DECLARE_TYPE_ACCESSORS(short)
        DECLARE_TYPE_ACCESSORS(int)
        DECLARE_TYPE_ACCESSORS(size_t)
        DECLARE_TYPE_ACCESSORS(long)
        DECLARE_TYPE_ACCESSORS(float)
        DECLARE_TYPE_ACCESSORS(double)
        DECLARE_TYPE_ACCESSORS(PhysicalQuantity)


        /** \author cec
         *  \date 18 juin 2013, 10:08:35
         *  \details When setting a value, we append an iterator to that value
         *  to a list containing all iterators of a given type (i.e. that can be
         *  casted into a given type). Then, when converting all signals to
         *  doubles, we use those lists to treat each type specifically. This
         *  version returns the bin, which will contain all types that cannot
         *  be coerced to doubles.
         *  \returns List of iterators to append to
        */
        template <typename T> std::list<ConstSignalIterator>& select_list_from_type(ConvertibleTypes& lists)
        {
            return lists.bin;
        }


        template<> std::list<ConstSignalIterator>& select_list_from_type<bool>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<char>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<wchar_t>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<short>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<int>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<size_t>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<long>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<float>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<double>(ConvertibleTypes& lists);
        template<> std::list<ConstSignalIterator>& select_list_from_type<PhysicalQuantity>(ConvertibleTypes& lists);
    }
}

#endif  /* SIGNALCONTAINERTYPELISTS_HPP_ */
