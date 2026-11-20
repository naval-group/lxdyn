#include "ConfBuilderTest.hpp"

#include "ConfBuilder.hpp"
#include "xdyn/core/unit_tests/test_data.hpp"
#include "xdyn/mesh/unit_tests/TriMeshTestData.hpp"

ConfBuilderTest::ConfBuilderTest() : a(ssc::random_data_generator::DataGenerator(78910)) {}

TEST_F(ConfBuilderTest, compute_right_value_of_Tmax_falling_ball)
{
    ConfBuilder builder(test_data::read("tutorials/tutorial_01_falling_ball.yml"));
    ASSERT_EQ(builder.Tmax, 0);
}

TEST_F(ConfBuilderTest, can_build_with_mesh)
{
    ConfBuilder builder(test_data::read("tutorials/tutorial_01_falling_ball.yml"), unit_cube());
    ASSERT_FALSE(builder.sim.get_bodies().empty());
    ASSERT_GT(builder.sim.get_bodies().front()->get_states().mesh.use_count(), 0);
    ASSERT_GT(builder.sim.get_bodies().front()->get_states().mesh->nb_of_static_edges, 0);
}
