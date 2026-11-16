#include "test_data.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string test_data::path(const std::string& name)
{
    return std::string(XDYN_TEST_DATA_DIR) + "/" + name;
}

std::string test_data::read(const std::string& name)
{
    std::ifstream file(path(name), std::ios::binary);
    if (not file)
        throw std::runtime_error("Cannot open test fixture " + path(name));
    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}
