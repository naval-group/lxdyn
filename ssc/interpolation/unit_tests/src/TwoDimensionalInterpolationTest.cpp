/*
 * TwoDimensionalSplinesTest.cpp
 *
 * \date 13 nov. 2012, 16:49:02
 * \author cec
 */

#include "TwoDimensionalInterpolationTest.hpp"
#include "ssc/interpolation/TwoDimensionalInterpolation.hpp"
#include "ssc/interpolation/LinearInterpolation.hpp"
#include <vector>

using namespace ssc::interpolation;

TwoDimensionalSplinesTest::TwoDimensionalSplinesTest() : a(ssc::random_data_generator::DataGenerator(12))
{
}

TwoDimensionalSplinesTest::~TwoDimensionalSplinesTest()
{
}

void TwoDimensionalSplinesTest::SetUp()
{
}

void TwoDimensionalSplinesTest::TearDown()
{
}

TEST_F(TwoDimensionalSplinesTest, example)
{
//! [TwoDimensionalSplinesTest example]
    std::vector<std::vector<double> > M;
    std::vector<double> v;
    v.push_back(2);v.push_back(5);v.push_back(10);v.push_back(17);v.push_back(26);v.push_back(37);v.push_back(50);v.push_back(65);v.push_back(82);v.push_back(101);
    M.push_back(v);v.clear();
    v.push_back(5);v.push_back(8);v.push_back(13);v.push_back(20);v.push_back(29);v.push_back(40);v.push_back(53);v.push_back(68);v.push_back(85);v.push_back(104);
    M.push_back(v);v.clear();
    v.push_back(10);v.push_back(13);v.push_back(18);v.push_back(25);v.push_back(34);v.push_back(45);v.push_back(58);v.push_back(73);v.push_back(90);v.push_back(109);
    M.push_back(v);v.clear();
    v.push_back(17);v.push_back(20);v.push_back(25);v.push_back(32);v.push_back(41);v.push_back(52);v.push_back(65);v.push_back(80);v.push_back(97);v.push_back(116);
    M.push_back(v);v.clear();
    v.push_back(26);v.push_back(29);v.push_back(34);v.push_back(41);v.push_back(50);v.push_back(61);v.push_back(74);v.push_back(89);v.push_back(106);v.push_back(125);
    M.push_back(v);v.clear();
    v.push_back(37);v.push_back(40);v.push_back(45);v.push_back(52);v.push_back(61);v.push_back(72);v.push_back(85);v.push_back(100);v.push_back(117);v.push_back(136);
    M.push_back(v);v.clear();
    v.push_back(50);v.push_back(53);v.push_back(58);v.push_back(65);v.push_back(74);v.push_back(85);v.push_back(98);v.push_back(113);v.push_back(130);v.push_back(149);
    M.push_back(v);v.clear();
    v.push_back(65);v.push_back(68);v.push_back(73);v.push_back(80);v.push_back(89);v.push_back(100);v.push_back(113);v.push_back(128);v.push_back(145);v.push_back(164);
    M.push_back(v);v.clear();
    v.push_back(82);v.push_back(85);v.push_back(90);v.push_back(97);v.push_back(106);v.push_back(117);v.push_back(130);v.push_back(145);v.push_back(162);v.push_back(181);
    M.push_back(v);v.clear();
    v.push_back(101);v.push_back(104);v.push_back(109);v.push_back(116);v.push_back(125);v.push_back(136);v.push_back(149);v.push_back(164);v.push_back(181);v.push_back(200);
    M.push_back(v);
//! [TwoDimensionalSplinesTest example]
//! [TwoDimensionalSplinesTest expected output]

    TwoDimensionalInterpolation<LinearInterpolation> spline(1,10,1,10,M);
    ASSERT_DOUBLE_EQ(5, spline.f(1.5,1.5));
    ASSERT_DOUBLE_EQ(9, spline.f(2.5,1.5));
//! [TwoDimensionalSplinesTest expected output]
}

TEST_F(TwoDimensionalSplinesTest, should_throw_if_all_elements_in_M_arent_the_same_size)
{
    std::vector<std::vector<double> > M;

    const size_t n = a.random<size_t>().no().greater_than(20);
    for (size_t i = 0 ; i < n ; ++i)
    {
        M.push_back(a.random_vector_of<double>());
    }
    ASSERT_THROW(TwoDimensionalInterpolation<LinearInterpolation>(a.random<double>(),a.random<double>(),a.random<double>(),a.random<double>(),M), TwoDimensionalInterpolationException);
}

