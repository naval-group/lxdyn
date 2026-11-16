/*
 * run_all_tests.cpp
 *
 *  Created on: Aug 19, 2020
 *      Author: cady
 */

#include <google/protobuf/stubs/common.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <random>
#include <string>

namespace {
// What a test writes then depends neither on where the suite is started nor on earlier tests.
class TemporaryWorkingDirectory : public ::testing::EmptyTestEventListener {
public:
    explicit TemporaryWorkingDirectory(const std::string& runner)
    {
        // Two checkouts can run the same suite at once.
        std::random_device random;
        do {
            root = std::filesystem::path(::testing::TempDir()) /
                   (runner + "-" + std::to_string(random()));
        } while (!std::filesystem::create_directories(root));
    }

    void OnTestStart(const ::testing::TestInfo& test) override
    {
        std::string name = std::string(test.test_suite_name()) + "." + test.name();
        std::replace(name.begin(), name.end(), '/', '_');
        const auto dir = root / name;
        std::filesystem::create_directories(dir);
        previous = std::filesystem::current_path();
        std::filesystem::current_path(dir);
    }

    void OnTestEnd(const ::testing::TestInfo&) override
    {
        std::filesystem::current_path(previous);
    }

    void OnTestProgramEnd(const ::testing::UnitTest&) override
    {
        std::filesystem::remove_all(root);
    }

private:
    std::filesystem::path root;
    std::filesystem::path previous;
};
}  // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::UnitTest::GetInstance()->listeners().Append(
        new TemporaryWorkingDirectory(std::filesystem::path(argv[0]).stem().string()));
    const int ret = RUN_ALL_TESTS();
    google::protobuf::ShutdownProtobufLibrary();
    return ret;
}
