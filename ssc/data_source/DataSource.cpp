/*
 * DataSource.cpp
 *
 * \date 13 mars 2013, 15:31:43
 * \author cec
 */

#include "ssc/data_source/DataSource.hpp"
#include "ssc/data_source/DataSourceException.hpp"
#include "ssc/data_source/DataSourceDrawer.hpp"

#include <iostream>
#include <sstream>

const std::string ssc::data_source::DataSource::default_setter = "Unregistered DataSource user";

class CycleException : public ssc::data_source::DataSourceException
{
    public:
        CycleException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
            DataSourceException(message, file, function, line)
        {
        }
};

ssc::data_source::DataSource::DataSource() :   name2module(FromName2Module()),
                                               readonly(false),
                                               signals_(SignalContainer()),
                                               module_setting_signals("DataSource user",""),
                                               signal2module(FromSignal2Module()),
                                               module2dependantmodules(DependantModules()),
                                               module2requiredmodules(DependantModules()),
                                               module2requiredsignals(DependantModules()),
                                               signal2dependantmodules(DependantModules()),
                                               is_up_to_date(UpdateState()),
                                               state_names(std::vector<std::pair<std::string,std::string> >()),
                                               aliases(std::map<TypedSignalName,TypedSignalName>()),
                                               forced_values(SignalContainer()),
                                               modules(std::vector<TypedModuleName>()),
                                               call_stack(std::stack<TypedSignalName>()),
                                               in_module(false)
{
}

ssc::data_source::DataSource::DataSource(const DataSource& ds) :  name2module(ds.name2module),
                                                                  readonly(ds.readonly),
                                                                  signals_(ds.signals_),
                                                                  module_setting_signals(ds.module_setting_signals),
                                                                  signal2module(ds.signal2module),
                                                                  module2dependantmodules(ds.module2dependantmodules),
                                                                  module2requiredmodules(ds.module2requiredmodules),
                                                                  module2requiredsignals(ds.module2requiredsignals),
                                                                  signal2dependantmodules(ds.signal2dependantmodules),
                                                                  is_up_to_date(ds.is_up_to_date),
                                                                  state_names(ds.state_names),
                                                                  aliases(ds.aliases),
                                                                  forced_values(ds.forced_values),
                                                                  modules(std::vector<TypedModuleName>()),
                                                                  call_stack(ds.call_stack),
                                                                  in_module(false)
{
    // We need to make sure that all modules now refer to the current DataSource
    FromName2Module::iterator it1 = name2module.begin();
    for (;it1!=name2module.end();++it1)
    {
        it1->second = ModulePtr(it1->second->clone(this));
    }
}

ssc::data_source::DataSource& ssc::data_source::DataSource::operator=(const ssc::data_source::DataSource& ds)
{
    if (this != &ds)
    {
        name2module = ds.name2module;
        readonly = ds.readonly;
        signals_ = ds.signals_;
        module_setting_signals = ds.module_setting_signals;
        signal2module = ds.signal2module;
        module2dependantmodules = ds.module2dependantmodules;
        module2requiredmodules = ds.module2requiredmodules;
        module2requiredsignals = ds.module2requiredsignals;
        signal2dependantmodules = ds.signal2dependantmodules;
        is_up_to_date = ds.is_up_to_date;
        state_names = ds.state_names;
        aliases = ds.aliases;
        forced_values = ds.forced_values;
        modules = ds.modules;
        FromName2Module::iterator it1 = name2module.begin();
        for (;it1!=name2module.end();++it1)
        {
            it1->second = ModulePtr(it1->second->clone(this));
        }
        call_stack = ds.call_stack;
        in_module = ds.in_module;
    }
    return *this;
}

std::string ssc::data_source::DataSource::draw(const bool yaml) const
{
    std::stringstream ss;
    DataSourceDrawer serialize(signal2module, module2requiredsignals, signals_);

    if (yaml)  ss << serialize.get_yaml();
    else       ss << serialize.get_graph();

    return ss.str();
}

bool ssc::data_source::DataSource::read_only() const
{
    return readonly;
}

ssc::data_source::FromName2Module ssc::data_source::DataSource::get_modules() const
{
    return name2module;
}

void ssc::data_source::DataSource::clear()
{
    name2module.clear();
    signals_.clear();
    module_setting_signals = TypedSignalName("DataSource user","");
    signal2module.clear();
    module2dependantmodules.clear();
    module2requiredsignals.clear();
    signal2dependantmodules.clear();
    is_up_to_date.clear();
    forced_values.clear();
}

void ssc::data_source::append(DependantModules& map, const TypedModuleName& key, const TypedModuleName& value)
{
    DependantModules::iterator it = map.find(key);
    if (it == map.end())
    {
        std::set<TypedModuleName> Value;
        Value.insert(value);
        map[key] = Value;
    }
    else it->second.insert(value);
}

