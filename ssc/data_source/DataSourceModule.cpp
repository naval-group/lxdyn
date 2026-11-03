/*
 * DataSourceModule.cpp
 *
 * \date 15 mars 2013, 14:15:08
 * \author cec
 */


#include "ssc/data_source/DataSourceModule.hpp"

using namespace ssc::data_source;

DataSourceModule::DataSourceModule(DataSource* const data_source, const std::string& module_name) : ds(data_source), module_name_(module_name)
{

}

DataSourceModule::~DataSourceModule()
{
}

std::string DataSourceModule::get_name() const
{
    return module_name_;
}

void DataSourceModule::initialize() const
{

}
