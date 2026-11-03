/*
 * LinearInterpolationVariableStepTest.cpp
 *
 * \date 10 avr. 2013, 12:52:58
 * \author cec
 */

#include "LinearInterpolationVariableStepTest.hpp"
#include "random_increasing_vector.hpp"

#include "ssc/interpolation/LinearInterpolationVariableStep.hpp"
#include "ssc/interpolation/PiecewiseConstantVariableStep.hpp"
#include "ssc/interpolation/InterpolatorException.hpp"
#include "ssc/macros/extra_test_assertions.hpp"
#include "ssc/interpolation/IndexFinder.hpp"

#define EPS 3e-3

using namespace ssc::interpolation;

LinearInterpolationVariableStepTest::LinearInterpolationVariableStepTest() : a(ssc::random_data_generator::DataGenerator(12))
{
}

LinearInterpolationVariableStepTest::~LinearInterpolationVariableStepTest()
{
}

void LinearInterpolationVariableStepTest::SetUp()
{
}

void LinearInterpolationVariableStepTest::TearDown()
{
}

TEST_F(LinearInterpolationVariableStepTest, example)
{
//! [LinearInterpolationVariableStepTest example]
    std::vector<double> x;
    x.push_back(2);x.push_back(5);x.push_back(7);x.push_back(7.5);x.push_back(11);x.push_back(13);
    std::vector<double> y;
    y.push_back(2);y.push_back(2);y.push_back(5);y.push_back(4); y.push_back(8);y.push_back(7);
    LinearInterpolationVariableStep interpolate(x,y);
//! [LinearInterpolationVariableStepTest example]
//! [LinearInterpolationVariableStepTest expected output]
    ASSERT_DOUBLE_EQ(2, interpolate.f(2));
    ASSERT_DOUBLE_EQ(2, interpolate.f(5));
    ASSERT_DOUBLE_EQ(5, interpolate.f(7));
    ASSERT_DOUBLE_EQ(4, interpolate.f(7.5));
    ASSERT_DOUBLE_EQ(8, interpolate.f(11));
    ASSERT_DOUBLE_EQ(7, interpolate.f(13));

//! [LinearInterpolationVariableStepTest expected output]
}

