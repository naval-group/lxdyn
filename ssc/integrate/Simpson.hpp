/*
 * Simpson.hpp
 *
 * \date 23 avr. 2013, 15:39:50
 * \author cec
 */

#ifndef SIMPSON_HPP_
#define SIMPSON_HPP_

#include "ssc/macros/tr1_macros.hpp"
#include TR1INC(memory)

#include "ssc/integrate/Integrator.hpp"




/** \author cec
 *  \ingroup quadpack
 *  \brief This class was created to
 *  \details It has the following responsibilities:
 *  - Resp. 1 ...
 *  \section ex1 Example
 *  \snippet quadpack/unit_tests/src/SimpsonTest.cpp SimpsonTest example
 *  \section ex2 Expected output
 *  \snippet quadpack/unit_tests/src/SimpsonTest.cpp SimpsonTest expected output
 */
namespace ssc
{
    namespace integrate
    {
        class Simpson : public Integrator
        {
            public:
                Simpson();
                Simpson(const Function& f);

            private:
                class Impl;
                TR1(shared_ptr)<Impl> pimpl;
                double integrate_impl(const Function& f, double a, double b, double eps=1e-6);
        };
    }
}

#endif /* SIMPSON_HPP_ */
