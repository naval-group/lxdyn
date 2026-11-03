/*
 * SignalContainerTest.cpp
 *
 * \date 18 mars 2013, 18:00:08
 * \author cec
 */

#include "SignalContainerTest.hpp"
#include "ssc/data_source/SignalContainer.hpp"

using namespace ssc::data_source;

SignalContainerTest::SignalContainerTest() : a(ssc::random_data_generator::DataGenerator(19))
{
}

SignalContainerTest::~SignalContainerTest()
{
}

void SignalContainerTest::SetUp()
{
}

void SignalContainerTest::TearDown()
{
}

TEST_F(SignalContainerTest, example)
{
//! [SignalContainerTest example]
    SignalContainer signals;
    const std::string s = a.random<std::string>();
    signals.set<std::string>("signal name",s);
    const double d = a.random<double>();
    signals.set("another signal",d);
//! [SignalContainerTest example]
//! [SignalContainerTest expected output]
    ASSERT_EQ(s, signals.get<std::string>("signal name"));
    ASSERT_DOUBLE_EQ(d, signals.get<double>("another signal"));
//! [SignalContainerTest expected output]
}

TEST_F(SignalContainerTest, should_throw_if_retrieving_an_inexistent_signal)
{
    SignalContainer signals;
    ASSERT_THROW(signals.get<double>(a.random<std::string>()), SignalContainerException);
}

TEST_F(SignalContainerTest, method_get_should_throw_if_signal_has_incorrect_return_type)
{
    SignalContainer signals;
    signals.set<std::string>("signal name",a.random<std::string>());
    ASSERT_THROW(signals.get<double>("signal name"), SignalContainerException);
}


TEST_F(SignalContainerTest, should_be_able_to_retrieve_all_signals_of_a_given_type)
{
    SignalContainer signals;
    const std::string s1 = a.random<std::string>();
    const std::string s2 = a.random<std::string>();
    const std::string s3 = a.random<std::string>();
    const double d1 = a.random<double>();
    const double d2 = a.random<double>();
    signals.set("signal1",s1);
    signals.set("signal2",s2);
    signals.set("signal3",s3);
    signals.set("signal1",d1);
    signals.set("signal2",d2);
    std::map<std::string,std::string> string_signals = signals.get_all<std::string>();
    std::map<std::string,double> double_signals = signals.get_all<double>();
    ASSERT_EQ(3, string_signals.size());
    ASSERT_EQ(2, double_signals.size());
    ASSERT_EQ(s1, string_signals["signal1"]);
    ASSERT_EQ(s2, string_signals["signal2"]);
    ASSERT_EQ(s3, string_signals["signal3"]);
    ASSERT_EQ(d1, double_signals["signal1"]);
    ASSERT_EQ(d2, double_signals["signal2"]);
}



TEST_F(SignalContainerTest, should_be_able_to_retrieve_names_of_all_types_in_SignalContainer)
{
    SignalContainer signals;
    const size_t n = a.random<size_t>().no().greater_than(20);
    const double d1 = a.random<double>();
    const double d2 = a.random<double>();
    const std::vector<double> vd = a.random_vector_of<double>().of_size(n);
    const float f1 = a.random<float>();
    const float f2 = a.random<float>();
    const std::vector<float> vf = a.random_vector_of<float>().of_size(n);
    const int i1 = a.random<int>();
    const int i2 = a.random<int>();
    const std::vector<int> vi = a.random_vector_of<int>().of_size(n);
    const size_t s1 = a.random<size_t>();
    const size_t s2 = a.random<size_t>();
    const std::vector<size_t> vs = a.random_vector_of<size_t>().of_size(n);

    signals.set(a.random<std::string>(),d1);
    signals.set(a.random<std::string>(),d2);
    signals.set(a.random<std::string>(),vd);
    signals.set(a.random<std::string>(),f1);
    signals.set(a.random<std::string>(),f2);
    signals.set(a.random<std::string>(),vf);
    signals.set(a.random<std::string>(),i1);
    signals.set(a.random<std::string>(),i2);
    signals.set(a.random<std::string>(),vi);
    signals.set(a.random<std::string>(),s1);
    signals.set(a.random<std::string>(),s2);
    signals.set(a.random<std::string>(),vs);
}

