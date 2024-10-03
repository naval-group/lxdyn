/*
 * BlockedDOFTest.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: cady
 */

// Local
#include "BlockedDOFTest.hpp"
#include "xdyn/core/BlockedDOF.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/test_data_generator/yaml_data.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"
#include "xdyn/external_data_structures/YamlSimulatorInput.hpp"
#include "xdyn/test_data_generator/hdb_data.hpp"
#include "xdyn/test_data_generator/TriMeshTestData.hpp"
#include "xdyn/core/BodyBuilder.hpp"
#include "xdyn/core/Body.hpp"
#include "xdyn/core/SimulatorBuilder.hpp"
#include "xdyn/core/DefaultSurfaceElevation.hpp"
#include "xdyn/environment_models/DefaultWindModel.hpp"

// Third Party
#include "yaml.h"
#include <boost/filesystem.hpp> // For boost::filesystem::unique_path

//STL
#include <fstream>




BlockedDOFTest::BlockedDOFTest() : a(ssc::random_data_generator::DataGenerator(854512))
{
}

BlockedDOFTest::~BlockedDOFTest()
{
}

void BlockedDOFTest::SetUp()
{
}

void BlockedDOFTest::TearDown()
{
}

TEST_F(BlockedDOFTest, should_throw_if_forcing_same_state_twice)
{
    const std::string yaml1 = "blocked dof:\n"
                             "   from CSV:\n"
                             "     - state: u\n"
                             "       t: T\n"
                             "       value: PS\n"
                             "       interpolation: spline\n"
                             "       filename: test.csv\n"
                             "     - state: u\n"
                             "       t: T\n"
                             "       value: PS\n"
                             "       interpolation: spline\n"
                             "       filename: test.csv\n";
    const std::string yaml2 =
                             "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: p\n"
                             "       t: [4.2]\n"
                             "       value: [5]\n"
                             "       interpolation: piecewise constant\n"
                             "     - state: p\n"
                             "       t: [4.2]\n"
                             "       value: [5]\n"
                             "       interpolation: piecewise constant\n";
    const std::string yaml3 = "blocked dof:\n"
                             "   from CSV:\n"
                             "     - state: u\n"
                             "       t: T\n"
                             "       value: PS\n"
                             "       interpolation: spline\n"
                             "       filename: test.csv\n"
                             "   from YAML:\n"
                             "     - state: u\n"
                             "       t: [4.2]\n"
                             "       value: [5]\n"
                             "       interpolation: piecewise constant\n";

    ASSERT_THROW(BlockedDOF b(yaml1), InvalidInputException);
    ASSERT_THROW(BlockedDOF b(yaml2), InvalidInputException);
    ASSERT_THROW(BlockedDOF b(yaml3), InvalidInputException);
}

TEST_F(BlockedDOFTest, should_throw_if_not_as_many_values_as_instants)
{
    const std::string invalid_yaml =
                             "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: p\n"
                             "       t: [4.2,5]\n"
                             "       value: [5]\n"
                             "       interpolation: piecewise constant\n";
    ASSERT_THROW(BlockedDOF b(invalid_yaml), InvalidInputException);
}

TEST_F(BlockedDOFTest, should_throw_if_t_not_strictly_increasing)
{
    const std::string invalid_yaml =
                             "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: p\n"
                             "       t: [5,4.2]\n"
                             "       value: [5,6]\n"
                             "       interpolation: piecewise constant\n";
    ASSERT_THROW(BlockedDOF b(invalid_yaml), InvalidInputException);
}

TEST_F(BlockedDOFTest, should_throw_if_CSV_file_does_not_exist)
{
    const std::string yaml = "blocked dof:\n"
                             "   from CSV:\n"
                             "     - state: u\n"
                             "       t: T\n"
                             "       value: PS\n"
                             "       interpolation: spline\n"
                             "       filename: test.csv\n";
    ASSERT_THROW(BlockedDOF b(yaml), InvalidInputException);
}

struct TmpFile
{
    TmpFile() : path(boost::filesystem::unique_path())
    {
    }

    ~TmpFile()
    {
        if (boost::filesystem::exists(path)) boost::filesystem::remove(path);
    }

    TmpFile& operator<<(const std::string& s)
    {
        std::ofstream of(path.string(), std::ofstream::out | std::ofstream::app);
        of << s;
        of.close();
        return *this;
    }

    std::string get_filename() const
    {
        return path.string();
    }

    private:
        TmpFile(const TmpFile& rhs);
        TmpFile& operator=(const TmpFile& rhs);
        boost::filesystem::path path;
};


