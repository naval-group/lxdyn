/*
 * EulerAngles.hpp
 *
 * \date 7 avr. 2014
 * \author cec
 */

#ifndef EULERANGLES_HPP_
#define EULERANGLES_HPP_

namespace ssc
{
    namespace kinematics
    {
        /** \author cec
         *  \brief
         *  \details
         *  \ingroup
         *  \section ex1 Example
         *  \snippet kinematics/unit_tests/src/EulerAnglesTest.cpp EulerAnglesTest example
         *  \section ex2 Expected output
         *  \snippet kinematics/unit_tests/src/EulerAnglesTest.cpp EulerAnglesTest expected output
         */
        class EulerAngles
        {
            public:
                EulerAngles();
                EulerAngles(const double phi, const double theta, const double psi);
                double phi;
                double theta;
                double psi;
        };
    }
}

#endif /* EULERANGLES_HPP_ */
