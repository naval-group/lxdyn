#ifndef DATAGENERATOR_HPP_
#define DATAGENERATOR_HPP_

#define GET_A_MIN_STRING_LENGTH 3
#define GET_A_MAX_STRING_LENGTH 17

#include <math.h> // For floor
#include <stdlib.h> // For size_t
#include <string>
#include <vector>
#include <set>

#if defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    #define not !
#endif


namespace ssc
{
    namespace random_data_generator
    {

        template <class C> class TypedScalarDataGenerator;
        template <class C> class TypedVectorDataGenerator;
        template <class C> class TypedSetDataGenerator;

        class DataGenerator
        {
            public:
                template <class T> TypedScalarDataGenerator<T> random() const
                {
                    TypedScalarDataGenerator<T> ret(*this);
                    ret.negated = false;
                    return ret;
                }
                template <class T> TypedVectorDataGenerator<T> random_vector_of() const
                {
                    return TypedVectorDataGenerator<T>(*this);
                }
                template <class T> TypedSetDataGenerator<T> random_set_of() const
                {
                    return TypedSetDataGenerator<T>(*this);
                }

                DataGenerator(const size_t& seed);
                virtual ~DataGenerator(){}

                virtual DataGenerator& no();

            protected:
                bool negated;
                bool random_bool() const;
                const std::vector<size_t>      vector_of_random_size_ts(const size_t& n, const size_t& a, const size_t& b) const;
                const std::vector<double>      vector_of_random_doubles(const size_t& n, const double& a, const double& b) const;

                int    random_int(const int& a, const int& b) const;
                size_t random_size_t(const size_t& a, const size_t& b) const;
                float  random_float(const float& a, const float& b) const;
                long   random_long(const long& a, const long& b) const;
                short  random_short(const short& a, const short& b) const;
                double random_double(const double& a, const double& b) const;
                char   random_char() const;
                const std::string random_string(const size_t& string_length) const;
                const std::string random_string(const size_t& min_string_length, const size_t& max_string_length) const;
                const std::string random_string() const;
            private:
                DataGenerator();
                int get_random_number() const;

        };

        template <class T> T get_min_bound() {return T();}
        template <class T> T get_max_bound() {return T();}
        template<> double get_min_bound<double>();
        template<> double get_max_bound<double>();
        template<> size_t get_min_bound<size_t>();
        template<> size_t get_max_bound<size_t>();


        template <class T> class TypedScalarDataGenerator : public DataGenerator
        {
            public:
                TypedScalarDataGenerator(const DataGenerator& rhs) : DataGenerator(rhs),
                                                                     min_bound(get_min_bound<T>()),
                                                                     max_bound(get_max_bound<T>()),
                                                                     forbidden_min(get_max_bound<T>()),
                                                                     forbidden_max(get_min_bound<T>())
                {
                }

                T operator()(void)
                {
                    #if defined(_MSC_VER) /* Microsoft Visual Studio bullshit */
                    return msvc_bullshit();
                    #else
                    T tmp = get();
                    min_bound = get_min_bound<T>();
                    max_bound = get_max_bound<T>();
                    negated = false;
                    return tmp;
                    #endif

                }
                #if defined(_MSC_VER) /* Microsoft Visual Studio bullshit */
                T msvc_bullshit()
                {
                    T tmp = get();
                    min_bound = get_min_bound<T>();
                    max_bound = get_max_bound<T>();
                    negated = false;
                    return tmp;
                }
                #endif

                operator T()
                {
                    #if defined(_MSC_VER) /* Microsoft Visual Studio bullshit */
                    return msvc_bullshit();
                    #else
                    return this->operator()();
                    #endif
                }

                TypedScalarDataGenerator& greater_than(const T& t)
                {
                    if (negated)
                    {
                        max_bound = t;
                    }
                    else
                    {
                        min_bound = t;
                        but_not(t);
                    }
                    return *this;
                }

                TypedScalarDataGenerator& between(const T& m, const T& M)
                {
                    if (not(negated))
                    {
                        min_bound = m;
                        max_bound = M;
                    }
                    else
                    {
                        forbidden_min = m;
                        forbidden_max = M;
                    }
                    return *this;
                }

                TypedScalarDataGenerator<T>& no()
                {
                    negated = true;
                    return *this;
                }

                TypedScalarDataGenerator<T>& but_not()
                {
                    return no();
                }

                TypedScalarDataGenerator<T>& but()
                {
                    negated = false;
                    return *this;
                }

                TypedScalarDataGenerator<T>& outside(const T& t1, const T& t2)
                {
                    return (but_not().between(t1, t2)).but();
                }

                TypedScalarDataGenerator<T>& but_not(const T& t)
                {
                    return outside(t, t);
                }



            private:
                TypedScalarDataGenerator();
                T get() const;


        public:
                T min_bound;
                T max_bound;
                T forbidden_min;
                T forbidden_max;
        };