TEST_F(BlockedDOFTest, should_not_throw_if_CSV_file_exists)
{
    TmpFile csv_file;
    const std::string yaml = "blocked dof"
                             "   from CSV:\n"
                             "     - state: u\n"
                             "       t: T\n"
                             "       value: PS\n"
                             "       interpolation: spline\n"
                             "       filename: " + csv_file.get_filename();
    const std::string csv = "T,PS\n"
                            "1,2\n";
    csv_file << csv;
    ASSERT_NO_THROW(BlockedDOF b(yaml));
}

TEST_F(BlockedDOFTest, piecewise_constant_should_work)
{
    const std::string yaml = "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: p\n"
                             "       t: [1,4.2,5]\n"
                             "       value: [1,2,3]\n"
                             "       interpolation: piecewise constant\n";
    StateType x(13);
    const BlockedDOF blocker(yaml);
    blocker.force_states(x,1);
    ASSERT_DOUBLE_EQ(1, x[PIDX(0)]);
    blocker.force_states(x,2.6);
    ASSERT_DOUBLE_EQ(1, x[PIDX(0)]);
    blocker.force_states(x,4.2);
    ASSERT_DOUBLE_EQ(2, x[PIDX(0)]);
    blocker.force_states(x,4.6);
    ASSERT_DOUBLE_EQ(2, x[PIDX(0)]);
    blocker.force_states(x,5);
    ASSERT_DOUBLE_EQ(3, x[PIDX(0)]);
}

TEST_F(BlockedDOFTest, linear_should_work)
{
    const std::string yaml = "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: u\n"
                             "       t: [1,4.2,5]\n"
                             "       value: [1,2,3]\n"
                             "       interpolation: linear\n";
    StateType x(13);
    const BlockedDOF blocker(yaml);
    blocker.force_states(x,1);
    ASSERT_DOUBLE_EQ(1, x[UIDX(0)]);
    blocker.force_states(x,2.6);
    ASSERT_DOUBLE_EQ(1.5, x[UIDX(0)]);
    blocker.force_states(x,4.2);
    ASSERT_DOUBLE_EQ(2, x[UIDX(0)]);
    blocker.force_states(x,4.6);
    ASSERT_DOUBLE_EQ(2.5, x[UIDX(0)]);
    blocker.force_states(x,5);
    ASSERT_DOUBLE_EQ(3, x[UIDX(0)]);
}

TEST_F(BlockedDOFTest, spline_should_work)
{
    const std::string yaml = "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: q\n"
                             "       t: [1,4.2,5]\n"
                             "       value: [1,2,3]\n"
                             "       interpolation: spline\n";
    StateType x(13);
    const BlockedDOF blocker(yaml);
    blocker.force_states(x,1);
    ASSERT_DOUBLE_EQ(1, x[QIDX(0)]);
    blocker.force_states(x,2.6);
    ASSERT_NEAR(0.9,    x[QIDX(0)], 1e-6);
    blocker.force_states(x,4.2);
    ASSERT_DOUBLE_EQ(2, x[QIDX(0)]);
    blocker.force_states(x,4.6);
    ASSERT_NEAR(2.4625, x[QIDX(0)], 1e-6);
    blocker.force_states(x,5);
    ASSERT_DOUBLE_EQ(3, x[QIDX(0)]);
}

TEST_F(BlockedDOFTest, blocked_derivative_should_work)
{
    const std::string yaml = "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: u\n"
                             "       t: [1,4.2,5]\n"
                             "       value: [1,2,3]\n"
                             "       interpolation: linear\n";
    StateType dx_dt(13);
    const BlockedDOF blocker(yaml);
    blocker.force_state_derivatives(dx_dt,1);
    ASSERT_DOUBLE_EQ(1./3.2, dx_dt[UIDX(0)]);
    blocker.force_state_derivatives(dx_dt,2.6);
    ASSERT_DOUBLE_EQ(1./3.2, dx_dt[UIDX(0)]);
    blocker.force_state_derivatives(dx_dt,4.2);
    ASSERT_DOUBLE_EQ(1./0.8, dx_dt[UIDX(0)]);
    blocker.force_state_derivatives(dx_dt,4.6);
    ASSERT_DOUBLE_EQ(1./0.8, dx_dt[UIDX(0)]);
    blocker.force_state_derivatives(dx_dt,5);
    ASSERT_DOUBLE_EQ(1./0.8, dx_dt[UIDX(0)]);
}

