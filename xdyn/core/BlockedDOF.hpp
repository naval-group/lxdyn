/*
 * BlockedDOF.hpp
 *
 *  Created on: Aug 19, 2015
 *      Author: cady
 */
/** \addtogroup Body
 *  \brief The logic to deal with forced DoFs
 *  \details It is this class' responsibility to force DoFs of a body. This includes:
 * - Initialization
 * - Calculation of the value of a forced state at a given time
 * - Update of the states derivatives taking into account forced states
 * - Update of the states taking into account forced states
 * - Calculation of the forcing force required to achieved the force states
 * - Calculation of the second derivatives of forced angles and positions and first derivatives of forced velocities
 * - Telling which states are forced (forcing matrices)
 * @{ */

#ifndef BLOCKEDDOF_HPP_
#define BLOCKEDDOF_HPP_

#include <map>
#include <string>
#include <vector>

#include <Eigen/Dense>

#include <ssc/kinematics.hpp>
#include <ssc/macros/tr1_macros.hpp>

#include TR1INC(memory)

#include "xdyn/external_data_structures/YamlBody.hpp"
#include "xdyn/core/StateMacros.hpp"

std::ostream& operator<<(std::ostream& os, const BlockableState& s);

class BlockedDOF
{
    public:
        /** \brief Initialize DoFs forcing
         *  \details To be able to force a state, one must provide:
         *
         * - the name of the forced state (among x,y,z,u,v,w,p,q,r,phi,theta,psi),
         * - its value at some times
         * - an interpolation method between these times. They can be "piecewise constant", "linear" or "spline"
         *
         * That information is contained in the 'input' data
         * */
        BlockedDOF(const YamlBlockedDOF& input, const size_t body_idx=0);
        BlockedDOF(const std::string& input, const size_t body_idx=0);

        /** \brief Update states taking into accoung forcing
         *  \details This method replaces forced states in x by forced values. The forced values are calculated according to the method defined
         *  at initialization.
         *  \param x state vector
         *  \param t time
         *  \returns x updated state vector taking into accoung forcing
        */
        void force_states(StateType& x, const double t) const;

        /** \brief Update states derivatives taking into accoung forcing
         *  \details This method replaces forced states derivatives in dx_dt by forced values. The forced values are calculated according to the method defined
         *  at initialization.
         *  \param dx_dt states derivatives vector
         *  \param t time
         *  \returns dx_dt updated states derivates taking into accoung forcing
        */
        void force_state_derivatives(StateType& dx_dt, const double t) const;

        /** \brief Calculate the force required to achieve forced states
         *  \details This method calculates the forcing force which is required to achieve the required forced states.
         *  This force is equal to the inertia matrix * (the forced accelerations - the free accelerations). The free accelerations are
         *  obtained from the inertia and sum of other forces.
         *  \param dx_dt states derivatives vector
         *  \param total_inertia inertia matrix (including added mass)
         *  \param sum_of_other_forces sum of forces excluding the forcing force
         *  \returns The forcing force (6 components, at COG, in body reference frame)
        */
        typedef Eigen::Matrix<double,6,1> Vector;
        Vector get_delta_F(const StateType& dx_dt, const Eigen::Matrix<double,6,6>& total_inertia, const ssc::kinematics::Wrench& sum_of_other_forces) const;

        /** \brief Get the k derivative of forced positions at current time
         *  \details This method calculates and returns the k derivative of the forced positions $(x^(k),y^(k),z^(k),phi^(k),theta^(k),psi^(k))$ at current time.
         *
         *  If a position is not forced, the input value is left unchanged.
         *  \param t the current time
         *  \returns $x^(k)$
        */
        void get_forced_x_k(const double t, Eigen::Matrix<double,6,1>& x, size_t k) const;

        /** \brief Get the k derivative of forced velocities at current time
         *  \details This method calculates and returns the k derivative of the forced positions $(u^(k),v^(k),w^(k),p^(k),q^(k),r^(k))$ at current time.
         *
         *  If a position is not forced, the input value is left unchanged.
         *  \param t the current time
         *  \returns $v^(k)$
        */
        void get_forced_v_k(const double t, Eigen::Matrix<double,6,1>& v, size_t k) const;

        /** \brief Get the forcing matrix of motion (x, y, z, roll, pitch, yaw) at time t
         *  \details The forcing matrix of angles is a 6x6 diagonal matrix. It is such as:         *
         * - (0,0)=1 if the x-coordinate is forced (0 otherwise)
         * - (1,1)=1 if the y-coordinate is forced (0 otherwise)
         * - (2,2)=1 if the z-coordinate is forced (0 otherwise)
         * - (3,3)=1 if the roll angle is forced (0 otherwise)
         * - (4,4)=1 if the pitch angle is forced (0 otherwise)
         * - (5,5)=1 if the yaw angle is forced (0 otherwise)
         * Note : it depends on time as the forced and unforced degrees of freedom may change over time
         * \returns The forcing matrix of angles
        */
        Eigen::Matrix<double,6,6> get_delta_x(const double & t = 0.) const;

        /** \brief Get the forcing matrix of velocities (u, v, w,p, q, r)*/
        Eigen::Matrix<double,6,6> get_delta_v(const double & t = 0.) const;

        /** \brief Get the T operator for forcing motion (x, y, z, roll, pitch, yaw)
         *  \details The T operator of angles is a 6x6  matrix. It reflects how the forcing is applied (see 2023 JST paper for details)
         *
         * \returns The T operator
        */
        Eigen::Matrix<double,6,6> get_T_x() const; // Velocities

    private:
        struct Impl;
        TR1(shared_ptr)<Impl> pimpl;
};

#endif /* BLOCKEDDOF_HPP_ */

/** @} */