void ssc::data_source::DataSource::add_dependencies_and_dependent_modules(const std::set<TypedSignalName>& required_signals, const TypedModuleName& module_using_signals)
{
    std::set<TypedSignalName>::const_iterator that_required_signal = required_signals.begin();
    for (; that_required_signal != required_signals.end(); ++that_required_signal)
    {
        const FromSignal2Module::const_iterator it = signal2module.find(*that_required_signal);
        if (it != signal2module.end())
        {
            const TypedModuleName module_producing_signal = it->second;
            append(module2requiredmodules, module_using_signals, module_producing_signal);
            append(module2dependantmodules, module_producing_signal, module_using_signals);
        }
    }

}

std::set<ssc::data_source::TypedModuleName> ssc::data_source::DataSource::get_dependencies(const TypedModuleName& ref_module, const TypedModuleName& current_module, std::set<TypedModuleName>& dependencies) const
{
    DependantModules::const_iterator that_module = module2dependantmodules.find(current_module);
    if (that_module != module2dependantmodules.end())
    {
        std::set<TypedModuleName> dependant_modules = that_module->second;
        for (std::set<TypedModuleName>::const_iterator that_dependant_module = dependant_modules.begin() ; that_dependant_module != dependant_modules.end() ; ++that_dependant_module)
        {
            if (*that_dependant_module == ref_module)
            {
                THROW(__PRETTY_FUNCTION__, CycleException, std::string("Cycle found: module '") + ref_module.get_signal_name() + std::string("' depends on itself"));
            }
            const std::set<TypedModuleName> new_dependencies = get_dependencies(ref_module,*that_dependant_module,dependencies);
            dependencies.insert(new_dependencies.begin(),new_dependencies.end());
            if (dependencies.find(ref_module) != dependencies.end())
            {
                std::stringstream ss;
                for (std::set<TypedModuleName>::const_iterator it = dependencies.begin() ; it != dependencies.end() ; ++it)
                {
                    ss << it->get_signal_name() << std::endl;
                }
                THROW(__PRETTY_FUNCTION__, CycleException, std::string("Module '") + ref_module.get_signal_name() + "' depends on itself: " + ss.str());
            }
        }
    }
    return dependencies;
}

std::set<ssc::data_source::TypedModuleName> ssc::data_source::DataSource::get_dependencies(const TypedModuleName& module, std::set<TypedModuleName>& dependencies) const
{
    return get_dependencies(module,module, dependencies);
}

bool ssc::data_source::DataSource::a_module_depends_on_itself()
{
    DependantModules::iterator it = module2dependantmodules.begin();
    for (;it!= module2dependantmodules.end();++it)
    {
        get_dependencies(it->first,it->second);
    }
    return false;
}


void ssc::data_source::DataSource::update_dependencies()
{
    DependantModules::const_iterator module_requirements_pair = module2requiredsignals.begin();
    for (;module_requirements_pair!=module2requiredsignals.end();++module_requirements_pair)
    {
        const std::set<TypedSignalName> required_signals = module_requirements_pair->second;
        const TypedSignalName module_using_signals = module_requirements_pair->first;
        add_dependencies_and_dependent_modules(required_signals, module_using_signals);
    }
    if (a_module_depends_on_itself())
    {
        THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("Circular dependency: module '") + module_setting_signals.get_signal_name() + "' depends on itself (eventually)");
    }
}

/** \author cec
 *  \date 21 août 2013, 16:34:06
 *  \brief Registers a derivative: the corresponding variable is added to the
 *  list of states. The order in which this list is sorted corresponds to the
 *  order in which successive calls to define_derivative were made.
 *  \returns Nothing.
 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest set_derivative_example
*/
void ssc::data_source::DataSource::define_derivative(const std::string& state_name, const std::string& derivative_name)
{
    std::vector<std::pair<std::string,std::string> >::const_iterator it = state_names.begin();
    for (;it != state_names.end() ; ++it)
    {
        if (it->first == state_name)
        {
            THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("Cannot define two variables for the same derivative: derivative of state '" + state_name + "' has already been defined as '" + it->second + "': attempting to define it as " + derivative_name));
        }
        if (it->second == derivative_name)
        {
            THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("A variable cannot be the derivative of two different states: '" + derivative_name + "' is already the derivative of '" + it->first + ": attempting to define it as the derivative of '" + state_name + "' as well"));
        }
    }
    state_names.push_back(std::make_pair(state_name,derivative_name));
}

/** \author cec
 *  \date 21 août 2013, 16:42:17
 *  \brief Computes the derivatives of all state variables in the DataSource
 *  \returns A vector of doubles containing all the values of the state variables
 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest get_derivatives_example
*/
void ssc::data_source::DataSource::get_derivatives(std::vector<double>& dx_dt //<! Vector storing the calculated derivatives (must be the right size)
                                 )
{
    const size_t n = state_names.size();
    if (dx_dt.size() != n)
    {
        std::stringstream ss;
        ss << "Invalid size for output vector dx_dt: should be " << n << ", but got " << dx_dt.size();
        THROW(__PRETTY_FUNCTION__, DataSourceException, ss.str());
    }
    for (size_t i=0 ; i<n ; ++i)
    {
        dx_dt[i] = get<double>(state_names[i].second);
    }
}