TEST_F(BlockedDOFTest, force_delta)
{
    const std::string yaml = "blocked dof:\n"
                             "   from YAML:\n"
                             "     - state: u\n"
                             "       t: [1,4.2,5]\n"
                             "       value: [1,2,3]\n"
                             "       interpolation: linear\n";
    const StateType dx_dt = {10,11,12,13,14,15,16,17,18,19,20,21,23};
    const BlockedDOF blocker(yaml);
    Eigen::Matrix<double,6,6> total_inertia;
    total_inertia << 1, 2, 3, 4, 5, 6,
                     7, 8, 9, 0, 1, 2,
                     3, 4, 5, 6, 7, 8,
                     9, 0, 1, 2, 3, 4,
                     5, 6, 7, 8, 9, 0,
                     1, 2, 3, 4, 5, 6;
    Eigen::Vector3d F,T;
    F(0) = 50.0;
    F(1) = 51.0;
    F(2) = 52.0;
    T(0) = 60.0;
    T(1) = 61.0;
    T(2) = 62.0;
    ssc::kinematics::Wrench sum_of_other_forces(ssc::kinematics::Point("d",1,2,3),F,T);
    const auto delta_F = blocker.get_delta_F(dx_dt,total_inertia,sum_of_other_forces);
    EXPECT_DOUBLE_EQ(293.0, delta_F(0));
    EXPECT_DOUBLE_EQ(340.0, delta_F(1));
    EXPECT_DOUBLE_EQ(477.0, delta_F(2));
    EXPECT_DOUBLE_EQ(227.0, delta_F(3));
    EXPECT_DOUBLE_EQ(474.0, delta_F(4));
    EXPECT_DOUBLE_EQ(281.0, delta_F(5));
}

TEST_F(BlockedDOFTest, define_matrix_of_forced_DoF)
{
    // ARRANGE
    std::ofstream writeHDB("KVLCC2.hdb");
    writeHDB << test_data::KVLCC2_hdb();
    writeHDB.close();
    YamlSimulatorInput yaml = SimulatorYamlParser(test_data::kvlcc2()+test_data::blockedDoF_OTT()).parse();
    std::remove("KVLCC2.hdb");
    // ACT
    BlockedDOF blocked_states((yaml.bodies.front()).blocked_dof,0);
    Eigen::Matrix<double,6,6> m_delta_x = blocked_states.get_delta_x(0.);
    Eigen::Matrix<double,6,6> m_delta_x_t1500 = blocked_states.get_delta_x(1500.);
    // ASSERT
    EXPECT_EQ(m_delta_x(0,0), 1.);
    EXPECT_EQ(m_delta_x(1,1), 1.);
    EXPECT_EQ(m_delta_x(2,2), 0.);
    EXPECT_EQ(m_delta_x(3,3), 1.);
    EXPECT_EQ(m_delta_x(4,4), 0.);
    EXPECT_EQ(m_delta_x(5,5), 1.);
    EXPECT_EQ(m_delta_x_t1500(0,0), 0.);
    EXPECT_EQ(m_delta_x_t1500(1,1), 0.);
    EXPECT_EQ(m_delta_x_t1500(2,2), 0.);
    EXPECT_EQ(m_delta_x_t1500(3,3), 0.);
    EXPECT_EQ(m_delta_x_t1500(4,4), 0.);
    EXPECT_EQ(m_delta_x_t1500(5,5), 0.);
    /*
    BodyBuilder builder(yaml.rotations);
    VectorOfVectorOfPoints mesh;
    BodyPtr KVLCC2 = builder.build(yaml.bodies.front(), mesh, 0, 0., yaml.rotations, 100.);
    */
}