TEST_F(SignalContainerTest, should_be_able_to_retrieve_all_doubles_in_a_single_list)
{
    SignalContainer signals;
    const size_t n = a.random<size_t>().no().greater_than(20);
    const double d1 = a.random<double>();
    const double d2 = a.random<double>();
    const std::vector<double> vd = a.random_vector_of<double>().of_size(n);
    const float f1 = a.random<float>();
    const float f2 = a.random<float>();
    const std::vector<float> vf = a.random_vector_of<float>().of_size(n);
    const int i1 = a.random<int>();
    const int i2 = a.random<int>();
    const std::vector<int> vi = a.random_vector_of<int>().of_size(n);
    const size_t s1 = a.random<size_t>();
    const size_t s2 = a.random<size_t>();
    const std::vector<size_t> vs = a.random_vector_of<size_t>().of_size(n);

    signals.set("signal1",d1);
    signals.set("signal3",d2);
    signals.set("signal2",vd);
    signals.set("signal5",f1);
    signals.set("signal7",f2);
    signals.set("signal4",vf);
    signals.set("signal6",i1);
    signals.set("signal9",i2);
    signals.set("signal8",vi);
    signals.set("signal10",s1);
    signals.set("signal11",s2);
    signals.set("signal12",vs);
    std::list<double> s = signals.to_doubles();

    ASSERT_EQ(4*(2+n), s.size());

    ASSERT_DOUBLE_EQ(i1, s.front());s.pop_front();
    ASSERT_DOUBLE_EQ(i2, s.front());s.pop_front();
    for (size_t k = 0 ; k < n ; ++k)
    {
        ASSERT_DOUBLE_EQ(vi.at(k), s.front());s.pop_front();
    }

    ASSERT_EQ(s1, s.front());s.pop_front();
    ASSERT_EQ(s2, s.front());s.pop_front();
    for (size_t k = 0 ; k < n ; ++k)
    {
        ASSERT_EQ(vs.at(k), s.front());s.pop_front();
    }

    ASSERT_DOUBLE_EQ(f1, s.front());s.pop_front();
    ASSERT_DOUBLE_EQ(f2, s.front());s.pop_front();
    for (size_t k = 0 ; k < n ; ++k)
    {
        ASSERT_DOUBLE_EQ(vf.at(k), s.front());s.pop_front();
    }

    ASSERT_DOUBLE_EQ(d1, s.front());s.pop_front();
    ASSERT_DOUBLE_EQ(d2, s.front());s.pop_front();
    for (size_t k = 0 ; k < n ; ++k)
    {
        ASSERT_DOUBLE_EQ(vd.at(k), s.front());s.pop_front();
    }
}

