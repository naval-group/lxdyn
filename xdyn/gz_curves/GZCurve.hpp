/*
 * GZCurve.hpp
 *
 *  Created on: Feb 20, 2015
 *      Author: cady
 */

#ifndef GZCURVE_HPP_
#define GZCURVE_HPP_

#include <string>
#include <vector>

#include <memory>

#include "GZTypes.hpp"

class Sim;

namespace GZ
{
    class Curve
    {
        public:
            explicit Curve(const Sim& sim);
            ~Curve();
            static std::vector<double> get_phi(const double dphi, const double phi_max);
            double gz(const double phi) const;
            double zeq(const double phi, const double theta) const;
            double get_theta_eq() const;
            State get_Xeq() const;

        private:
            Curve();
            struct Impl;
            std::unique_ptr<Impl> pimpl;
            double theta_eq;
    };
}

#endif /* GZCURVE_HPP_ */
