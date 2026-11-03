/*
 * TestModules.hpp
 *
 * \date 19 juin 2013, 08:55:24
 * \author cec
 */

#ifndef TESTMODULES_HPP_
#define TESTMODULES_HPP_

#include "ssc/data_source/DataSourceModule.hpp"
#include "ssc/data_source/DataSource.hpp"

DECLARE_MODULE(ModuleDependingOnItself);
void ModuleDependingOnItself::update() const
{
    size_t nb_of_updates = ds->get<size_t>("nb_of_updates");
    ds->set<size_t>("nb_of_updates", nb_of_updates+1);
}

DECLARE_MODULE(TestModule);
void TestModule::update() const
{
    size_t nb_of_updates = ds->get<size_t>("nb_of_updates");
    ds->set<size_t>("nb_of_updates2", nb_of_updates+1);
}

DECLARE_MODULE(ModuleA);
void ModuleA::update() const
{
    size_t nb_of_updates = ds->get<size_t>("C");
    ds->set<size_t>("A", nb_of_updates+1);
}

DECLARE_MODULE(ModuleB);
void ModuleB::update() const
{
    size_t nb_of_updates = ds->get<size_t>("A");
    ds->set<size_t>("B", nb_of_updates+1);
}

DECLARE_MODULE(ModuleC);
void ModuleC::update() const
{
    size_t nb_of_updates = ds->get<size_t>("B");
    ds->set<size_t>("C", nb_of_updates+1);
}

DECLARE_MODULE(ModuleD);
void ModuleD::update() const
{
    ds->get<size_t>("B");
    ds->set<double>("B", 1);
}





#endif  /* TESTMODULES_HPP_ */
