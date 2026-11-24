/*
 * BlockedDOFTests.cpp
 */

#include <cstdio>
#include <fstream>

#include "xdyn/core/Body.hpp"
#include "xdyn/core/SimulatorBuilder.hpp"
#include "xdyn/core/unit_tests/BlockedDOFTest.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/hydro_db/read_added_mass.hpp"
#include "xdyn/mesh/unit_tests/TriMeshTestData.hpp"
#include "xdyn/simulator_api/model_tables.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"

TEST_F(BlockedDOFTest, force_acceleration)
{
    // ARRANGE
    std::ofstream writeHDB("KVLCC2.hdb");
    writeHDB << test_data::read("hdb/test_ship.hdb");
    writeHDB.close();
    YamlSimulatorInput input =
        SimulatorYamlParser(
            test_data::read("yaml/kvlcc2.yml") + test_data::read("yaml/blockedDoF_OTT.yml"))
            .parse();
    input.bodies.front().dynamics.added_mass =
        read_added_mass(input.bodies.front().dynamics.added_mass);
    SimulatorBuilder builder(input, make_force_model, add_environment_model);
    EnvironmentAndFrames env = builder.build_environment_and_frames();
    MeshMap dummyMesh;
    dummyMesh[input.bodies.front().name] = two_triangles();
    std::vector<BodyPtr> bodies =
        builder.get_bodies(dummyMesh, std::vector<bool>(1, false), std::map<std::string, double>());
    builder.add_initial_transforms(bodies, env.k);
    StateType x = builder.get_initial_states();
    for (std::size_t i = 0; i < 13; ++i) {
        x[i] = 0.1 * static_cast<int>(i);
    }
    std::remove("KVLCC2.hdb");
    // ACT
    StateType dx;
    dx.resize(13);
    ssc::kinematics::Point O("NED");
    ssc::kinematics::Wrench zeroForce(O);
    double t(0.);
    bodies.front()->calculate_state_derivatives(zeroForce, x, dx, t, env);
    // ASSERT
    EXPECT_NEAR(dx[0], -0.488022, 0.000001);
    EXPECT_NEAR(dx[1], -0.108789, 0.000001);
    EXPECT_NEAR(dx[2], 0.5, 0.1);
    EXPECT_NEAR(dx[3], 8.53429, 0.0001);
    EXPECT_NEAR(dx[4], 144.5652, 0.001);
    EXPECT_NEAR(dx[5], 0.00051197, 0.0000001);
    EXPECT_NEAR(dx[6], -63.818, 0.001);
    EXPECT_NEAR(dx[7], 0.03566336, 0.0000001);
    EXPECT_NEAR(dx[8], -17.9366, 0.0001);
    EXPECT_NEAR(dx[9], -1.165, 0.001);
    EXPECT_NEAR(dx[10], 0.29, 0.01);
    EXPECT_NEAR(dx[11], 0.275, 0.001);
    EXPECT_NEAR(dx[12], 0.38, 0.01);
}

TEST_F(BlockedDOFTest, force_acceleration_w_idealRudder)
{
    // ARRANGE
    std::ofstream writeHDB("KVLCC2.hdb");
    writeHDB << test_data::read("hdb/test_ship.hdb");
    writeHDB.close();
    YamlSimulatorInput input =
        SimulatorYamlParser(
            test_data::read("yaml/kvlcc2.yml") + test_data::read("yaml/blockedDoF_idealRudder.yml"))
            .parse();
    input.bodies.front().dynamics.added_mass =
        read_added_mass(input.bodies.front().dynamics.added_mass);
    SimulatorBuilder builder(input, make_force_model, add_environment_model);
    EnvironmentAndFrames env = builder.build_environment_and_frames();
    MeshMap dummyMesh;
    dummyMesh[input.bodies.front().name] = two_triangles();
    std::vector<BodyPtr> bodies =
        builder.get_bodies(dummyMesh, std::vector<bool>(1, false), std::map<std::string, double>());
    builder.add_initial_transforms(bodies, env.k);
    StateType x = builder.get_initial_states();
    for (std::size_t i = 0; i < 13; ++i) {
        x[i] = 0.1 * static_cast<int>(i);
    }
    std::remove("KVLCC2.hdb");
    // ACT
    StateType dx;
    dx.resize(13);
    ssc::kinematics::Point O("NED");
    ssc::kinematics::Wrench zeroForce(O);
    double t(0.);
    bodies.front()->calculate_state_derivatives(zeroForce, x, dx, t, env);
    /* for (std::size_t i=0; i<13; ++i) {
        std::cout << "dx(" << i << "), " << dx[i] << std::endl;
    } */
    // ASSERT
    EXPECT_NEAR(dx[0], -0.488022, 0.000001);
    EXPECT_NEAR(dx[1], -0.108789, 0.000001);
    EXPECT_NEAR(dx[2], 0.5, 0.1);
    EXPECT_NEAR(dx[3], -0.000120195, 0.000001);
    EXPECT_NEAR(dx[4], 834.24, 0.001);
    EXPECT_NEAR(dx[5], 0.000470068, 0.00000001);
    EXPECT_NEAR(dx[6], 1.87238, 0.00001);
    EXPECT_NEAR(dx[7], 0.00305254, 0.00000001);
    EXPECT_NEAR(dx[8], -17.8714, 0.0001);
    EXPECT_NEAR(dx[9], -1.165, 0.001);
    EXPECT_NEAR(dx[10], 0.29, 0.01);
    EXPECT_NEAR(dx[11], 0.275, 0.001);
    EXPECT_NEAR(dx[12], 0.38, 0.01);
}