TEST_F(TwoDimensionalSplinesTest, should_throw_if_M_has_less_than_two_elements)
{
    std::vector<std::vector<double> > M;
    ASSERT_THROW(TwoDimensionalInterpolation<LinearInterpolation>(a.random<double>(),a.random<double>(),a.random<double>(),a.random<double>(),M), TwoDimensionalInterpolationException);
    M.push_back(a.random_vector_of<double>());
    ASSERT_THROW(TwoDimensionalInterpolation<LinearInterpolation>(a.random<double>(),a.random<double>(),a.random<double>(),a.random<double>(),M), TwoDimensionalInterpolationException);
}

TEST_F(TwoDimensionalSplinesTest, should_throw_if_any_element_in_M_has_one_element)
{
    std::vector<std::vector<double> > M;

    const size_t n = a.random<size_t>().no().greater_than(20);
    for (size_t i = 0 ; i < n ; ++i)
    {
        M.push_back(std::vector<double>(1,a.random<double>()));
    }
    ASSERT_THROW(TwoDimensionalInterpolation<LinearInterpolation>(a.random<double>(),a.random<double>(),a.random<double>(),a.random<double>(),M), TwoDimensionalInterpolationException);
}

TEST_F(TwoDimensionalSplinesTest, should_throw_if_any_element_in_M_is_empty)
{
    std::vector<std::vector<double> > M;

    const size_t n = a.random<size_t>().no().greater_than(20);
    for (size_t i = 0 ; i < n ; ++i)
    {
        M.push_back(std::vector<double>());
    }
    ASSERT_THROW(TwoDimensionalInterpolation<LinearInterpolation>(a.random<double>(),a.random<double>(),a.random<double>(),a.random<double>(),M), TwoDimensionalInterpolationException);
}


TEST_F(TwoDimensionalSplinesTest, should_work_like_matlabs_griddedInterpolant)
{
//! [TwoDimensionalSplinesTest example]
    std::vector<std::vector<double> > M;
    std::vector<double> v;
    v.push_back(3);v.push_back(4);v.push_back(5);v.push_back(6);v.push_back(7);v.push_back(8);v.push_back(9);v.push_back(10);v.push_back(11);v.push_back(12);
    M.push_back(v);v.clear();
    v.push_back(5);v.push_back(6);v.push_back(7);v.push_back(8);v.push_back(9);v.push_back(10);v.push_back(11);v.push_back(12);v.push_back(13);v.push_back(14);
    M.push_back(v);v.clear();
    v.push_back(7);v.push_back(8);v.push_back(9);v.push_back(10);v.push_back(11);v.push_back(12);v.push_back(13);v.push_back(14);v.push_back(15);v.push_back(16);
    M.push_back(v);v.clear();
    v.push_back(9);v.push_back(10);v.push_back(11);v.push_back(12);v.push_back(13);v.push_back(14);v.push_back(15);v.push_back(16);v.push_back(17);v.push_back(18);
    M.push_back(v);v.clear();
    v.push_back(11);v.push_back(12);v.push_back(13);v.push_back(14);v.push_back(15);v.push_back(16);v.push_back(17);v.push_back(18);v.push_back(19);v.push_back(20);
    M.push_back(v);v.clear();
    v.push_back(13);v.push_back(14);v.push_back(15);v.push_back(16);v.push_back(17);v.push_back(18);v.push_back(19);v.push_back(20);v.push_back(21);v.push_back(22);
    M.push_back(v);v.clear();
    v.push_back(15);v.push_back(16);v.push_back(17);v.push_back(18);v.push_back(19);v.push_back(20);v.push_back(21);v.push_back(22);v.push_back(23);v.push_back(24);
    M.push_back(v);v.clear();
    v.push_back(17);v.push_back(18);v.push_back(19);v.push_back(20);v.push_back(21);v.push_back(22);v.push_back(23);v.push_back(24);v.push_back(25);v.push_back(26);
    M.push_back(v);v.clear();
    v.push_back(19);v.push_back(20);v.push_back(21);v.push_back(22);v.push_back(23);v.push_back(24);v.push_back(25);v.push_back(26);v.push_back(27);v.push_back(28);
    M.push_back(v);v.clear();
    v.push_back(21);v.push_back(22);v.push_back(23);v.push_back(24);v.push_back(25);v.push_back(26);v.push_back(27);v.push_back(28);v.push_back(29);v.push_back(30);
    M.push_back(v);v.clear();
//! [TwoDimensionalSplinesTest example]
//! [TwoDimensionalSplinesTest expected output]
    TwoDimensionalInterpolation<LinearInterpolation> spline(1,10,1,10,M);
    ASSERT_DOUBLE_EQ(3, spline.f(1,1));
    ASSERT_DOUBLE_EQ(30, spline.f(10,10));
    ASSERT_DOUBLE_EQ(4.5, spline.f(1.5,1.5));
    ASSERT_DOUBLE_EQ(6.5, spline.f(2.5,1.5));
    ASSERT_DOUBLE_EQ(5.5, spline.f(1.5,2.5));
    ASSERT_DOUBLE_EQ(8.146, spline.f(1.234,5.678));
//! [TwoDimensionalSplinesTest expected output]
}

