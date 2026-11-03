/*
 * DataSource.hpp
 *
 * \date 13 mars 2013, 15:31:43
 * \author cec
 */

#ifndef DATASOURCE_HPP_
#define DATASOURCE_HPP_

#include "ssc/macros/tr1_macros.hpp"
#include TR1INC(memory)
#include "ssc/data_source/DataSourceModule.hpp"
#include "ssc/data_source/SignalContainer.hpp"
#include "ssc/data_source/DataSourceException.hpp"
#include "ssc/numeric/almost_equal.hpp"

#include <string>
#include <set>
#include <sstream>
#include <stack>

#define MAKE(ns,x,T) namespace x {typedef T _type;static const std::string _name = #ns "_" #x;}
#define DEFINE(ns,x, T) namespace ns {MAKE(ns,x,T)}
#define PTR_GET(x) ds->get<x::_type>(x::_name)
#define PTR_SET(x,val) ds->set<x::_type>(x::_name,val)
#define GET(ds,x) ds.get<x::_type>(x::_name)
#define SET(ds,x,val) ds.set<x::_type>(x::_name,val)
#define FORCE(ds,x,val) ds.force<x::_type>(x::_name,val)

#define USE_HASH_MAPS 1

#if USE_HASH_MAPS
#include TR1INC(unordered_map)
#else
#include <map>
#endif

namespace ssc
{
    namespace data_source
    {
        typedef TR1(shared_ptr)<const DataSourceModule> ModulePtr;

        #if USE_HASH_MAPS
            typedef TR1(unordered_map)<TypedModuleName,ModulePtr > FromName2Module;
            typedef TR1(unordered_map)<TypedSignalName,TypedModuleName> FromSignal2Module;
            typedef TR1(unordered_map)<TypedModuleName,std::set<TypedModuleName> > DependantModules;
            typedef TR1(unordered_map)<TypedModuleName,bool > UpdateState;
        #else
            typedef std::map<TypedModuleName,ModulePtr > FromName2Module;
            typedef std::map<TypedSignalName,TypedModuleName> FromSignal2Module;
            typedef std::map<TypedModuleName,std::set<TypedModuleName> > DependantModules;
            typedef std::map<TypedModuleName,bool > UpdateState;
        #endif



        void append(DependantModules& map, const TypedModuleName& key, const TypedModuleName& value);

        /** \author cec
         *  \brief Simplifies the creation of simulators using forward/reverse chaining.
         *  \details It provides modules with the data they need (which may come from other modules)
         *  \ingroup data_source
         *  \section ex1 Example
         *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest example
         *  \section ex2 Expected output
         *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest expected output
         */
        class DataSource
        {
            public:
                DataSource();
                DataSource(const DataSource& ds);
                DataSource& operator=(const DataSource& ds);

                /**  \author cec
                  *  \date Apr 28, 2014, 1:59:54 PM
                  *  \brief Checks if DataSource has a given signal
                  *  \details Avoids having to write a try/catch block
                  *  \returns true if the signal was found, false otherwise
                  *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest has_example
                  */
                template <typename T> bool has(const std::string& in //!< Signal to test
                        ) const
                {
                    return signals_.has(typify<T>(in));
                }


                /** \author cec
                 *  \date 1 juil. 2013, 17:41:30
                 *  \brief draws the DataSource's dependency graph
                 *  \returns String containing the serialization of the DataSource
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest draw_example
                */
                std::string draw(const bool yaml=true) const;

