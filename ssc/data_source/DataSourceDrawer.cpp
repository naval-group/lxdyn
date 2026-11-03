/*
 * DataSourceSerializer.cpp
 *
 * \date 29 oct. 2013, 08:38:28
 * \author cec
 */

#include "ssc/data_source/DataSourceDrawer.hpp"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <map>

using namespace ssc::data_source;

std::string DataSourceDrawer::serialize(const std::set<std::string>& s) const
{
    std::stringstream ss;
    size_t n = s.size();
    size_t i = 0;
    ss << "[";
    for (std::set<std::string>::const_iterator it = s.begin() ; it != s.end() ; ++it)
    {
        ss << *it;
        if (i++ != (n-1))
        {
            ss << ", ";
        }
    }
    ss << "]" << std::endl;
    return ss.str();
}

DataSourceDrawer::DataSourceDrawer(const FromSignal2Module& signal2module,
                                           const DependantModules& dependant_modules,
                                           const SignalContainer& all_signals) : modules(std::map<std::string, DSModuleDraw>()),
                                                                                        signals(std::map<std::string, DSSignalDraw>()),
                                                                                        connexions(std::vector<Connexion>())
{
    for (FromSignal2Module::const_iterator it = signal2module.begin() ; it != signal2module.end() ; ++it)
    {
        std::stringstream ss;
        if (it->second.get_type_name() != "unknown")
        {
            modules[it->second.get_signal_name()].outputs.insert(it->first.get_signal_name());
            modules[it->second.get_signal_name()].type = it->second.get_type_name();
            ss << it->second.get_signal_name();
        }
        else
        {
            ss << it->second;
        }

        signals[it->first.get_signal_name()].created_by = ss.str();
        signals[it->first.get_signal_name()].type = it->first.get_type_name();

    }
    for (DependantModules::const_iterator it = dependant_modules.begin() ; it != dependant_modules.end() ; ++it)
    {
        for (std::set<TypedSignalName>::const_iterator it2 = it->second.begin() ; it2 != it->second.end() ; ++it2)
        {
            if (it->first.get_type_name() != "unknown")
            {
                modules[it->first.get_signal_name()].inputs.insert(it2->get_signal_name());
                modules[it->first.get_signal_name()].type = it->first.get_type_name();
            }
            signals[it2->get_signal_name()].used_by.insert(it->first.get_signal_name());
            signals[it2->get_signal_name()].type = it2->get_type_name();
        }
    }
    size_t i = 0;
    for(std::map<std::string, DSModuleDraw>::iterator it = modules.begin() ; it!= modules.end() ; ++it)
    {
        std::stringstream ss;
        ss << "M" << i++;
        it->second.name_in_graph = ss.str();
    }
    const Signals s = all_signals.get_all_signals();
    for (Signals::const_iterator it = s.begin() ; it != s.end() ; ++it)
    {
        const SignalName signal_name = it->first.get_signal_name();
        std::map<std::string, DSSignalDraw>::const_iterator found = signals.find(signal_name);
        if (found != signals.end())
        {
            if (found->second.type != it->first.get_type_name())
            {
                signals[it->first.get_signal_name()].type = it->first.get_type_name();
            }
        }
        else
        {
            signals[it->first.get_signal_name()].type = it->first.get_type_name();
        }
    }
}

std::string escape(std::string data);
std::string escape(std::string data)
{
    using boost::algorithm::replace_all;
    replace_all(data, "&",  " and ");
    replace_all(data, "\"", "&quot;");
    //replace_all(data, "\'", "&apos;");
    replace_all(data, "<",  "&lt;");
    replace_all(data, ">",  "&gt;");
    replace_all(data, "�",  "e");
    replace_all(data, "�",  "e");
    replace_all(data, "�",  "a");
    replace_all(data, "�",  "e");


    return data;
}

std::string DataSourceDrawer::get_graph() const
{
    std::stringstream ss;
    ss << "digraph data_source {" << std::endl;
    ss << "\tUser [style=filled,color=\".7 .3 1.0\"];" << std::endl;
    for(std::map<std::string, DSModuleDraw>::const_iterator it = modules.begin() ; it!= modules.end() ; ++it)
    {
        ss << "\t" << it->second.name_in_graph << "[shape=box,style=filled,color=lightgrey,label=<<b>" << escape(it->first) << "</b> (<I>" << it->second.type << "</I>)>];" << std::endl;
    }
    ss << std::endl;
    for (std::map<std::string, DSSignalDraw>::const_iterator it = signals.begin() ; it != signals.end() ; ++it)
    {
        const std::string signal_name = it->first;
        for (std::set<std::string>::const_iterator that_consumer = it->second.used_by.begin() ; that_consumer != it->second.used_by.end() ; ++that_consumer)
        {
            const std::string signal_type = it->second.type;
            std::map<std::string, DSModuleDraw>::const_iterator it1 = modules.find(it->second.created_by);
            std::map<std::string, DSModuleDraw>::const_iterator it2 = modules.find(*that_consumer);
            const std::string M1 = it->second.created_by=="" ? "User" : it1->second.name_in_graph;
            const std::string M2 = it2->second.name_in_graph;
            ss << "\t" << escape(M1) << " -> " << escape(M2) << " [label=<<b>" << escape(signal_name) << "</b> (<I>" << signal_type << "</I>)>" << "];" << std::endl;
        }
    }
    for(std::map<std::string, DSModuleDraw>::const_iterator it = modules.begin() ; it!= modules.end() ; ++it)
    {
        const std::set<std::string> S = it->second.outputs;
        for (std::set<std::string>::const_iterator s = S.begin() ; s != S.end() ; ++s)
        {
            const std::map<std::string, DSSignalDraw>::const_iterator it2 = signals.find(*s);
            if (it2->second.used_by.empty())
            {
                ss << "\t" << it->second.name_in_graph << "M[shape=none,label=<<b>" << escape(*s) << "</b> (<I>" << it2->second.type << "</I>)>" << "];" << std::endl;
                ss << "\t" << it->second.name_in_graph << " -> " << it->second.name_in_graph << "M;" << std::endl;
            }
        }
    }
    ss << "}";
    return ss.str();
}

std::string DataSourceDrawer::get_yaml() const
{
    std::stringstream ss;
    ss << "data_source:" << std::endl;
    ss << "    modules:" << std::endl;
    for(std::map<std::string, DSModuleDraw>::const_iterator it = modules.begin() ; it!= modules.end() ; ++it)
    {
        ss << "      - name: " << it->first << std::endl;
        ss << "        type: " << it->second.type << std::endl;
        ss << "        inputs: " << serialize(it->second.inputs);
        ss << "        outputs: " << serialize(it->second.outputs);
    }
    ss << "    signals:" << std::endl;
    for(std::map<std::string, DSSignalDraw>::const_iterator it = signals.begin() ; it!= signals.end() ; ++it)
    {
        ss << "      - name: " << it->first << std::endl;
        ss << "        type: " << it->second.type << std::endl;
        ss << "        created by: ";
        //if (it->second.created_by.empty()) ss << "DataSource user";
        //else
            ss << it->second.created_by;
        ss << std::endl;
        ss << "        used by: " << serialize(it->second.used_by);
    }
    return ss.str();
}
