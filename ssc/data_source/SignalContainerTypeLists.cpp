/*
 * SignalContainerTypeLists.cpp
 *
 * \date 18 juin 2013, 10:42:29
 * \author cec
 */

#include "ssc/data_source/SignalContainerTypeLists.hpp"
#include <ostream>

using namespace ssc::data_source;

TypedSignalName::TypedSignalName(const SignalName& signal_name, const TypeName& type_name) : _signal_name(signal_name),
                    _type_name(type_name)
{}

TypedSignalName::TypedSignalName() : _signal_name(""),
                    _type_name("")
{}

TypedSignalName& TypedSignalName::operator=(const TypedSignalName& rhs)
{
    if (this != &rhs)
    {
        _signal_name = rhs._signal_name;
        _type_name = rhs._type_name;
    }
    return *this;
}

bool TypedSignalName::operator<(const TypedSignalName& rhs) const
{
    const bool is_lower =  ((_signal_name < rhs._signal_name)
                           || ((_signal_name == rhs._signal_name) && (_type_name < rhs._type_name)));
    return is_lower;
}

bool TypedSignalName::operator==(const TypedSignalName& rhs) const
{
    return (_signal_name == rhs._signal_name) && (_type_name == rhs._type_name);
}

bool TypedSignalName::operator!=(const TypedSignalName& rhs) const
{
    return not(operator==(rhs));
}

SignalName TypedSignalName::get_signal_name() const
{
    return _signal_name;
}
TypeName TypedSignalName::get_type_name() const
{
    return _type_name;
}

ConvertibleTypes::ConvertibleTypes() :
iter_bool(std::list<ConstSignalIterator>()),
iter_char(std::list<ConstSignalIterator>()),
iter_wchar_t(std::list<ConstSignalIterator>()),
iter_short(std::list<ConstSignalIterator>()),
iter_int(std::list<ConstSignalIterator>()),
iter_size_t(std::list<ConstSignalIterator>()),
iter_long(std::list<ConstSignalIterator>()),
iter_float(std::list<ConstSignalIterator>()),
iter_double(std::list<ConstSignalIterator>()),
iter_phys_qty(std::list<ConstSignalIterator>()),
bin(std::list<ConstSignalIterator>())
{

}

void ConvertibleTypes::clear()
{
    iter_bool.clear();
    iter_char.clear();
    iter_wchar_t.clear();
    iter_short.clear();
    iter_int.clear();
    iter_size_t.clear();
    iter_long.clear();
    iter_float.clear();
    iter_double.clear();
    iter_phys_qty.clear();
    bin.clear();
}

void ConvertibleTypes::erase(const TypedSignalName& name)
{
    erase(iter_bool, name);
    erase(iter_char, name);
    erase(iter_wchar_t, name);
    erase(iter_short, name);
    erase(iter_int, name);
    erase(iter_size_t, name);
    erase(iter_long, name);
    erase(iter_float, name);
    erase(iter_double, name);
    erase(iter_phys_qty, name);
    erase(bin, name);
}

void ConvertibleTypes::erase(std::list<ConstSignalIterator>& l, const TypedSignalName& name)
{
    std::list<ConstSignalIterator> ret;
    for (std::list<ConstSignalIterator>::const_iterator it = l.begin() ; it != l.end() ; ++it)
    {
        if ((*it)->first != name) ret.push_back(*it);
    }
    l = ret;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<bool>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_bool;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<char>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_char;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<wchar_t>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_wchar_t;
}
template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<short>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_short;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<int>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_int;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<size_t>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_size_t;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<long>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_long;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<float>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_float;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<double>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_double;
}

template<> std::list<ssc::data_source::ConstSignalIterator>& ssc::data_source::select_list_from_type<PhysicalQuantity>(ssc::data_source::ConvertibleTypes& lists)
{
    return lists.iter_phys_qty;
}

namespace ssc
{
    namespace data_source
    {
        DEFINE_TYPE_ACCESSORS(bool)
        DEFINE_TYPE_ACCESSORS(char)
        DEFINE_TYPE_ACCESSORS(wchar_t)
        DEFINE_TYPE_ACCESSORS(short)
        DEFINE_TYPE_ACCESSORS(int)
        DEFINE_TYPE_ACCESSORS(size_t)
        DEFINE_TYPE_ACCESSORS(long)
        DEFINE_TYPE_ACCESSORS(float)
        DEFINE_TYPE_ACCESSORS(double)
        template <> ConvertibleTypesIterator begin<PhysicalQuantity>(const ConvertibleTypes& l)
        {
            return l.iter_phys_qty.begin();
        }

        template <> ConvertibleTypesIterator end<PhysicalQuantity>(const ConvertibleTypes& l)
        {
            return l.iter_phys_qty.end();
        }

        ::std::ostream& operator<<(::std::ostream& os, const TypedSignalName& s)
        {
            os << s.get_signal_name() << "(:" << s.get_type_name() << ")";
            return os;
        }
    }
}
