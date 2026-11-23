/*
 * force_model_tableTest.cpp
 */

#include <algorithm>
#include <string>

#include "force_model_table.hpp"
#include "gtest/gtest.h"

// Two models answering to one name would leave one of them unreachable, silently
TEST(find_force_model, every_name_is_unique)
{
    auto names = force_model_names();
    std::sort(names.begin(), names.end());
    const auto duplicate = std::adjacent_find(names.begin(), names.end());
    ASSERT_EQ(names.end(), duplicate) << "'" << *duplicate << "' names two force models";
}
