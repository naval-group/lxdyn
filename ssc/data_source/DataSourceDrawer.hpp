/*
 * DataSourceDrawer.hpp
 *
 * \date 29 oct. 2013, 08:38:28
 * \author cec
 */

#ifndef DATASOURCEDRAWER_HPP_
#define DATASOURCEDRAWER_HPP_

#include <set>
#include <string>
#include <map>

#include "ssc/data_source/DataSource.hpp"

namespace ssc
{
    namespace data_source
    {
        struct DSModuleDraw
        {
            DSModuleDraw() : inputs(std::set<std::string>()),outputs(std::set<std::string>()), type(""), name_in_graph(""){}
            std::set<std::string> inputs;
            std::set<std::string> outputs;
            std::string type;
            std::string name_in_graph;
        };

        struct DSSignalDraw
        {
            DSSignalDraw() : created_by(""),used_by(std::set<std::string>()),type(""){}
            std::string created_by;
            std::set<std::string> used_by;
            std::string type;
        };

        struct Connexion
        {
                Connexion() : start(""),end(""),edge("") {}
                std::string start;
                std::string end;
                std::string edge;
        };


        /** \author cec
         *  \ingroup data_source
         *  \brief Outputs the contents of a DataSource in a string
         *  \details
         *  \section ex1 Example
         *  \snippet data_source/unit_tests/src/DataSourceSerializerTest.cpp DataSourceSerializerTest example
         *  \section ex2 Expected output
         *  \snippet data_source/unit_tests/src/DataSourceSerializerTest.cpp DataSourceSerializerTest expected output
         */

        class DataSourceDrawer
        {
            public:
                DataSourceDrawer(const FromSignal2Module& signal2module,
                                     const DependantModules& dependant_modules,
                                     const SignalContainer& all_signals);

                std::string get_graph() const;
                std::string get_yaml() const;

            private:
                std::string serialize(const std::set<std::string>& s) const;
                DataSourceDrawer();
                std::map<std::string, DSModuleDraw> modules;
                std::map<std::string, DSSignalDraw> signals;
                std::vector<Connexion> connexions;
        };
    }
}
#endif /* DATASOURCEDRAWER_HPP_ */