TEST_F(LinearInterpolationVariableStepTest, should_be_able_to_retrieve_initial_values_bug)
{
    std::vector<double> x;
    x.push_back(2590646681.5750818253);x.push_back(2590646739.3061804771);x.push_back(2590647507.8936319351);x.push_back(2590647639.8501925468);x.push_back(2590648039.1781187057);x.push_back(2590648876.3285222054);x.push_back(2590649850.0800924301);x.push_back(2590650127.3491587639);x.push_back(2590650308.4413080215);x.push_back(2590650835.7462720871);x.push_back(2590650922.1302652359);x.push_back(2590650922.2486286163);x.push_back(2590651716.4692950249);x.push_back(2590651905.6780767441);x.push_back(2590652367.8981146812);x.push_back(2590653250.6430096626);x.push_back(2590654171.5978746414);x.push_back(2590654969.2793812752);x.push_back(2590655437.9171380997);x.push_back(2590656360.0427446365);x.push_back(2590656921.6756033897);x.push_back(2590657096.6664185524);x.push_back(2590657338.9127655029);x.push_back(2590658186.8733530045);x.push_back(2590658207.9641985893);x.push_back(2590658952.6549134254);x.push_back(2590659477.8687958717);x.push_back(2590660432.0627980232);x.push_back(2590661049.5781435966);x.push_back(2590661854.3548269272);x.push_back(2590662167.7148189545);x.push_back(2590662700.007256031);x.push_back(2590663001.6911635399);x.push_back(2590663306.1720843315);x.push_back(2590664207.3722276688);x.push_back(2590665130.893928051);x.push_back(2590665440.390941143);x.push_back(2590665668.4642882347);x.push_back(2590666399.5222496986);x.push_back(2590666445.2499046326);x.push_back(2590667296.6532697678);x.push_back(2590667686.1247301102);x.push_back(2590668241.7560658455);x.push_back(2590668618.4195127487);x.push_back(2590668810.9722127914);x.push_back(2590669027.9408707619);x.push_back(2590669088.8791556358);x.push_back(2590669960.0336780548);x.push_back(2590670674.4166917801);x.push_back(2590670950.8542561531);x.push_back(2590671542.6758332253);x.push_back(2590671548.1177973747);x.push_back(2590672522.5214562416);x.push_back(2590673254.8159770966);x.push_back(2590674019.2934451103);x.push_back(2590674958.245262146);x.push_back(2590675107.7772078514);x.push_back(2590675749.8390874863);x.push_back(2590675766.2688474655);x.push_back(2590676475.784485817);x.push_back(2590677125.3182058334);x.push_back(2590677164.2784142494);x.push_back(2590678004.6099395752);x.push_back(2590678608.7609825134);x.push_back(2590678842.0532021523);x.push_back(2590679125.2787747383);
    std::vector<double> y;
    y.push_back(453055631.82882922888);y.push_back(7129451673.3185071945);y.push_back(9899357229.2317523956);y.push_back(7524872323.2361154556);y.push_back(-1199043797.8549640179);y.push_back(7084989605.1586713791);y.push_back(2139262845.04291749);y.push_back(3806649838.5569233894);y.push_back(6725904183.9345722198);y.push_back(-4802166701.2297706604);y.push_back(-68396398.890604451299);y.push_back(-8740238562.439540863);y.push_back(7684265881.592004776);y.push_back(-9899055320.1098175049);y.push_back(3770207723.4115829468);y.push_back(-652782436.8921431303);y.push_back(1243996606.4935603142);y.push_back(-5206398931.9083871841);y.push_back(278476400.83092707396);y.push_back(9444700051.1350326538);y.push_back(-2974125331.0381264687);y.push_back(7279528123.8527145386);y.push_back(-6809542397.0589256287);y.push_back(-8662297835.637462616);y.push_back(564125742.55927824974);y.push_back(5470461943.9987096786);y.push_back(-288299150.51595377922);y.push_back(-3938184532.1575694084);y.push_back(-4898107308.0279188156);y.push_back(3796460304.3715696335);y.push_back(576984658.78514242172);y.push_back(-6931352401.3903226852);y.push_back(-5967686025.5585803986);y.push_back(-5633060624.1420097351);y.push_back(-6347974578.5911560059);y.push_back(-3926398249.8693671227);y.push_back(-2419775516.4376187325);y.push_back(8100592963.6690130234);y.push_back(5884040499.7473468781);y.push_back(-1515710866.7370898724);y.push_back(-7345912318.1739349365);y.push_back(2231300019.3637180328);y.push_back(1604609392.143907547);y.push_back(-4672007045.1194925308);y.push_back(-4834298321.0160150528);y.push_back(-8539735584.9053726196);y.push_back(-3771200522.5407185555);y.push_back(6572502302.8035840988);y.push_back(1000539049.8069410324);y.push_back(9177106338.1967182159);y.push_back(7234513248.8180265427);y.push_back(9753608462.6822109222);y.push_back(-1137690006.9685080051);y.push_back(-3602452172.5402469635);y.push_back(2003441126.2172114849);y.push_back(2576545908.2747812271);y.push_back(-4340657957.168636322);y.push_back(-5903005107.4701032639);y.push_back(1599156089.2359912395);y.push_back(452433027.24466085434);y.push_back(-8165699730.7001485825);y.push_back(5764022419.2155065536);y.push_back(5035498807.3847551346);y.push_back(-3263318648.998152256);y.push_back(6431239942.168598175);y.push_back(4953811133.682053566);
    LinearInterpolationVariableStep interpolate(x,y);
    for (size_t k = 0 ; k < 66 ; ++k)
    {
        ASSERT_SMALL_RELATIVE_ERROR(y.at(k), interpolate.f(x.at(k)),EPS);
    }
}

TEST_F(LinearInterpolationVariableStepTest, should_be_able_to_retrieve_initial_values)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y = a.random_vector_of<double>().of_size(n);
        LinearInterpolationVariableStep interpolate(x,y);
        for (size_t k = 0 ; k < n ; ++k)
        {
            ASSERT_SMALL_RELATIVE_ERROR(y.at(k), interpolate.f(x.at(k  )),EPS);
        }
    }
}

TEST_F(LinearInterpolationVariableStepTest, first_derivative_should_be_zero_if_y_is_constant)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y(n, a.random<double>());
        LinearInterpolationVariableStep interpolate(x,y);
        for (size_t i = 0 ; i < n ; ++i)
        {
            ASSERT_DOUBLE_EQ(0, interpolate.df(a.random<double>().between(x.front(),x.back())));
        }
    }
}

TEST_F(LinearInterpolationVariableStepTest, first_derivative_should_be_constant_if_points_are_on_straight_line)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const double slope = a.random<double>();
        const size_t n = a.random<size_t>().between(2,100);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        std::vector<double> y;
        const double x0 = x.front();
        for (auto it = x.begin() ; it != x.end() ; ++it)
        {
            y.push_back(slope*(*it-x0));
        }
        LinearInterpolationVariableStep interpolate(x,y);
        for (size_t i = 0 ; i < n ; ++i)
        {
            ASSERT_SMALL_RELATIVE_ERROR(slope, interpolate.df(a.random<double>().between(x.front(),x.back())),EPS);
        }
    }
}

