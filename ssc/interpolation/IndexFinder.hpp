/*
 * IndexFinder.hpp
 *
 * \date 10 sept. 2013, 10:03:21
 * \author cec
 */

#ifndef INDEXFINDER_HPP_
#define INDEXFINDER_HPP_

#include "ssc/exception_handling/Exception.hpp"
#include <vector>

namespace ssc
{
    namespace interpolation
    {
        class IndexFinderException : public ::ssc::exception_handling::Exception
        {
            public:
                IndexFinderException(const std::string& message, const std::string& file, const std::string& function, const unsigned int line) :
                    Exception(message, file, function, line)
                {
                }
        };


        /** \author cec
         *  \ingroup MODULE_NAME
         *  \brief Responsibility
         *  \details
         *  \section ex1 Example
         *  \snippet interpolation/unit_tests/src/IndexFinderTest.cpp IndexFinderTest example
         *  \section ex2 Expected output
         *  \snippet interpolation/unit_tests/src/IndexFinderTest.cpp IndexFinderTest expected output
         */

        class IndexFinder
        {
            public:
                IndexFinder(const std::vector<double>& x, const bool throw_if_outside_bounds_=true);
                size_t compute(const double x0);

            private:
                IndexFinder();
                bool is_still_valid_for(const double x0) const;
                void adjust_left(int& left) const;
                size_t greater_than_max(const int ilo);
                size_t lower_than_min(const int ilo);
                size_t between_min_and_max(const int ilo);
                size_t bisection (const int ilo, const int ihi, const double x);
                size_t increase_ihi_to_capture_x(int& ilo, int& ihi, const double x);
                size_t decrease_ilo_to_capture_x(int& ilo, int& ihi, const double x);
                void check_x_is_strictly_increasing(const std::vector<double>& x);

                int n;
                std::vector<double> x_;
                double xmin;
                double xmax;
                int idx_low;
            public:
                bool throw_if_outside_bounds;
        };
    }
}

#endif /* INDEXFINDER_HPP_ */
