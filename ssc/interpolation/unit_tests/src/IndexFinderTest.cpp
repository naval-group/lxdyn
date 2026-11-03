/*
 * IndexFinderTest.cpp
 *
 * \date 10 sept. 2013, 10:15:24
 * \author cec
 */

#include "IndexFinderTest.hpp"
#include "ssc/interpolation/IndexFinder.hpp"

using namespace ssc::interpolation;

IndexFinderTest::IndexFinderTest() : a(ssc::random_data_generator::DataGenerator(112233))
{
}

IndexFinderTest::~IndexFinderTest()
{
}

void IndexFinderTest::SetUp()
{
}

void IndexFinderTest::TearDown()
{
}

TEST_F(IndexFinderTest, example)
{
//! [IndexFinderTest example]
    std::vector<double> x;
    x.push_back(1);x.push_back(1.1);x.push_back(2);x.push_back(20);x.push_back(20.01);x.push_back(30);x.push_back(40);
    IndexFinder index(x);
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_THROW(index.compute(a.random<double>().no().greater_than(1)), IndexFinderException);
    }
    ASSERT_EQ(0, index.compute(1));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_EQ(0, index.compute(a.random<double>().between(1,1.1)));
    }
    ASSERT_EQ(1, index.compute(1.1));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_EQ(1, index.compute(a.random<double>().between(1.1,2)));
    }
    ASSERT_EQ(2, index.compute(2));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_EQ(2, index.compute(a.random<double>().between(2,20)));
    }
    ASSERT_EQ(3, index.compute(20));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_EQ(3, index.compute(a.random<double>().between(20,20.01)));
    }
    ASSERT_EQ(4, index.compute(20.01));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_EQ(4, index.compute(a.random<double>().between(20.01,30)));
    }
    ASSERT_EQ(5, index.compute(30));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_EQ(5, index.compute(a.random<double>().between(30,40)));
    }
    ASSERT_EQ(6, index.compute(40));
    for (size_t i = 0 ; i < 1000 ; ++i)
    {
        ASSERT_THROW(index.compute(a.random<double>().greater_than(40)), IndexFinderException);
    }
//! [IndexFinderTest example]
//! [IndexFinderTest expected output]
//! [IndexFinderTest expected output]
}

TEST_F(IndexFinderTest, bug_detected_in_LinearInterpolationVariableStep)
{
    std::vector<double> v;
    v.push_back(1336494152.2254669666); v.push_back(1336495118.1408424377); v.push_back(1336495866.0685279369); v.push_back(1336496233.459425211); v.push_back(1336496714.096323967); v.push_back(1336496787.8505876064); v.push_back(1336496793.2058169842); v.push_back(1336497140.2871055603); v.push_back(1336497482.5309216976); v.push_back(1336497700.4826481342); v.push_back(1336497833.6430580616); v.push_back(1336498734.1675724983); v.push_back(1336499120.9335830212); v.push_back(1336499566.4158723354); v.push_back(1336500228.3480532169); v.push_back(1336500244.4563536644); v.push_back(1336500895.3111896515); v.push_back(1336501541.720015049); v.push_back(1336501864.7073059082); v.push_back(1336502720.3997087479); v.push_back(1336503121.6866281033); v.push_back(1336503328.5609574318); v.push_back(1336504297.1004216671); v.push_back(1336504895.4999558926); v.push_back(1336505568.4806885719); v.push_back(1336506025.3629992008); v.push_back(1336506355.3781228065); v.push_back(1336506455.7610492706); v.push_back(1336507211.2143540382); v.push_back(1336507816.9076209068); v.push_back(1336508535.9555342197); v.push_back(1336509433.2901380062); v.push_back(1336510091.5192584991); v.push_back(1336510242.2360956669); v.push_back(1336510854.5510005951); v.push_back(1336511833.2112386227); v.push_back(1336512832.3535079956); v.push_back(1336513089.1514942646); v.push_back(1336513640.01689744); v.push_back(1336514299.0644152164); v.push_back(1336514853.0695505142); v.push_back(1336515830.82964921); v.push_back(1336516732.7529537678); v.push_back(1336517390.6776382923); v.push_back(1336518119.5361466408); v.push_back(1336518521.9914116859); v.push_back(1336519450.6190748215); v.push_back(1336519598.4542648792); v.push_back(1336520272.9835639); v.push_back(1336521042.5978732109); v.push_back(1336521381.920425415); v.push_back(1336521497.7392804623); v.push_back(1336522112.1084647179); v.push_back(1336522932.7256042957); v.push_back(1336523879.8202602863); v.push_back(1336524610.9489121437); v.push_back(1336525108.5528185368); v.push_back(1336525483.3545594215); v.push_back(1336525904.8604154587); v.push_back(1336526457.763453722); v.push_back(1336527455.6827328205); v.push_back(1336528446.077480793); v.push_back(1336529192.3871359825); v.push_back(1336530146.1461977959); v.push_back(1336530239.4208881855); v.push_back(1336530973.4445753098); v.push_back(1336531725.2061913013); v.push_back(1336532672.0546970367); v.push_back(1336533378.2310602665); v.push_back(1336534192.0407266617); v.push_back(1336534750.6352462769); v.push_back(1336534812.3408231735); v.push_back(1336535292.7216033936); v.push_back(1336535890.4113755226); v.push_back(1336536027.9432942867); v.push_back(1336536615.3384947777); v.push_back(1336537135.3067576885); v.push_back(1336538021.1851024628); v.push_back(1336538324.9952745438); v.push_back(1336538994.6525785923); v.push_back(1336539659.5926702023); v.push_back(1336540163.2695696354); v.push_back(1336540424.8446900845); v.push_back(1336540501.404191494); v.push_back(1336540602.6538543701); v.push_back(1336541151.9195940495); v.push_back(1336541527.5045416355); v.push_back(1336541542.6540441513); v.push_back(1336542335.5694901943); v.push_back(1336542956.4470350742); v.push_back(1336543730.0506124496); v.push_back(1336544683.6313734055); v.push_back(1336544797.8757455349); v.push_back(1336545116.338388443); v.push_back(1336545713.1582345963); v.push_back(1336545761.3111371994); v.push_back(1336545875.5169153214); v.push_back(1336546091.4818344116); v.push_back(1336546192.0552268028); v.push_back(1336546265.3970293999);
    const double x0 = 1336495419.2217235565;
    IndexFinder index(v);
    ASSERT_EQ(1, index.compute(x0));
}

