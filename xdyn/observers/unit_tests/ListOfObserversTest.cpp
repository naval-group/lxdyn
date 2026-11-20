#include "ListOfObserversTest.hpp"

#include "ListOfObservers.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/yaml_parser/parse_output.hpp"

TEST_F(ListOfObserversTest, should_be_able_to_create_a_list_of_observers)
{
    const std::vector<YamlOutput> yaml = parse_output(test_data::read("yaml/full_example.yml"));
    {
        ListOfObservers s(yaml);
    }
    for (const auto& output : yaml) {
        if (output.format == "hdf5") {
            EXPECT_EQ(0, remove(output.filename.c_str()));
        }
    }
}

TEST_F(ListOfObserversTest, an_unknown_format_suggests_the_closest_known_one)
{
    YamlOutput output;
    output.format = "jsn";
    std::string message;
    try {
        ListOfObservers::parse_observer(output);
    } catch (const InvalidInputException& e) {
        message = e.get_message();
    }
    EXPECT_NE(std::string::npos, message.find("output format 'jsn'")) << message;
    EXPECT_NE(std::string::npos, message.find("Did you mean 'json'?")) << message;
}
