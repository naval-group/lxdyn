/*
 * Body.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */
/**   \defgroup Body_dynamics
 *    \ingroup Model
          *  \brief The class to deal with the dynamic of a body
          *  \details It is this class' responsibility to deal with the dynamic of a body. This includes:
          * - Update of the time derivative of the components of the complete state vector corresponding to this body
          * - Update of other dynamic data related to this body which includes;
          *         -
          *  **Note**: This class does not provide method for initialization. This is left to a dedicated builder (see BodyBuilder class).
          @{
          */

#ifndef BODY_HPP_
#define BODY_HPP_

#include "BlockedDOF.hpp"
#include "BodyStates.hpp"
#include "StatesFilter.hpp"
#include "StateMacros.hpp"
#include "State.hpp"

#include <ssc/kinematics.hpp>

#include <tuple>

struct YamlBody;
struct YamlRotation;
struct EnvironmentAndFrames;

class Observer;

class Body
{
    public:
        virtual ~Body();

        /** \brief A constructor for the body object
         *  \details A constructor for a body object.
         *
         *  **Note:** The constructor does not perform the initialization. This is left to the BodyBuilder class.
         *  \param `idx` the number of the body
         *  \param `blocked_states` the definition of the forced DoFs
         *  \param `filtered_states` the definition of filters for states filtering
        */
        Body(const size_t idx, const BlockedDOF& blocked_states, const YamlFilteredStates& filtered_states);

        /** \brief Constructor for the body object
         *  \details Constructor for a body object.
         *
         *  **Note:** The constructor does not perform the initialization. This is left to the BodyBuilder class.
         *  \param `idx` the number of the body
         *  \param `states` the mechanical and geometric characteristics of the body
         *  \param `blocked_states` the definition of the forced DoFs
         *  \param `filtered_states` the definition of filters for states filtering
        */
        Body(const BodyStates& states, const size_t idx, const BlockedDOF& blocked_states, const YamlFilteredStates& filtered_states);

        /** \brief Constructor for the body object
         *  \details Constructor for a body object.
         *
         *  **Note:** The constructor does not perform the initialization. This is left to the BodyBuilder class.
         *  \param `idx` the number of the body
         *  \param `blocked_states` the definition of the forced DoFs
         *  \param `states_filter` the definition of filters for states filtering
        */
        Body(const size_t idx, const BlockedDOF& blocked_states, const StatesFilter& states_filter);

        /** \brief Constructor for the body object
         *  \details Constructor for a body object.
         *
         *  **Note:** The constructor does not perform the initialization. This is left to the BodyBuilder class.
         *  \param `idx` the number of the body
         *  \param `states` the mechanical and geometric characteristics of the body
         *  \param `blocked_states` the definition of the forced DoFs
         *  \param `states_filter` the definition of filters for states filtering
        */
        Body(const BodyStates& states, const size_t idx, const BlockedDOF& blocked_states, const StatesFilter& states_filter);

        /** \brief Returns the states of this body
         *  \returns The states of this body (states includes mechanical and geometric characteristics)
        */
        BodyStates get_states() const;

        /** \brief Use SurfaceElevation to compute wave height & update accordingly
         */
        virtual void update_intersection_with_free_surface(
            const EnvironmentAndFrames& env,
            const double t
            ) = 0;

        /** \brief Extract the coordinates $(x, y, z)$ of the origin of body in the NED reference frame given a state vector `x`
         *  \returns The coordinates $(x, y, z)$ of the origin of body in the NED reference frame
         */
        ssc::kinematics::Point get_origin(const StateType& x) const;

        /** \brief Get the coordinates $(X, Y, Z)$ of the origin of the body in the mesh reference frame
         *  \returns The coordinates $(X, Y, Z)$ of the origin of the body in the mesh reference frame
         */
        ssc::kinematics::Point get_position_of_body_relative_to_mesh() const;

        /** \brief Get the transform operator from mesh-fixed coordinates to body-fixed coordinates
         *  \details This operator can be used to convert coordinates from the mesh-fixed coordinates to body-fixed coordinates
         *  \returns The transform operator from mesh-fixed coordinates to body-fixed coordinates
         */
        ssc::kinematics::Transform get_transform_from_mesh_to_body() const;

        /** \brief Get the transform operator from NED-fixed coordinates to body-fixed coordinates
         *  \details This operator can be used to convert coordinates from the NED-fixed coordinates to body-fixed coordinates
         *  \returns The transform operator from NED-fixed coordinates to body-fixed coordinates
         */
        ssc::kinematics::Transform get_transform_from_ned_to_body(const StateType& x) const;

