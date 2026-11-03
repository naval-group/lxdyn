/*
 * TypeCoercionTest.cpp
 *
 * \date 17 juin 2013, 14:54:12
 * \author cec
 */

#include "TypeCoercionTest.hpp"
#include "ssc/data_source/TypeCoercion.hpp"
#include "ssc/data_source/CoercionException.hpp"

using namespace ssc::data_source;

TypeCoercionTest::TypeCoercionTest() : a(ssc::random_data_generator::DataGenerator(1))
{
}

TypeCoercionTest::~TypeCoercionTest()
{
}

void TypeCoercionTest::SetUp()
{
}

void TypeCoercionTest::TearDown()
{
}

TEST_F(TypeCoercionTest, can_coerce_doubles_to_doubles)
{
    const double d1 = a.random<double>();
    const double d2 = a.random<double>();
    const double d3 = a.random<double>();
    std::list<double> v;
    coerce(v,d1);
    coerce(v,d2);
    coerce(v,d3);
    ASSERT_EQ(3, v.size());
    ASSERT_DOUBLE_EQ(d1, v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(d2, v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(d3, v.front());v.pop_front();
}

TEST_F(TypeCoercionTest, can_coerce_vector_of_doubles_to_doubles)
{
    const std::vector<double> v1 = a.random_vector_of<double>().of_size(3);
    const std::vector<double> v2 = a.random_vector_of<double>().of_size(5);
    const std::vector<double> v3 = a.random_vector_of<double>().of_size(8);
    std::list<double> v;
    coerce(v,v1);
    coerce(v,v2);
    coerce(v,v3);
    ASSERT_EQ(16, v.size());
    ASSERT_DOUBLE_EQ(v1.at(0), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v1.at(1), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v1.at(2), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v2.at(0), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v2.at(1), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v2.at(2), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v2.at(3), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v2.at(4), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(0), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(1), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(2), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(3), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(4), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(5), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(6), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3.at(7), v.front());
}

TEST_F(TypeCoercionTest, can_coerce_different_types_to_doubles)
{
    const std::vector<int> v1 = a.random_vector_of<int>().of_size(3);
    const size_t v2 = a.random<size_t>();
    const bool v3 = a.random<bool>();
    const std::vector<float> v4 = a.random_vector_of<float>().of_size(4);
    const short v5 = 7;
    std::list<double> v;
    coerce(v,v1);
    coerce(v,v2);
    coerce(v,v3);
    coerce(v,v4);
    coerce(v,v5);
    ASSERT_EQ(10, v.size());
    ASSERT_DOUBLE_EQ(v1.at(0), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v1.at(1), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v1.at(2), v.front());v.pop_front();
    ASSERT_EQ(v2, v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v3, v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v4.at(0), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v4.at(1), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v4.at(2), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v4.at(3), v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(v5, v.front());v.pop_front();
}

// Lets define a non-arithmetic type
struct S
{
    S(const double a_, const double b_):a(a_),b(b_) {}
    double a;
    double b;
};

// Then define a coercion function for it (note that this is an overload, not a
// specialization because specialization of function templates should generally
// be avoided)
void coerce(std::list<double>& ret, const S& thing_to_convert);
void coerce(std::list<double>& ret, const S& thing_to_convert)
{
    ret.push_back(thing_to_convert.a);
    ret.push_back(thing_to_convert.b);
}

TEST_F(TypeCoercionTest, can_define_coercion_for_non_arithmetic_types)
{
    std::list<double> v;

    const double a_ = a.random<double>();
    const double b_ = a.random<double>();
    coerce(v, S(a_,b_));
    ASSERT_DOUBLE_EQ(a_,v.front());v.pop_front();
    ASSERT_DOUBLE_EQ(b_,v.front());v.pop_front();
}

TEST_F(TypeCoercionTest, can_decoerce_a_scalar)
{
    std::list<double> v;
    double d = 0;
    ASSERT_THROW(decoerce(v,d),CoercionException);
    const double ref = a.random<double>();
    v.push_back(ref);
    ASSERT_NO_THROW(decoerce(v,d));
    ASSERT_TRUE(v.empty());
    ASSERT_DOUBLE_EQ(ref, d);
}

TEST_F(TypeCoercionTest, can_decoerce_a_vector)
{
    std::list<double> L;
    std::vector<double> v;
    ASSERT_NO_THROW(decoerce(L,v));
    const size_t n = a.random<size_t>().no().greater_than(1000);
    const std::vector<double> ref = a.random_vector_of<double>().of_size(n);
    L = std::list<double>(ref.begin(), ref.end());
    ASSERT_NO_THROW(decoerce(L,v));
    v = a.random_vector_of<double>().of_size(a.random<size_t>().between(n,2*n));
    ASSERT_THROW(decoerce(L,v),CoercionException);
    v = std::vector<double>(n,111);
    ASSERT_NO_THROW(decoerce(L,v));
    ASSERT_TRUE(L.empty());
    ASSERT_EQ(n, v.size());
    for (size_t i = 0 ; i < n ; ++i)
    {
        ASSERT_DOUBLE_EQ(ref.at(i), v.at(i));
    }
}

TEST_F(TypeCoercionTest, coerce_decoerce_floats)
{
    std::list<double> v;
    //for (size_t i = 0 ; i<1000 ; ++i)
    {
        /*c:/Documents and Settings/maroff/eclipse_workspace/simulator_sdk/data_source/unit_tests/src/SignalContainerTest.cpp(230): Error: Value of: signals.get<float>("signal5")
          Actual: -8383272448
        Expected: (float)ref.at(k++)
        Which is: -8383272304.9414673*/
        const float ref = (float)-8383272304.9414673;//a.random<float>();
        coerce(v, ref);
        double d = 0;
        decoerce(v, d);
        ASSERT_DOUBLE_EQ(ref, d);
    }
}
