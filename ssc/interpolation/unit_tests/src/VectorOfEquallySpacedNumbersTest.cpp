/*
 * VectorOfEquallySpacedNumbersTest.cpp
 *
 * \date 31 juil. 2012, 09:07:26
 * \author cec
 */

#include "VectorOfEquallySpacedNumbersTest.hpp"
#include "ssc/interpolation/VectorOfEquallySpacedNumbers.hpp"
#include "ssc/interpolation/VectorOfEquallySpacedNumbersException.hpp"

using namespace ssc::interpolation;
using namespace ssc::random_data_generator;

VectorOfEquallySpacedNumbersTest::VectorOfEquallySpacedNumbersTest() : a(ssc::random_data_generator::DataGenerator(666558882)),
eps(1E-10)
{
}

VectorOfEquallySpacedNumbersTest::~VectorOfEquallySpacedNumbersTest()
{
}

void VectorOfEquallySpacedNumbersTest::SetUp()
{
}

void VectorOfEquallySpacedNumbersTest::TearDown()
{
}

namespace ssc
{
    namespace random_data_generator
    {
        template<> VectorOfEquallySpacedNumbers get_min_bound<VectorOfEquallySpacedNumbers>()
        {
            return VectorOfEquallySpacedNumbers(0, 1, 2);
        }

        template<> VectorOfEquallySpacedNumbers get_max_bound<VectorOfEquallySpacedNumbers>()
        {
            return VectorOfEquallySpacedNumbers(0, 1, 2);
        }

        template <> VectorOfEquallySpacedNumbers TypedScalarDataGenerator<VectorOfEquallySpacedNumbers>::get() const
        {
            const double min_bound = random<double>().between(-1e5,1e5);
            const double max_bound = random<double>().greater_than(min_bound).but_not().greater_than(1e5);
            const size_t nb_of_values = random<size_t>().no().greater_than(1e4);
            return VectorOfEquallySpacedNumbers(min_bound, max_bound, nb_of_values);
        }
    }
}

TEST_F(VectorOfEquallySpacedNumbersTest, example)
{
//! [VectorOfEquallySpacedNumbersTest example]
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    VectorOfEquallySpacedNumbers v(min_bound, max_bound, nb_of_values);
//! [VectorOfEquallySpacedNumbersTest example]
//! [VectorOfEquallySpacedNumbersTest expected output]
    ASSERT_EQ(nb_of_values, v.get().size());
    ASSERT_DOUBLE_EQ(min_bound, v.get().front());
    ASSERT_DOUBLE_EQ(max_bound, v.get().back());
//! [VectorOfEquallySpacedNumbersTest expected output]
}

TEST_F(VectorOfEquallySpacedNumbersTest, elements_should_be_in_strictly_increasing_order)
{
    VectorOfEquallySpacedNumbers vv = a.random<VectorOfEquallySpacedNumbers>();
    std::vector<double> v = vv.get();
    for (size_t i = 0 ; i<v.size()-1 ; ++i)
    {
        ASSERT_GT(v.at(i+1),v.at(i));
    }
}

TEST_F(VectorOfEquallySpacedNumbersTest, elements_should_be_separated_by_a_constant_interval)
{
    VectorOfEquallySpacedNumbers vv = a.random<VectorOfEquallySpacedNumbers>();
    std::vector<double> v = vv.get();
    const double length_of_first_interval = v.at(1) - v.at(0);
    for (size_t i = 1 ; i<v.size()-1 ; ++i)
    {
        ASSERT_NEAR(length_of_first_interval, v.at(i+1)-v.at(i),eps);
    }
}

TEST_F(VectorOfEquallySpacedNumbersTest, constructor_should_throw_if_min_greater_than_max)
{
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    ASSERT_THROW(VectorOfEquallySpacedNumbers(max_bound, min_bound, nb_of_values), VectorOfEquallySpacedNumbersException);
}

TEST_F(VectorOfEquallySpacedNumbersTest, constructor_should_throw_if_too_few_elements)
{
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1);
    ASSERT_THROW(VectorOfEquallySpacedNumbers(min_bound, max_bound, nb_of_values), VectorOfEquallySpacedNumbersException);
}

TEST_F(VectorOfEquallySpacedNumbersTest, constructor_should_throw_if_too_many_elements)
{
    std::vector<double> vector_of_doubles;
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().greater_than(vector_of_doubles.max_size());
    ASSERT_THROW(VectorOfEquallySpacedNumbers(min_bound, max_bound, nb_of_values), VectorOfEquallySpacedNumbersException);
}

TEST_F(VectorOfEquallySpacedNumbersTest, should_have_a_method_to_get_difference_between_two_consecutive_numbers)
{
    std::vector<double> vector_of_doubles;
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    VectorOfEquallySpacedNumbers v(min_bound, max_bound, nb_of_values);
    ASSERT_NEAR((max_bound-min_bound)/(double(nb_of_values)-1),v.get_delta(), eps);
}

TEST_F(VectorOfEquallySpacedNumbersTest, should_have_a_method_to_get_the_number_of_values_in_vector)
{
    std::vector<double> vector_of_doubles;
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    VectorOfEquallySpacedNumbers v(min_bound, max_bound, nb_of_values);
    ASSERT_EQ(nb_of_values,v.size());
}

TEST_F(VectorOfEquallySpacedNumbersTest, should_have_a_method_to_get_the_min_value)
{
    std::vector<double> vector_of_doubles;
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    VectorOfEquallySpacedNumbers v(min_bound, max_bound, nb_of_values);
    ASSERT_DOUBLE_EQ(min_bound,v.get_min());
}

TEST_F(VectorOfEquallySpacedNumbersTest, should_have_a_method_to_get_the_max_value)
{
    std::vector<double> vector_of_doubles;
    const double min_bound = a.random<double>().between(-1e5,1e5);
    const double max_bound = a.random<double>().greater_than(min_bound).but_not().greater_than(1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    VectorOfEquallySpacedNumbers v(min_bound, max_bound, nb_of_values);
    ASSERT_DOUBLE_EQ(max_bound,v.get_max());
}

TEST_F(VectorOfEquallySpacedNumbersTest, should_have_a_method_to_retrieve_an_element_without_generating_whole_vector)
{
    VectorOfEquallySpacedNumbers vv = a.random<VectorOfEquallySpacedNumbers>();
    std::vector<double> v = vv.get();
    for (size_t i = 0 ; i < vv.size() ; ++i)
    {
        ASSERT_DOUBLE_EQ(v.at(i), vv.at(i));
    }
}

TEST_F(VectorOfEquallySpacedNumbersTest, should_throw_if_retrieving_out_of_bound_element)
{
    VectorOfEquallySpacedNumbers vv = a.random<VectorOfEquallySpacedNumbers>();
    const size_t n = a.random<size_t>().greater_than(vv.size());
    ASSERT_THROW(vv.at(n), VectorOfEquallySpacedNumbersException);
}

TEST_F(VectorOfEquallySpacedNumbersTest, if_min_equals_max_then_a_constant_vector_should_be_generated)
{
    const double bound = a.random<double>().between(-1e5,1e5);
    const size_t nb_of_values = a.random<size_t>().no().greater_than(1e4);
    VectorOfEquallySpacedNumbers v(bound, bound, nb_of_values);
    for (size_t i = 0 ; i < v.size() ; ++i)
    {
        ASSERT_DOUBLE_EQ(bound, v.at(i));
    }
}