TEST_F(TwoDimensionalSplinesTest, should_work_for_asymetric_bounds)
{
//! [TwoDimensionalSplinesTest example]
    std::vector<std::vector<double> > M;
    std::vector<double> v;
    v.push_back(883);v.push_back(969);v.push_back(1059);v.push_back(1153);v.push_back(1251);v.push_back(1353);v.push_back(1459);v.push_back(1569);v.push_back(1683);v.push_back(1801);M.push_back(v);v.clear();
    v.push_back(886);v.push_back(972);v.push_back(1062);v.push_back(1156);v.push_back(1254);v.push_back(1356);v.push_back(1462);v.push_back(1572);v.push_back(1686);v.push_back(1804);M.push_back(v);v.clear();
    v.push_back(891);v.push_back(977);v.push_back(1067);v.push_back(1161);v.push_back(1259);v.push_back(1361);v.push_back(1467);v.push_back(1577);v.push_back(1691);v.push_back(1809);M.push_back(v);v.clear();
    v.push_back(898);v.push_back(984);v.push_back(1074);v.push_back(1168);v.push_back(1266);v.push_back(1368);v.push_back(1474);v.push_back(1584);v.push_back(1698);v.push_back(1816);M.push_back(v);v.clear();
    v.push_back(907);v.push_back(993);v.push_back(1083);v.push_back(1177);v.push_back(1275);v.push_back(1377);v.push_back(1483);v.push_back(1593);v.push_back(1707);v.push_back(1825);M.push_back(v);v.clear();
    v.push_back(918);v.push_back(1004);v.push_back(1094);v.push_back(1188);v.push_back(1286);v.push_back(1388);v.push_back(1494);v.push_back(1604);v.push_back(1718);v.push_back(1836);M.push_back(v);v.clear();
    v.push_back(931);v.push_back(1017);v.push_back(1107);v.push_back(1201);v.push_back(1299);v.push_back(1401);v.push_back(1507);v.push_back(1617);v.push_back(1731);v.push_back(1849);M.push_back(v);v.clear();
    v.push_back(946);v.push_back(1032);v.push_back(1122);v.push_back(1216);v.push_back(1314);v.push_back(1416);v.push_back(1522);v.push_back(1632);v.push_back(1746);v.push_back(1864);M.push_back(v);v.clear();
    v.push_back(963);v.push_back(1049);v.push_back(1139);v.push_back(1233);v.push_back(1331);v.push_back(1433);v.push_back(1539);v.push_back(1649);v.push_back(1763);v.push_back(1881);M.push_back(v);v.clear();
    v.push_back(982);v.push_back(1068);v.push_back(1158);v.push_back(1252);v.push_back(1350);v.push_back(1452);v.push_back(1558);v.push_back(1668);v.push_back(1782);v.push_back(1900);M.push_back(v);v.clear();
    TwoDimensionalInterpolation<LinearInterpolation> spline(1,10,21,30,M);
    ASSERT_DOUBLE_EQ(883, spline.f(1,21));
    ASSERT_DOUBLE_EQ(886, spline.f(2,21));
    ASSERT_DOUBLE_EQ(891, spline.f(3,21));

    ASSERT_DOUBLE_EQ(982, spline.f(10,21));
    ASSERT_DOUBLE_EQ(1900, spline.f(10,30));
    ASSERT_DOUBLE_EQ(1801, spline.f(1,30));
    ASSERT_DOUBLE_EQ(1331.5, spline.f(5.5,25.5));
}
