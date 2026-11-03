#ifndef RAPIDJSONHPP
#define RAPIDJSONHPP

#include "rapidjson/document.h"
#include <string>

namespace ssc
{
    namespace json
    {
        std::string print_type(const rapidjson::Value& v);
        double find_double(const std::string& key, const rapidjson::Value& v);
        int find_int(const std::string& key, const rapidjson::Value& v);
        std::string find_string(const std::string& key, const rapidjson::Value& v);
        std::string dump(const rapidjson::Value& v);
        void parse(const std::string& json, rapidjson::Document& document);
        double find_optional_double(const std::string& key, const rapidjson::Value& v, const double default_value);
    }
}

#endif
