/*
 * UnsafeWrench.cpp
 *
 * \date May 19, 2014
 * \author cec
 */

#include "ssc/kinematics/UnsafeWrench.hpp"

using namespace ssc::kinematics;

UnsafeWrench::UnsafeWrench() : Wrench()
{
}

UnsafeWrench::UnsafeWrench(const Point& P) : Wrench(P)
{
}

UnsafeWrench::UnsafeWrench(const Point& P, const Eigen::Vector3d& force, const Eigen::Vector3d& torque) : Wrench(P,force,torque)
{
}

UnsafeWrench::UnsafeWrench(const UnsafeWrench& rhs)  : Wrench(rhs)
{
}

UnsafeWrench::UnsafeWrench(const Wrench& rhs) : Wrench(rhs)
{
}

UnsafeWrench UnsafeWrench::operator+(const UnsafeWrench& rhs) const
{
    return UnsafeWrench(P, force + rhs.force, torque + rhs.torque);
}

UnsafeWrench UnsafeWrench::operator-(const UnsafeWrench& rhs) const
{
    return UnsafeWrench(P, force - rhs.force, torque - rhs.torque);
}

UnsafeWrench& UnsafeWrench::operator+=(const UnsafeWrench& rhs)
{
    force += rhs.force;
    torque += rhs.torque;
    return *this;
}

UnsafeWrench& UnsafeWrench::operator-=(const UnsafeWrench& rhs)
{
    force -= rhs.force;
    torque -= rhs.torque;
    return *this;
}

UnsafeWrench& UnsafeWrench::operator+=(const Wrench& rhs)
{
    force += rhs.force;
    torque += rhs.torque;
    return *this;
}

UnsafeWrench& UnsafeWrench::operator+=(const Wrench&& rhs)
{
    force += rhs.force;
    torque += rhs.torque;
    return *this;
}

UnsafeWrench& UnsafeWrench::operator-=(const Wrench& rhs)
{
    force -= rhs.force;
    torque -= rhs.torque;
    return *this;
}