TEST_F(BlockedDOFTest, force_acceleration)
{
    // ARRANGE
    std::ofstream writeHDB("KVLCC2.hdb");
    writeHDB << test_data::KVLCC2_hdb();
    writeHDB.close();
    YamlSimulatorInput input = SimulatorYamlParser(test_data::kvlcc2()+test_data::blockedDoF_OTT()).parse();
    SimulatorBuilder builder(input);
    builder.can_parse<DefaultSurfaceElevation>();
    builder.can_parse<DefaultWindModel>();
    EnvironmentAndFrames env = builder.build_environment_and_frames();
    MeshMap dummyMesh;
    dummyMesh[input.bodies.front().name] = two_triangles();
    std::vector<BodyPtr> bodies = builder.get_bodies(dummyMesh, std::vector<bool>(1,false), std::map<std::string,double>());
    builder.add_initial_transforms(bodies,env.k);
    StateType x = builder.get_initial_states();
    for (std::size_t i=0; i<13; ++i) {
        x[i] = 0.1*static_cast<int>(i);
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
    EXPECT_NEAR(dx[3], 11.9752, 0.0001);
    EXPECT_NEAR(dx[4], 140.078, 0.001);
    EXPECT_NEAR(dx[5], 0.0795591, 0.0000001);
    EXPECT_NEAR(dx[6], -63.617, 0.001);
    EXPECT_NEAR(dx[7], 0.0260779, 0.0000001);
    EXPECT_NEAR(dx[8], -17.9174, 0.0001);
    EXPECT_NEAR(dx[9], -1.165, 0.001);
    EXPECT_NEAR(dx[10], 0.29, 0.01);
    EXPECT_NEAR(dx[11], 0.275, 0.001);
    EXPECT_NEAR(dx[12], 0.38, 0.01);
}

TEST_F(BlockedDOFTest, define_matrix_of_forcing_force_according_to_user_info)
{
    // ARRANGE
    std::ofstream writeHDB("KVLCC2.hdb");
    writeHDB << test_data::KVLCC2_hdb();
    writeHDB.close();
    YamlSimulatorInput yaml = SimulatorYamlParser(test_data::kvlcc2()+test_data::blockedDoF_idealRudder()).parse();
    std::remove("KVLCC2.hdb");
    // ACT
    BlockedDOF blocked_states((yaml.bodies.front()).blocked_dof,0);
    Eigen::Matrix<double,6,6> m_delta_x = blocked_states.get_delta_x(0.);
    Eigen::Matrix<double,6,6> m_T_x = blocked_states.get_T_x();
    // ASSERT
    EXPECT_EQ(m_delta_x(0,0), 0.);
    EXPECT_EQ(m_delta_x(1,1), 0.);
    EXPECT_EQ(m_delta_x(2,2), 0.);
    EXPECT_EQ(m_delta_x(3,3), 0.);
    EXPECT_EQ(m_delta_x(4,4), 0.);
    EXPECT_EQ(m_delta_x(5,5), 1.);
    EXPECT_EQ(m_T_x(0,0), 0.);
    EXPECT_EQ(m_T_x(1,1), 0.);
    EXPECT_EQ(m_T_x(2,2), 0.);
    EXPECT_EQ(m_T_x(3,3), 0.);
    EXPECT_EQ(m_T_x(4,4), 0.);
    EXPECT_EQ(m_T_x(1,5), 1.);
    EXPECT_EQ(m_T_x(5,5), -170.);
}

TEST_F(BlockedDOFTest, force_acceleration_w_idealRudder)
{
    // ARRANGE
    std::ofstream writeHDB("KVLCC2.hdb");
    writeHDB << test_data::KVLCC2_hdb();
    writeHDB.close();
    YamlSimulatorInput input = SimulatorYamlParser(test_data::kvlcc2()+test_data::blockedDoF_idealRudder()).parse();
    SimulatorBuilder builder(input);
    builder.can_parse<DefaultSurfaceElevation>();
    builder.can_parse<DefaultWindModel>();
    EnvironmentAndFrames env = builder.build_environment_and_frames();
    MeshMap dummyMesh;
    dummyMesh[input.bodies.front().name] = two_triangles();
    std::vector<BodyPtr> bodies = builder.get_bodies(dummyMesh, std::vector<bool>(1,false), std::map<std::string,double>());
    builder.add_initial_transforms(bodies,env.k);
    StateType x = builder.get_initial_states();
    for (std::size_t i=0; i<13; ++i) {
        x[i] = 0.1*static_cast<int>(i);
    }
    std::remove("KVLCC2.hdb");
    // ACT
    StateType dx;
    dx.resize(13);
    ssc::kinematics::Point O("NED");
    ssc::kinematics::Wrench zeroForce(O);
    double t(0.);
    bodies.front()->calculate_state_derivatives(zeroForce, x, dx, t, env);
    for (std::size_t i=0; i<13; ++i) {
        std::cout << "dx(" << i << "), " << dx[i] << std::endl;
    }
    // ASSERT
    EXPECT_NEAR(dx[0], -0.488022, 0.000001);
    EXPECT_NEAR(dx[1], -0.108789, 0.000001);
    EXPECT_NEAR(dx[2], 0.5, 0.1);
    EXPECT_NEAR(dx[3], 0.0142606, 0.000001);
    EXPECT_NEAR(dx[4], 787.42, 0.001);
    EXPECT_NEAR(dx[5], -0.00246338, 0.00000001);
    EXPECT_NEAR(dx[6], 0.93661, 0.00001);
    EXPECT_NEAR(dx[7], 0.00195871, 0.00000001);
    EXPECT_NEAR(dx[8], -17.8692, 0.0001);
    EXPECT_NEAR(dx[9], -1.165, 0.001);
    EXPECT_NEAR(dx[10], 0.29, 0.01);
    EXPECT_NEAR(dx[11], 0.275, 0.001);
    EXPECT_NEAR(dx[12], 0.38, 0.01);
}