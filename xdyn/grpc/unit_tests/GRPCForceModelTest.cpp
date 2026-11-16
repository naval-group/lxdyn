/*
 * GRPCForceModelTest.cpp
 *
 *  Created on: Jun 17, 2019
 *      Author: cady
 */

#include "GRPCForceModelTest.hpp"

#include "GRPCForceModel.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"

TEST_F(GRPCForceModelTest, can_parse_reference_frame)
{
    const auto input = GRPCForceModel::parse(test_data::read("yaml/gRPC_force_model.yml"));
    ASSERT_EQ("force-model:9002", input.url);
}

TEST_F(GRPCForceModelTest, can_parse_name)
{
    const auto input = GRPCForceModel::parse(test_data::read("yaml/gRPC_force_model.yml"));
    ASSERT_EQ("parametric oscillator", input.name);
}

TEST_F(GRPCForceModelTest, can_parse_rest_of_yaml_parameters)
{
    const auto input = GRPCForceModel::parse(test_data::read("yaml/gRPC_force_model.yml"));
    ASSERT_EQ(
        "name: parametric oscillator\n"
        "model: grpc\n"
        "url: force-model:9002\n"
        "k: 60\n"
        "c: 1",
        input.yaml);
}

TEST_F(GRPCForceModelTest, can_parse_hdb)
{
    const auto input =
        GRPCForceModel::parse(test_data::read("yaml/gRPC_force_model.yml") + "hdb: some_hdb.hdb\n");
    ASSERT_EQ("some_hdb.hdb", input.hdb_filename);
}

TEST_F(GRPCForceModelTest, can_parse_precal)
{
    const auto input = GRPCForceModel::parse(
        test_data::read("yaml/gRPC_force_model.yml") + "raodb: some_precal.ini\n");
    ASSERT_EQ("some_precal.ini", input.precal_filename);
}

TEST_F(GRPCForceModelTest, should_throw_if_both_hdb_and_precal_are_defined_in_any_order)
{
    ASSERT_THROW(
        GRPCForceModel::parse(
            test_data::read("yaml/gRPC_force_model.yml") + "hdb: some_hdb.hdb\n" +
            "raodb: some_precal.ini\n"),
        InvalidInputException);
    ASSERT_THROW(
        GRPCForceModel::parse(
            test_data::read("yaml/gRPC_force_model.yml") + "raodb: some_precal.ini\n" +
            "hdb: some_hdb.hdb\n"),
        InvalidInputException);
}