        template <class T> class TypedVectorDataGenerator : public DataGenerator
        {
            public:
                TypedVectorDataGenerator(const DataGenerator& rhs) : DataGenerator(rhs),
                                                                     size(0),
                                                                     min_bound(get_min_bound<T>()),
                                                                     max_bound(get_max_bound<T>()),
                                                                     forbidden_min(get_max_bound<T>()),
                                                                     forbidden_max(get_min_bound<T>())
                {}

                TypedVectorDataGenerator& of_size(const size_t& n)
                {
                    size = n;
                    return *this;
                }

            private:

                std::vector<T> get_vector() const
                    {
                        std::vector<T> ret;
                        ret.reserve(size);
                        TypedScalarDataGenerator<T> r = random<T>();
                        for (size_t i = 0 ; i < size ; ++i)
                        {
                            r.between(min_bound, max_bound)
                             .outside(forbidden_min,forbidden_max);
                            #if defined(_MSC_VER) /* Microsoft Visual Studio bullshit */
                            ret.push_back(random<T>().msvc_bullshit());
                            #else
                            ret.push_back((T)r);
                            #endif
                        }
                        return ret;
                    }
            public:
                std::vector<T> get_impl()
                {
                    std::vector<T> tmp = get_vector();
                    negated = false;
                    min_bound = get_min_bound<T>();
                    max_bound = get_max_bound<T>();
                    return tmp;
                }

                operator std::vector<T>()
                {
                    return get_impl();
                }

                TypedVectorDataGenerator<T>& greater_than(const T& t)
                {
                    if (negated)
                    {
                        max_bound = t;
                    }
                    else
                    {
                        min_bound = t;
                    }
                    return *this;
                }

                TypedVectorDataGenerator<T>& but()
                {
                    negated = false;
                    return *this;
                }


                TypedVectorDataGenerator& between(const T& t1, const T& t2)
                {
                    if (not(negated))
                    {
                        min_bound = t1;
                        max_bound = t2;
                    }
                    else
                    {
                        forbidden_min = t1;
                        forbidden_max = t2;
                    }
                    return *this;
                }

                TypedVectorDataGenerator<T>& no()
                {
                    negated = true;
                    return *this;
                }

            private:
                size_t size;
                T min_bound;
                T max_bound;
                T forbidden_min;
                T forbidden_max;
        };

        template <> double TypedScalarDataGenerator<double>::operator()();

        template <class T> class TypedSetDataGenerator : public DataGenerator
        {
            public:
                TypedSetDataGenerator(const DataGenerator& rhs) : DataGenerator(rhs),
                                                                  size(0),
                                                                  min_bound(get_min_bound<T>()),
                                                                  max_bound(get_max_bound<T>())
                {}

                TypedSetDataGenerator& of_size(const size_t& n)
                {
                    size = n;
                    return *this;
                }

                std::set<T> operator()()
                {
                    std::set<T> tmp = get();
                    negated = false;
                    min_bound = get_min_bound<T>();
                    max_bound = get_max_bound<T>();
                    return tmp;
                }

                TypedSetDataGenerator& greater_than(const T& t)
                {
                    if (negated)
                    {
                        max_bound = t;
                    }
                    else
                    {
                        min_bound = t;
                    }
                    return *this;
                }

                TypedSetDataGenerator<T>& no()
                {
                    negated = true;
                    return *this;
                }

            private:
                std::set<T> get() const;
                size_t size;
                T min_bound;
                T max_bound;
        };
    }
}


#endif