        /** \brief Get the transform operator from NED-fixed coordinates to local-NED-fixed coordinates
         *  \details This operator can be used to convert coordinates from the NED-fixed coordinates to local-NED-fixed coordinates
         *  \returns The transform operator from NED-fixed coordinates to local-NED-fixed coordinates
         */
        ssc::kinematics::Transform get_transform_from_ned_to_local_ned(const StateType& x) const;

        /**  \brief Update Body structure taking the new coordinates & wave heights into account
         */
        void update(const EnvironmentAndFrames& env, const StateType& x, const double t);
        void set_history(const EnvironmentAndFrames& env, const State& states);
        void update_kinematics(const StateType& x, const ssc::kinematics::KinematicsPtr& k) const;
        void update_body_states(StateType x, const double t);

        /** \brief Update states with forced values at time `t`
         *  \param x the complete states vector at current time `t`. It includes both the states of the present body and that of the other bodies.
         *  \param t the current time
         * */
        void force_states(StateType& x, const double t) const;

        StateType block_states_if_necessary(StateType x, const double t) const;

        /**  \brief Update down vector (expressed in body's mesh frame), taking the new coordinates into account
         */
        void update_projection_of_z_in_mesh_frame(const double g,
                                                  const ssc::kinematics::KinematicsPtr& k);

        /** \brief Calculate state derivatives of this body
         *  \details This method returns the states derivatives of the body given the current time, states and sum of forces applying to it.
         *  It implements the equation of motion **with** forced degrees of freedom (if any).
         *
         *  \param sum_of_forces the sum of forces applying to the body
         *  \param x the complete states vector at current time `t`. It includes both the states of the present body and that of the other bodies.
         *  \param t the current time
         *  \param env the environment
         *  \returns dx_dt the complete states derivatives vectors in which the states derivatives of present body are updated.
         */
        void calculate_state_derivatives(const ssc::kinematics::Wrench& sum_of_forces,
                                         const StateType& x,
                                         StateType& dx_dt,
                                         const double t,
                                         const EnvironmentAndFrames& env) const;

        Eigen::Vector3d get_uvw(const StateType& x) const;
        Eigen::Vector3d get_pqr(const StateType& x) const;
        std::string get_name() const;
        ssc::kinematics::RotationMatrix get_rot_from_ned_to(const StateType& x) const;
        ssc::kinematics::EulerAngles get_angles(const StateType& all_states, const YamlRotation& c) const;
        std::tuple<double,double,double,double> get_quaternions(const ssc::kinematics::EulerAngles& angle, const YamlRotation& c) const;

        FilteredStates get_filtered_states() const;
        FilteredStates get_filtered_states(AbstractStates<History> state_history, const StateType& x) const;
        void feed(const StateType& x, Observer& observer, const YamlRotation& c) const;

        /** \brief Calculate the forcing wrench
         *  \details This method returns the wrench required to achieve forced states (characterized by the acceleration vector) given the current time the sum of forces applying to it.
         *  It implements the equation $\hat{\tau}=\mathbf{M_T}(\mathbf{\hat{\dot{V}}}_u)-\Sigma_i \tau_{i,\mathbf{G}})$.
         *
         *  \param dx_dt the time derivatives of the complete states vector
         *  \param sum_of_forces the sum of forces applying to the body at the body COG and in the body reference frame
         *  \returns The forcing wrench at the body COG
         */
        ssc::kinematics::Wrench get_delta_F(const StateType& dx_dt, const ssc::kinematics::Wrench& sum_of_other_forces) const;

        void set_states_history(const AbstractStates<History>& states);
        void reset_history();

    protected:
        BodyStates states;         // The body description (mechanical and geometric characteristics) and states history

    private:
        Body();

        size_t idx; //!< Index of the first state of body in state vector and state vector derivatives
        BlockedDOF blocked_states;
        StatesFilter states_filter;

        // DoFs blocking logic
        /// Matrices of forced DoFs
        Eigen::Matrix<double,6,6> m_delta_x; // Positions
        Eigen::Matrix<double,6,6> m_delta_v; // Velocities
        /// T_x operator (= T_tau,mu in 2023 JST paper)
        Eigen::Matrix<double,6,6> m_T_x;

};

typedef TR1(shared_ptr)<Body> BodyPtr;

#endif /* BODY_HPP_ */

/** @} */
