#include "RapidJSON.hpp"
#include "JSONException.hpp"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


using namespace ssc::json;

std::string ssc::json::print_type(const rapidjson::Value& v)
{
    static const char* kTypeNames[] =
          { "Null", "False", "True", "Object", "Array", "String", "Number" };
    return kTypeNames[v.GetType()];
}

double ssc::json::find_double(const std::string& key, const rapidjson::Value& v)
{
    if (not(v.IsObject()))
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Cannot find key '" << key << "' because the part of the JSON we're looking at is not an object: it's a " << print_type(v) << ". The JSON we're looking at was: " << dump(v));
    }
    static const char* kTypeNames[] =
          { "Null", "False", "True", "Object", "Array", "String", "Number" };
    rapidjson::Value::ConstMemberIterator itr = v.FindMember(key.c_str());
    if (itr != v.MemberEnd())
    {
        if (itr->value.IsDouble() || itr->value.IsNumber())
        {
            return itr->value.GetDouble();
        }
        else
        {
            THROW(__PRETTY_FUNCTION__, ssc::json::Exception, key << " should be a double, but it's a " << kTypeNames[itr->value.GetType()] << " in input JSON '" << dump(v) << "'");
        }
    }
    else
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Could not find key '" << key << "' in input JSON '" << dump(v) << "'");
    }
    return 0;
}

int ssc::json::find_int(const std::string& key, const rapidjson::Value& v)
{
    if (not(v.IsObject()))
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Cannot find key '" << key << "' because the part of the JSON we're looking at is not an object: it's a " << print_type(v) << ". The JSON we're looking at was: " << dump(v));
    }
    static const char* kTypeNames[] =
          { "Null", "False", "True", "Object", "Array", "String", "Number" };
    rapidjson::Value::ConstMemberIterator itr = v.FindMember(key.c_str());
    if (itr != v.MemberEnd())
    {
        if (itr->value.IsInt())
        {
            return itr->value.GetInt();
        }
        else
        {
            THROW(__PRETTY_FUNCTION__, ssc::json::Exception, key << " should be a int, but it's a " << kTypeNames[itr->value.GetType()] << " in input JSON '" << dump(v) << "'");
        }
    }
    else
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Could not find key '" << key << "' in input JSON '" << dump(v) << "'");
    }
    return 0;
}

std::string ssc::json::find_string(const std::string& key, const rapidjson::Value& v)
{
    if (not(v.IsObject()))
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Cannot find key '" << key << "' because the part of the JSON we're looking at is not an object: it's a " << print_type(v) << ". The JSON we're looking at was: " << dump(v));
    }
    static const char* kTypeNames[] =
          { "Null", "False", "True", "Object", "Array", "String", "Number" };
    rapidjson::Value::ConstMemberIterator itr = v.FindMember(key.c_str());
    if (itr != v.MemberEnd())
    {
        if (itr->value.IsString())
        {
            return itr->value.GetString();
        }
        else
        {
            THROW(__PRETTY_FUNCTION__, ssc::json::Exception, key << " should be a string, but it's a " << kTypeNames[itr->value.GetType()] << " in input JSON '" << dump(v) << "'");
        }
    }
    else
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Could not find key '" << key << "' in input JSON '" << dump(v) << "'");
    }
    return "";
}

std::string ssc::json::dump(const rapidjson::Value& v)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);
    return buffer.GetString();
}

void ssc::json::parse(const std::string& json, rapidjson::Document& document)
{
    const rapidjson::ParseResult result = document.Parse(json.c_str());
    if (!result)
    {
      THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "JSON parse error: " << GetParseError_En(result.Code()) << " (offset " << result.Offset() << ") in input JSON '" << json << "'");
    }
}

double ssc::json::find_optional_double(const std::string& key, const rapidjson::Value& v, const double default_value)
{
    if (not(v.IsObject()))
    {
        THROW(__PRETTY_FUNCTION__, ssc::json::Exception, "Cannot find key '" << key << "' because the part of the JSON we're looking at is not an object: it's a " << print_type(v) << ". The JSON we're looking at was: " << dump(v));
    }
    if (v.HasMember(key.c_str()))
    {
        return find_double(key, v);
    }
    return default_value;
}
