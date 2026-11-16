#include "ListOfObserversTest.hpp"

#include "ListOfObservers.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
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