TEST_F(SignalContainerTest, should_be_able_to_set_all_coercible_types_using_a_list)
{
    SignalContainer signals;
    const size_t n = a.random<size_t>().no().greater_than(20);
    const double d1 = a.random<double>();
    const double d2 = a.random<double>();
    const std::vector<double> vd = a.random_vector_of<double>().of_size(n);
    const float f1 = a.random<float>();
    const float f2 = a.random<float>();
    const std::vector<float> vf = a.random_vector_of<float>().of_size(n);

    const int i1 = a.random<int>();
    const int i2 = a.random<int>();
    const std::vector<int> vi = a.random_vector_of<int>().of_size(n);
    const size_t s1 = a.random<size_t>();
    const size_t s2 = a.random<size_t>();
    const std::vector<size_t> vs = a.random_vector_of<size_t>().of_size(n);

    signals.set("signal1",d1);
    signals.set("signal3",d2);
    signals.set("signal2",vd);
    signals.set("signal5",f1);
    signals.set("signal7",f2);
    signals.set("signal4",vf);
    signals.set("signal6",i1);
    signals.set("signal9",i2);
    signals.set("signal8",vi);
    signals.set("signal10",s1);
    signals.set("signal11",s2);
    signals.set("signal12",vs);

    const std::vector<float> ref_ = a.random_vector_of<float>().of_size(4*(2+n));
    std::vector<double> ref;
    for (size_t i = 0 ; i < ref_.size() ; ++i) ref.push_back(ref_.at(i));
    const std::list<double> l(ref.begin(), ref.end());

    signals.from_doubles(l);

    size_t k = 0;

    ASSERT_DOUBLE_EQ((int)ref.at(k++), signals.get<int>("signal6"));
    ASSERT_DOUBLE_EQ((int)ref.at(k++), signals.get<int>("signal9"));
    std::vector<int> vvi = signals.get<std::vector<int> >("signal8");
    for (size_t i = 0 ; i < n ; ++i)
    {
        ASSERT_DOUBLE_EQ((int)ref.at(k++), vvi.at(i));
    }

    ASSERT_EQ((size_t)ref.at(k++), signals.get<size_t>("signal10"));
    ASSERT_EQ((size_t)ref.at(k++), signals.get<size_t>("signal11"));
    std::vector<size_t> vvs = signals.get<std::vector<size_t> >("signal12");
    for (size_t i = 0 ; i < n ; ++i)
    {
        ASSERT_EQ((size_t)ref.at(k++), vvs.at(i));
    }

    ASSERT_DOUBLE_EQ((float)ref.at(k++), signals.get<float>("signal5"));
    ASSERT_DOUBLE_EQ((float)ref.at(k++), signals.get<float>("signal7"));
    std::vector<float> vvf = signals.get<std::vector<float> >("signal4");
    for (size_t i = 0 ; i < n ; ++i)
    {
        ref.at(k);
        vvf.at(i);
        ASSERT_DOUBLE_EQ((float)ref.at(k++), vvf.at(i));
    }
    ASSERT_DOUBLE_EQ(ref.at(k++), signals.get<double>("signal1"));
    ASSERT_DOUBLE_EQ(ref.at(k++), signals.get<double>("signal3"));
    std::vector<double> vvd = signals.get<std::vector<double> >("signal2");
    for (size_t i = 0 ; i < n ; ++i)
    {
        ASSERT_DOUBLE_EQ(ref.at(k++), vvd.at(i));
    }
}

TEST_F(SignalContainerTest, float_coercion_bug)
{
    SignalContainer signals;
    const float f1 = a.random<float>();

    signals.set("signal5",f1);


    const std::vector<float> ref_f = a.random_vector_of<float>().of_size(2);
    std::vector<double> ref;
    for (size_t i = 0 ; i < ref_f.size() ; ++i) ref.push_back(ref_f.at(i));
    const std::list<double> l(ref.begin(), ref.end());

    signals.from_doubles(l);

    size_t k = 0;

    ASSERT_DOUBLE_EQ((float)ref.at(k++), signals.get<float>("signal5"));
}

TEST_F(SignalContainerTest, can_remove_a_signal)
{
    const std::string name = a.random<std::string>();
    SignalContainer signals;
    signals.set(name,a.random<double>()());
    ASSERT_NO_THROW(signals.get<double>(name));
    signals.remove<double>(name);
    ASSERT_THROW(signals.get<double>(name), SignalContainerException);
}

TEST_F(SignalContainerTest, can_get_number_of_signals_in_container)
{
    SignalContainer signals;
    for (size_t nb_of_signals = 0 ; nb_of_signals < 100 ; ++nb_of_signals)
    {
        signals.clear();
        for (size_t j = 0 ; j < nb_of_signals ; ++j)
        {
            signals.set(a.random<std::string>(),a.random<double>()());
        }
        ASSERT_EQ(nb_of_signals, signals.size());
    }
}