/** \author cec
 *  \date 21 août 2013, 16:50:03
 *  \brief Sets the values of the state variables.
 *  \returns Nothing.
 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest set_derivatives_example
*/
void ssc::data_source::DataSource::set_states(const std::vector<double>& v)
{
    const size_t n = v.size();
    if (state_names.size() != n)
    {
        std::stringstream ss;
        ss << "Invalid number of values: v has " << n << " elements, but "
           << state_names.size() << " states were detected.";
        THROW(__PRETTY_FUNCTION__, DataSourceException, ss.str());
    }
    for (size_t i = 0 ; i < n ; ++i)
    {
        set<double>(state_names.at(i).first, v[i]);
    }
}

/** \author cec
 *  \date 22 août 2013, 09:53:53
 *  \brief Get the names of all the states in the DataSource
 *  \returns A vector of state names, sorted in the same order as set_states & get_state_derivatives.
 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest get_state_names_example
*/
std::vector<std::string> ssc::data_source::DataSource::get_state_names() const
{
    std::vector<std::string> ret;
    std::vector<std::pair<std::string,std::string> >::const_iterator it;
    for (it = state_names.begin() ; it != state_names.end() ; ++it)
    {
        ret.push_back(it->first);
    }
    return ret;
}

/** \author cec
 *  \date 25 mars 2014, 18:24
 *  \brief Get the current value of all states
 *  \returns A vector of state state values, sorted in the same order as set_states & get_state_derivatives.
*/
std::vector<double> ssc::data_source::DataSource::get_states()
{
    std::vector<double> ret;
    std::vector<std::pair<std::string,std::string> >::const_iterator it;
    for (it = state_names.begin() ; it != state_names.end() ; ++it)
    {
        ret.push_back(get<double>(it->first));
    }
    return ret;
}

/** \author cec
 *  \date 22 août 2013, 12:23:32
 *  \returns All the signals currently in the DataSource
 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest DataSource::get_all_signal_names_example
*/
std::vector<std::string> ssc::data_source::DataSource::get_all_signal_names() const
{
    return signals_.get_all_signal_names();
}

std::list<ssc::data_source::TypedModuleName> ssc::data_source::DataSource::get_module_list() const
{
    std::list<TypedModuleName> ret;
    FromName2Module::const_iterator it = name2module.begin();
    for (;it!=name2module.end() ; ++it)
    {
        ret.push_back(it->first);
    }
    ret.sort();
    return ret;
}

namespace ssc
{
    namespace data_source
    {
        bool operator<(const std::pair<ssc::data_source::TypedSignalName, boost::any>& lhs, const std::pair<ssc::data_source::TypedSignalName, boost::any>& rhs);
    }
}

bool ssc::data_source::operator<(const std::pair<ssc::data_source::TypedSignalName, boost::any>& lhs, const std::pair<ssc::data_source::TypedSignalName, boost::any>& rhs)
{
    return lhs.first < rhs.first;
}

std::list<std::pair<ssc::data_source::TypedSignalName, boost::any> > ssc::data_source::DataSource::get_signals() const
{
    std::list<std::pair<ssc::data_source::TypedSignalName, boost::any> > ret;
    TR1(unordered_map)<ssc::data_source::TypedSignalName, boost::any, OwnHash> in = signals_.get_all_signals();
    TR1(unordered_map)<ssc::data_source::TypedSignalName, boost::any, OwnHash>::const_iterator it = in.begin();
    for (;it!=in.end();++it)
    {
        ret.push_back(std::make_pair(it->first,it->second));
    }
    ret.sort();
    return ret;
}

std::string ssc::data_source::DataSource::who_am_i() const
{
    if (call_stack.empty()) return default_setter;
                            return call_stack.top().get_signal_name() + " (of type " + call_stack.top().get_type_name() + ")";
}

ssc::data_source::TypedSignalName ssc::data_source::DataSource::me() const
{
    if (call_stack.empty()) return TypedSignalName(default_setter, "unknown");
                            return call_stack.top();
}

void ssc::data_source::DataSource::check_in(const ssc::data_source::TypedSignalName& caller)
{
    call_stack.push(caller);
}

void ssc::data_source::DataSource::check_in(const std::string& caller)
{
    check_in(TypedSignalName(caller, "unknown"));
}

void ssc::data_source::DataSource::check_out()
{
    if (not(call_stack.empty())) call_stack.pop();
}

std::string ssc::data_source::DataSource::unwind_call_stack()
{
    std::stringstream ss;
    ss << std::endl << "---[CALL STACK]---" << std::endl;
    if (call_stack.empty()) ss << "by " << default_setter;
    bool first = true;
    while (not(call_stack.empty()))
    {
        const TypedSignalName name = call_stack.top();
        call_stack.pop();
        if (first)
        {
            ss << "X  ";
            first = false;
        }
        else if (call_stack.empty())
            ss << "V  ";
        else
            ss << "|  ";
        ss << name << std::endl;
    }
    ss << "------------------" << std::endl;
    return ss.str();
}
