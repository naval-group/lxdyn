#include "ssc/data_source/DataSourceModule.hpp"

// Defined so we can derive several types of mock
class MockableDataSourceModule : public ssc::data_source::DataSourceModule
{
    public:
        MockableDataSourceModule();
};

// Used when we don't need to track the calls to get_name (because get_name has
// a perfectly suitable default implementation & doesn't usually need to be
// overridden).
class MockDataSourceModule : public MockableDataSourceModule
{
    public:
        MockDataSourceModule()
        {

        }
        MockDataSourceModule(ssc::data_source::DataSource* const d, const std::string& s)
        {
            (void)d;
            (void)s;
        }
        MOCK_CONST_METHOD0(clone, ssc::data_source::DataSourceModule*());
        MOCK_CONST_METHOD1(clone, ssc::data_source::DataSourceModule*(ssc::data_source::DataSource* const));
        MOCK_CONST_METHOD0(update, void());
        MOCK_CONST_METHOD0(initialize, void());
};


// Used to track the calls to get_name (get_name has been artificially rendered
// virtual for this reason only)
class MockDataSourceModuleWithGetName : public MockableDataSourceModule
{
    public:
        MOCK_CONST_METHOD0(clone, ssc::data_source::DataSourceModule*());
        MOCK_CONST_METHOD1(clone, ssc::data_source::DataSourceModule*(ssc::data_source::DataSource* const));
        MOCK_CONST_METHOD0(update, void());
        MOCK_CONST_METHOD0(get_name, std::string());
        MOCK_CONST_METHOD0(initialize, void());
};