                /** \author cec
                 *  \date 17 juin 2013, 10:52:20
                 *  \brief Adds a module to the DataSource.
                 *  \details This method adds the module to the list, initializes it &
                 *  retrieves all its dependencies.
                 *  Another API which was considered earlier was add<M>(module_name) but
                 *  this doesn't play well with mocks (you don't have access to
                 *  the module because you leave its instantiation to the DataSource)
                 *  \returns
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                template <typename T> void add(const T& module)
                {
                    const ModulePtr m = add_module_if_not_already_present_and_return_clone(module);
                    m->initialize();
                    const bool read_only_bak = readonly;
                    const TypedModuleName module_setting_signals_bak = module_setting_signals;
                    readonly = true; // We don't want the following call to module.update()
                                      // to modify the signals in the DataSource: we just
                                      // want to track module dependencies
                    const bool in_module_back = in_module;
                    try
                    {
                        check_in(typify<T>(m->get_name()));
                        in_module = true;
                        m->update();
                        in_module = in_module_back;
                        check_out();
                    }
                    catch(DataSourceException& e)
                    {
                        throw(e);
                    }
                    catch(...)
                    {
                        // It's OK for m->update() to throw at this stage because we just want to retrieve its dependencies
                    }
                    modules.push_back(typify<T>(module.get_name()));
                    readonly = read_only_bak;
                }

                /** \author cec
                 *  \date 23 août 2013, 14:34:30
                 *  \brief This method was created because we sometimes don't know the name
                 *  of a signal from inside a module: therefore, when adding the module, we
                 *  create an alias of the signal name the module is expecting against the name
                 *  of the signal actually in the DataSource.
                 *  \returns Nothing.
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest DataSource::alias_example
                */
                template <typename T>
                void alias(const std::string& name_of_copy, const std::string& copied_signal)
                {
                    FromSignal2Module::const_iterator that_signal = signal2module.find(typify<T>(name_of_copy));
                    if (that_signal != signal2module.end())
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("\nAsked to create alias ")+name_of_copy + " -> " + copied_signal + ", but '" + name_of_copy + "' is already in this DataSource (set by '" + that_signal->second.get_signal_name() + "')");
                    }
                    const std::map<TypedSignalName,TypedSignalName>::const_iterator it = aliases.find(typify<T>(name_of_copy));
                    if ((it != aliases.end()) && (it->second != typify<T>(copied_signal)))
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("\n'")+name_of_copy+"' is already an alias for '" + it->second.get_signal_name());
                    }
                    aliases[typify<T>(name_of_copy)] = typify<T>(copied_signal);
                }

                /** \author cec
                 *  \date 24 juil. 2013, 15:13:58
                 *  \brief Remove a module from a data source (and from the lists of dependencies)
                 *  In this version, the module name is assumed to have been auto-generated.
                 *  \returns Nothing
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                template <typename T> void remove()
                {
                    const std::string module_to_remove = std::string(typeid(T).name()) + "auto-generated by DataSource";
                    remove<T>(module_to_remove);
                }

                /** \author cec
                 *  \date 24 juil. 2013, 15:13:58
                 *  \brief Remove a module from a data source (and from the lists of dependencies)
                 *  \returns Nothing
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                template <typename T> void remove(const std::string& module_to_remove)
                {
                    const TypedSignalName typed_module_to_remove(module_to_remove, typeid(T).name());
                    if (name2module.find(typed_module_to_remove) == name2module.end())
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException,
                                std::string("\nAttempting to remove module '")
                                        + module_to_remove
                                        + "' which is not in this DataSource.");
                    }
                    module2dependantmodules.erase(typed_module_to_remove);
                    name2module.erase(typed_module_to_remove);
                    std::vector<TypedSignalName> signals_to_remove;
                    for (FromSignal2Module::const_iterator it = signal2module.begin() ; it != signal2module.end() ; ++it)
                    {
                        if (it->second == typed_module_to_remove) signals_to_remove.push_back(it->first);
                    }
                    for (std::vector<TypedSignalName>::const_iterator it = signals_to_remove.begin() ; it != signals_to_remove.end() ; ++it)
                    {
                        signal2module.erase(*it);
                    }
                    module2dependantmodules.erase(typed_module_to_remove);
                    module2requiredmodules.erase(typed_module_to_remove);
                    module2requiredsignals.erase(typed_module_to_remove);
                    update_dependencies();
                }

                /** \author cec
                 *  \date 18 juin 2013, 22:19:54
                 *  \brief Templated version of 'add'
                 *  \details This is the version normally used to add a module because
                 *  it automatically adds the present DataSource (*this) to the module's
                 *  constructor.
                 *  \returns Nothing
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                template <typename T> void add(const std::string& module_name)
                {
                    if (module_name == default_setter)
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("\nCannot name a module '") + default_setter + "' because it is a reserved name (used to track the signals not set by a module (i.e. 'outside' the DataSource)" + unwind_call_stack());
                    }
                    if (module_name == "")
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException, "\nModule name cannot be empty" + unwind_call_stack());
                    }
                    const T t(this, module_name);
                    add(t);
                }

                /** \author cec
                 *  \date 2 juil. 2013, 14:16:50
                 *  \brief 'add' without specifying the name of the module (if there is only one of each type)
                 *  \returns Nothing
                 *  \snippet /unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                template <typename T> void add()
                {
                    add<T>(std::string(typeid(T).name()) + "auto-generated by DataSource");
                }

                /** \author cec
                 *  \date 17 juin 2013, 10:55:37
                 *  \brief Retrieve the list of all modules currently in the DataSource
                 *  \returns List of all present modules
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                FromName2Module get_modules() const;

                /** \author cec
                 *  \date 17 juin 2013, 10:41:56
                 *  \brief Removes all modules & all signals from a DataSource.
                 *  \returns Nothing
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest DataSource::clear_example
                */
                void clear();

            private:
                /** \author cec
                 *  \date 17 juin 2013, 11:00:13
                 *  \details Checks if a module with the same name exists in the DataSource.
                 *  If it exists, throw an exception. Otherwise, add the module to name2module.
                 *  \returns Pointer to the added module.
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                 */
                template <typename T> ModulePtr add_module_if_not_already_present_and_return_clone(const T& module)
                {
                    const TypedModuleName module_name = typify<T>(module.get_name());
                    FromName2Module::iterator it = name2module.find(module_name);
                    const bool module_is_already_in_map = it != name2module.end();
                    if (module_is_already_in_map)
                    {
                        std::string s = "\nA module named '";
                        THROW(__PRETTY_FUNCTION__, DataSourceException, s + module.get_name() + "' already exists" + unwind_call_stack());
                    }
                    ModulePtr ret(module.clone());
                    module_setting_signals = module_name;
                    name2module.insert(std::make_pair(module_setting_signals, ret));
                    is_up_to_date[module_setting_signals] = false;
                    return ret;
                }

                template<typename T>
                void check_signal_is_neither_forced_nor_set_by_another_module(const std::string& signal_name)
                {
                    if (forced_values.has(typify<T>(signal_name)))
                    {
                        if (not(in_module))
                        {
                            THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("\nAttempting to set '") + signal_name + "' which has been overridden using DataSource::force" + unwind_call_stack());
                        }
                    }
                    else
                    {
                        FromSignal2Module::const_iterator it = signal2module.find(typify<T>(signal_name));
                        const bool is_up_to_date_contains_module_name = (it != signal2module.end()) and (is_up_to_date.find(it->second)!=is_up_to_date.end());
                        if ((it != signal2module.end())
                                && (it->second.get_signal_name() != "")
                                && (it->second != me())
                                && is_up_to_date_contains_module_name)
                        {
                            THROW(__PRETTY_FUNCTION__, DataSourceException,
                                              "\nAttempt to set signal '" + signal_name
                                            + "', but module '" + it->second.get_signal_name()
                                            + "' already sets it. A possible fix may be to use DataSource::remove: "
                                            + unwind_call_stack());
                        }
                    }
                }

                template<typename T>
                void set_all_dependent_modules_out_of_date(const std::string& signal_name)
                {
                    DependantModules::const_iterator it = signal2dependantmodules.find(typify<T>(signal_name));
                    if (it != signal2dependantmodules.end())
                    {
                        const std::set<TypedSignalName> S = it->second;
                        for (std::set<TypedSignalName>::const_iterator it = S.begin(); it != S.end(); ++it)
                        {
                            is_up_to_date[*it] = false;
                            const std::set<TypedSignalName> S2 = module2dependantmodules[*it];
                            for (std::set<TypedSignalName>::const_iterator it2 = S2.begin(); it2 != S2.end(); ++it2)
                            {
                                is_up_to_date[*it2] = false;
                            }
                        }
                    }
                }

                template<typename T> void update_dependencies(const std::string& signal_name)
                {
                    if (signal_name != "")
                    {
                        signal2module[typify<T>(signal_name)] = me();// module_setting_signals;
                        update_dependencies();
                    }
                }

            public:
                /** \author cec
                 *  \date 18 juin 2013, 21:56:15
                 *  \brief Adds/updates a signal in the DataSource
                 *  \returns Nothing
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest set_example
                 */
                template<typename T>
                void set(std::string signal_name, //<! Name of the signal to create or update
                         const T& t               //<! Value to set
                         )
                {
                    std::map<TypedSignalName, TypedSignalName>::const_iterator that_alias = aliases.find(typify<T>(signal_name));
                    const bool found_alias = that_alias != aliases.end();
                    if (found_alias) signal_name = that_alias->second.get_signal_name();
                    check_signal_is_neither_forced_nor_set_by_another_module<T>(signal_name);
                    if (readonly) update_dependencies<T>(signal_name);
                    else
                    {
                        if (signals_.has(typify<T>(signal_name)))
                        {
                            const T old_t = signals_.get<T>(signal_name);
                            if (not(almost_equal(old_t,t)))
                            {
                                signals_.set(signal_name, t);
                                set_all_dependent_modules_out_of_date<T>(signal_name);
                            }
                        }
                        else
                        {
                            signals_.set(signal_name, t);
                            set_all_dependent_modules_out_of_date<T>(signal_name);
                            signal2module[typify<T>(signal_name)] = me();
                        }
                    }
                }

                /** \author cec
                 *  \date 21 nov. 2013, 15:44:12
                 *  \brief
                 *  \returns
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                */
                template<typename T>
                        void unset(const std::string& signal_name //<! Name of the signal to remove
                                   )
                {
                    std::map<TypedSignalName, TypedSignalName>::iterator that_alias = aliases.find(typify<T>(signal_name));
                    const bool found_alias = that_alias != aliases.end();
                    if (found_alias) aliases.erase(that_alias);
                    signals_.remove<T>(signal_name);
                    set_all_dependent_modules_out_of_date<T>(signal_name);
                }

                /** \author cec
                 *  \date 27 août 2013, 08:53:49
                 *  \brief Override a signal set by a module.
                 *  This works only if the signal has been set by a module, *not* if it
                 *  was set 'manually' by the default_setter (outside the DataSource).
                 *  All subsequent 'set' operations must be performed by 'force'.
                 *  A 'force' operation can be canceled by DataSource::release.
                 *  \returns Nothing.
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest force_example
                 */
                template<typename T>
                void force(const std::string& signal_name, const T& forced_value)
                {
                    FromSignal2Module::const_iterator it = signal2module.find(typify<T>(signal_name));
                    const bool signal_set_by_module = it==signal2module.end() ? false : it->second.get_type_name()!="unknown";
                    if (not(signal_set_by_module))
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException,
                                std::string("\nAttempting to force the value of signal '")
                                        + signal_name + "' (of type '"
                                        + std::string(typeid(T).name())
                                        + "'), which is not set by any module: can only force a value set by a module (otherwise, use DataSource::set). " + unwind_call_stack());
                    }
                    forced_values.set<T>(signal_name, forced_value);
                    set_all_dependent_modules_out_of_date<T>(signal_name);
                }

                /** \author cec
                 *  \date 27 août 2013, 09:26:14
                 *  \brief Removes a signal from the list of forced values. This means
                 *  the signal will be computed by the module instead.
                 *  \returns Nothing.
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest enclosing_method_example
                 */
                template<typename T>
                void release(const std::string& signal_name)
                {
                    if (!(forced_values.has(typify<T>(signal_name))))
                    {
                        THROW(__PRETTY_FUNCTION__, DataSourceException, std::string("\nAttempting to release '") + signal_name + "' which is not forced. " + unwind_call_stack());
                    }
                    forced_values.remove<T>(signal_name);
                }

        private:
                void update_if_necessary(const TypedSignalName& module_name)
                {
                    const bool is_up_to_date_contains_module_name = (is_up_to_date.find(module_name)!=is_up_to_date.end());
                    if (is_up_to_date_contains_module_name && not(is_up_to_date[module_name]))
                    {
                        const bool in_module_back = in_module;
                        module_setting_signals = module_name;
                        check_in(module_name);
                        in_module = true;
                        name2module[module_name]->update();
                        in_module = in_module_back;
                        check_out();
                        is_up_to_date[module_name] = true;
                    }
                }

                template<typename T>
                void append_to_maps(const std::string& signal_name)
                {
                    append(signal2dependantmodules, typify<T>(signal_name),
                            module_setting_signals);
                    append(module2requiredsignals, module_setting_signals,
                            typify<T>(signal_name));
                }

                template<typename T>
                void update_or_throw(const std::string& signal_name)
                {
                    const TypedSignalName typed_name = typify<T>(signal_name);
                    const FromSignal2Module::const_iterator that_signal = signal2module
                            .find(typed_name);
                    const bool computable = that_signal != signal2module.end();
                    const bool stored = signals_.has(typed_name);
                    if (computable)
                    {
                        const TypedSignalName module_name = that_signal->second;
                        update_if_necessary(module_name);
                    } else if (!(stored))
                    {
                        const std::vector<TypedSignalName> closest_match = signals_.get_closest_match(typed_name);
                        std::stringstream ss;
                        if (not(closest_match.empty()))
                        {
                            ss << " Closest match: '"
                               << closest_match.front().get_signal_name() << "' (of type '" << closest_match.front().get_type_name() << "')";
                        }
                        else
                        {
                            ss << " None of the signals have the right type or the right name";
                        }
                        for (size_t i = 1 ; i < closest_match.size() ; ++i)
                        {
                            ss << ", '" << closest_match.at(i).get_signal_name()
                               << "' (of type '" << closest_match.at(i).get_type_name() << "')";
                        }
                        if (not(closest_match.empty())) ss << ".";
                        THROW(__PRETTY_FUNCTION__, DataSourceException,
                                std::string("\nUnable to find signal '") + signal_name
                                        + "' (of type '" + typed_name.get_type_name() + "') " + unwind_call_stack());
                    }
                }
        public:
                /** \author cec
                 *  \date 18 juin 2013, 22:01:33
                 *  \brief Retrieves a signal from the SignalContainer
                 *  \returns Value of the signal
                 *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest get_example
                 */
                template<typename T>
                T get(const std::string& signal_name)
                {
                    std::map<TypedSignalName,TypedSignalName>::const_iterator it = aliases.find(typify<T>(signal_name));
                    if (it != aliases.end())
                    {
                        return get<T>(it->second.get_signal_name());
                    }
                    if (forced_values.has(typify<T>(signal_name)))
                    {
                        return forced_values.get<T>(signal_name);
                    }
                    if (readonly && (module_setting_signals != typify<T>(default_setter)))
                    {
                        append_to_maps<T>(signal_name);
                        const TypedSignalName module_setting_signals_bak = module_setting_signals;
                        update_dependencies();
                        module_setting_signals = module_setting_signals_bak;
                        return T();
                    }
                    else
                    {
                        const TypedSignalName module_setting_signals_bak = module_setting_signals;
                        update_or_throw<T>(signal_name);
                        module_setting_signals = module_setting_signals_bak;
                    }
                    return signals_.get<T>(signal_name);
                }
                bool read_only() const;
                void define_derivative(const std::string& state_name, const std::string& derivative_name);
                void get_derivatives(std::vector<double>& dx_dt);
                void set_states(const std::vector<double>& v);
                std::vector<std::string> get_state_names() const;
                std::vector<double> get_states();
                std::vector<std::string> get_all_signal_names() const;
                std::list<TypedModuleName> get_module_list() const;
                std::list<std::pair<TypedSignalName, boost::any> > get_signals() const;

                /**  \author cec
                  *  \date Jun 12, 2014, 9:24:26 AM
                  *  \brief Returns the name of the module currently setting signals in the DataSource
                  *  \returns Top of the call stack
                  *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest who_am_i_example
                  */
                std::string who_am_i() const;
                TypedSignalName me() const;

                /**  \author cec
                  *  \date Jun 12, 2014, 9:24:26 AM
                  *  \brief Register caller in DataSource
                  *  \details All get/set calls are now supposed to originate from the caller.
                  *  Adds it to the call stack.
                  *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest who_am_i_example
                  */
                void check_in(const std::string& caller);

                /**  \author cec
                  *  \date Jun 12, 2014, 9:24:26 AM
                  *  \brief Register caller in DataSource (module version)
                  *  \details All get/set calls are now supposed to originate from the caller.
                  *  Adds it to the call stack.
                  *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest who_am_i_example
                  */
                void check_in(const TypedSignalName& caller);

                /**  \author cec
                  *  \date Jun 12, 2014, 9:24:26 AM
                  *  \brief Removes last caller from the call stack (pop)
                  *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest who_am_i_example
                  */
                void check_out();

                static const std::string default_setter; //!< Default name of the DataSource user

                /**  \returns A map containing all signals of a given type
                  *  \snippet data_source/unit_tests/src/DataSourceTest.cpp DataSourceTest get_all_example
                  */
                template <typename T> typename std::map<std::string,T> get_all() const
                {
                    return signals_.get_all<T>();
                }

            private:
                template <typename T> TypedSignalName typify(const std::string& signal_name) const
                {
                    return TypedSignalName(signal_name, typeid(T).name());
                }

                std::string unwind_call_stack();

                std::set<TypedModuleName> get_dependencies(const TypedModuleName& module_name, std::set<TypedModuleName>& ret) const;
                std::set<TypedModuleName> get_dependencies(const TypedModuleName& ref_module, const TypedModuleName& current_module, std::set<TypedModuleName>& dependencies) const;
                bool a_module_depends_on_itself();
                void update_dependencies();
                void add_dependencies_and_dependent_modules(const std::set<TypedSignalName>& required_signals, const TypedModuleName& module_using_required_signals);
                FromName2Module name2module; //!< Map giving, for each module name, a (smart) pointer to the corresponding module
                bool readonly; //!< If this flag is set to true, DataSource::set will not modify the state of the DataSource. This is used so that the first call to DataSourceModule::update (by DataSource::add) can track dependencies between modules
                SignalContainer signals_; //!< All signals currently in the DataSource
                TypedModuleName module_setting_signals; //!< Module currently adding signals to the DataSource (used to track if two different modules set the same signal)
                FromSignal2Module signal2module; //!< Tracks which module sets which signal
                DependantModules module2dependantmodules; //!< For each module, stores the set of the names of the modules depending on it
                DependantModules module2requiredmodules; //!< For each module, stores the set of the names of the modules it depends on
                DependantModules module2requiredsignals; //!< For each module, stores the signals it depends on
                DependantModules signal2dependantmodules; //!< For each signal, stores the modules that depend on it
                UpdateState is_up_to_date; //!< For each module, whether it is up-to-date or not
                std::vector<std::pair<std::string,std::string> > state_names;
                std::map<TypedSignalName,TypedSignalName> aliases; //!< Correspondence between an alias & the name of the original signal
                SignalContainer forced_values; //!< All signals that have been forced
                std::vector<TypedModuleName> modules; //!< All modules in DataSource
                std::stack<TypedSignalName> call_stack; //!< Names of modules in call stack (M1 called M2 called...)
                bool in_module; //!< True if the current setter is a DataSourceModule
        };
    }
}
#endif /* DATASOURCE_HPP_ */
