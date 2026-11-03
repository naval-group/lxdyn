#include "ssc/kinematics/EulerAngles.hpp"

using namespace ssc::kinematics;

EulerAngles::EulerAngles() : phi(0), theta(0), psi(0)
{
}

EulerAngles::EulerAngles(const double phi_, const double theta_, const double psi_) : phi(phi_), theta(theta_), psi(psi_)
{
}
