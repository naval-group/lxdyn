/*
 * VectorOfEquallySpacedNumbers.hpp
 *
 * \date 31 juil. 2012, 09:06:33
 * \author cec
 */

#ifndef VECTOROFEQUALLYSPACEDNUMBERS_HPP_
#define VECTOROFEQUALLYSPACEDNUMBERS_HPP_
#include <vector>
#include <cstddef>

namespace ssc
{
    namespace interpolation
    {
        /** \author cec
         *  \brief This class was created to ensure that Splines get equally spaced numbers without having to test at runtime
         *  \details It has the following responsibilities:
         *  - Resp. 1 Construct a vector of doubles in strictly increasing order & equally spaced
         *  \section ex1 Example
         *  \snippet splines/unit_tests/src/VectorOfEquallySpacedNumbersTest.cpp VectorOfEquallySpacedNumbersTest example
         *  \section ex2 Expected output
         *  \snippet splines/unit_tests/src/VectorOfEquallySpacedNumbersTest.cpp VectorOfEquallySpacedNumbersTest expected output
         */
        class VectorOfEquallySpacedNumbers
        {
            public:
                VectorOfEquallySpacedNumbers(const double& min_bound, const double& max_bound, const size_t& nb_of_values);
                std::vector<double> get() const;
                double get_delta() const;
                size_t size() const;
                double get_min() const;
                double get_max() const;
                double at(const size_t& i) const;

            private:
                VectorOfEquallySpacedNumbers();
                double _min_bound;
                double _max_bound;
                size_t _nb_of_values;
                std::vector<double> vec;
                double delta;

        };
    }
}

#endif /* VECTOROFEQUALLYSPACEDNUMBERS_HPP_ */