TEST_F(LinearInterpolationVariableStepTest, first_derivative_should_be_correctly_computed_for_example_case)
{
    std::vector<double> x;
    x.push_back(2);x.push_back(5);x.push_back(7);x.push_back(7.5);x.push_back(11);x.push_back(13);
    std::vector<double> y;
    y.push_back(2);y.push_back(2);y.push_back(5);y.push_back(4); y.push_back(8);y.push_back(7);
    LinearInterpolationVariableStep interpolate(x,y);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_DOUBLE_EQ(0,interpolate.df(a.random<double>().between(2,5)));
        ASSERT_DOUBLE_EQ(1.5,interpolate.df(a.random<double>().between(5,7)));
        ASSERT_DOUBLE_EQ(-2,interpolate.df(a.random<double>().between(7,7.5)));
        ASSERT_DOUBLE_EQ(4./3.5,interpolate.df(a.random<double>().between(7.5,11)));
        ASSERT_DOUBLE_EQ(-0.5,interpolate.df(a.random<double>().between(11,13)));
    }
}

TEST_F(LinearInterpolationVariableStepTest, second_derivative_should_always_be_zero)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y = a.random_vector_of<double>().of_size(n);
        LinearInterpolationVariableStep interpolate(x,y);
        for (size_t i = 0 ; i < 20 ; ++i)
        {
            ASSERT_DOUBLE_EQ(0, interpolate.df(a.random<double>().between(x.front(),x.back()),2));
        }
    }
}

TEST_F(LinearInterpolationVariableStepTest, interpolated_values_should_be_between_bounds)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y = a.random_vector_of<double>().of_size(n);
        LinearInterpolationVariableStep interpolate(x,y);
        for (size_t i = 0 ; i < n-1 ; ++i)
        {
            const double y0 = y.at(i)>y.at(i+1) ? y.at(i+1) : y.at(i);
            const double y1 = y.at(i)>y.at(i+1) ? y.at(i) : y.at(i+1);
            ASSERT_LE(y0, interpolate.f(a.random<double>().between(x.at(i),x.at(i+1))));
            ASSERT_GE(y1, interpolate.f(a.random<double>().between(x.at(i),x.at(i+1))));
        }
    }
}

TEST_F(LinearInterpolationVariableStepTest, should_throw_if_x_and_y_have_different_sizes)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        const size_t different_n = a.random<size_t>().between(2,100).but_not(n);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y = a.random_vector_of<double>().of_size(different_n);
        ASSERT_THROW(LinearInterpolationVariableStep(x,y), PiecewiseConstantVariableStepException);
    }
}

TEST_F(LinearInterpolationVariableStepTest, should_throw_if_x_is_not_in_strictly_increasing_order)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y = a.random_vector_of<double>().of_size(n);
        ASSERT_NO_THROW(LinearInterpolationVariableStep(x,y));
        const size_t idx = a.random<size_t>().between(1,n-1);
        x.at(idx) = x.at(idx-1);
        ASSERT_THROW(LinearInterpolationVariableStep(x,y), IndexFinderException);
        x.at(idx) = x.at(idx-1)-1;
        ASSERT_THROW(LinearInterpolationVariableStep(x,y), IndexFinderException);
    }
}

TEST_F(LinearInterpolationVariableStepTest, should_throw_if_there_are_not_at_least_two_points)
{
    ASSERT_THROW(LinearInterpolationVariableStep(std::vector<double>(),std::vector<double>()), InterpolatorException);
    ASSERT_THROW(LinearInterpolationVariableStep(std::vector<double>(1,a.random<double>()),std::vector<double>(1,a.random<double>())), PiecewiseConstantVariableStepException);
}

TEST_F(LinearInterpolationVariableStepTest, should_return_constant_when_retrieving_an_x_outside_bounds)
{
    for (size_t i = 0 ; i < 100 ; ++i)
    {
        const size_t n = a.random<size_t>().between(2,100);
        const std::vector<double> x = a_random_vector_of_doubles_in_increasing_order_of_size(a, n);
        const std::vector<double> y = a.random_vector_of<double>().of_size(n).between(-1E3,1E3);
        LinearInterpolationVariableStep interpolate(x,y);
        ASSERT_SMALL_RELATIVE_ERROR(y.front(), interpolate.f(a.random<double>().no().greater_than(x.front())), 1E-6) << "i = " << i;
        ASSERT_SMALL_RELATIVE_ERROR(y.back(), interpolate.f(a.random<double>().greater_than(x.back())), 1E-6) << "i = " << i;
        ASSERT_DOUBLE_EQ(0, interpolate.df(a.random<double>().outside(x.front(),x.back()))) << "i = " << i;
    }
}
