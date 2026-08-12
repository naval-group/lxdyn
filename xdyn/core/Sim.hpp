/*
 * Sim.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 *  Comments  : Apr 19, 2023
 *      Author: aba
 */
/**   \addtogroup Model
          *  \brief This is the model
          *  \details It is this class' responsibility to deal with the model. This includes:
          * - Initialization of the model
          * - Evaluation of the model states derivatives at any given time
          *  The model includes:
          * - An enviroment
          * - Bodies, each having 6 degrees of freedoms
          * - Forces, each applying to a body
          * - Instructions 
          @{
          */

#ifndef SIM_HPP_
#define SIM_HPP_

#include <vector>
#include <ssc/data_source.hpp>
#include <ssc/kinematics.hpp>
#include "xdyn/core/Body.hpp"
#include "xdyn/core/StateMacros.hpp"
#include "xdyn/core/EnvironmentAndFrames.hpp"
#include "xdyn/core/ForceModel.hpp"
#include "xdyn/core/SurfaceElevationGrid.hpp"
#include "xdyn/core/State.hpp"
#include <ssc/solver/ContinuousSystem.hpp>
#include <ssc/solver/DiscreteSystem.hpp>

typedef std::map<std::string, std::map< std::string,ssc::kinematics::Vector6d > > OuputtedForces;
typedef std::vector<std::pair<std::string,std::vector<std::string> > > VectorOfStringModelForEachBody;

class Observer;

class Sim : public ssc::solver::ContinuousSystem
{
    public:
        /** \brief Model initialization 
         *  \details Initialize the model. Required inputs are:
         * - bodies: a list of bodies
         * - forces: a list of forces applying to the bodies
         * - env: the environment object. It includes the environmental constants 
         * - x: the states (StateType == std::vector<double>). There are 13 states per body:
         *            _X(x,i) = x-coordinate of COG of body i in NED reference frame
         *            _Y(x,i) = y-coordinate of COG of body i in NED reference frame
         *            _Z(x,i) = z-coordinate of COG of body i in NED reference frame
         *            _U(x,i) = velocity of COG of body i along x axis of body reference frame
         *            _V(x,i) = velocity of COG of body i along y axis of body reference frame
         *            _W(x,i) = velocity of COG of body i along z axis of body reference frame
         *            _P(x,i) = coordinates of rotation vector of body i along x axis of body reference frame
         *            _Q(x,i) = coordinates of rotation vector of body i along y axis of body reference frame
         *            _R(x,i) = coordinates of rotation vector of body i along z axis of body reference frame
         *            _QR(x,i) = r-coordinate of quaternion of body i
         *            _QI(x,i) = i-coordinate of quaternion of body i
         *            _QJ(x,i) = j-coordinate of quaternion of body i
         *            _QK(x,i) = k-coordinate of quaternion of body i
         * - command_listener: the instructions
         */
        Sim(const std::vector<BodyPtr>& bodies,
            const std::vector<ListOfForces>& forces,
            const EnvironmentAndFrames& env,
            const StateType& x,
            const ssc::data_source::DataSource& command_listener);

        /** \brief Evaluation of the state derivatives
         *  \details Calculation of the state derivatives given the current state x and time t. 
         *  
         *  **Note:** The implementation of dx_dt is required to enable the use of the ssc::solver library. 
         *  Indeed, `Sim` is derived from `ssc:ContinousSystem` which includes a method to overload the operator() 
         *  with arguments (`x`, `dxdt`, `t`) which is required by ssc::solver library / Boost::numeric::odeint.
         *  
         *  **Note:** If some of the states are forced, dxdt does take into account that forcing (the corresponding dxdt are equal to the forced values).
         *  \param 
         *  \returns dxdt the vector of states derivatives.
        */
        void dx_dt(const StateType& x, StateType& dxdt, const double t);

        /** \brief Force states to a prescribed value 
         *  \details It is possible to make some states of some bodies follow a given path. The states
         *  which can be prescribed are u, v, w, p, q, r of any of the bodies. The data of which states are
         *  forced and how they are forced (how they change with time) is stored in the attribute `blocked_states` of the object `bodies`.
         *  It is the responsibility of this function to update the current states accordingly.
         *  
         *  *** THIS RESPONSIBILITY COULD BE ENTRUSTED TO THE TIME-STEPPER ? ***
         * 
         *  \returns Updated states according to forcing requirements
        */

        void force_states(StateType& x, const double t) const;

        /** @} */ 

        /**  \brief Serialize wave data on mesh for an ASCII observer
          *  \details Called by SimCsvObserver at each time step. The aim is to
          *  calculate the wave data on a mesh expressed in a particular frame of
          *  reference (eg. NED or body). For example we might want to calculate the
          *  wave data on a mesh surrounding the ship for visualization purposes.
          *  \returns Vector of coordinates on the free surface (in the NED frame),
          *           the z coordinate being the wave height (in meters)
          *  \snippet simulator/unit_tests/SimTest.cpp SimTest get_waves_example
          */
        ssc::kinematics::PointMatrix get_waves(const double t            //!< Current instant
                                              ) const;

        /** \brief Feed the observer with current data 
         *  \details This is a data acquisition method. It feeds the observer object with:
         *  - the states of the bodies. !!! the states are updated in the method to take into account forcing !!!
         *  - the forces, including sum of forces for each body, fictitious forces and possible additionnal forces required for the forcing of states.
         *  - the instructions (discrete_systems)
         * 
         *  *** THIS RESPONSIBILITY MAY BE TAKEN OUT OF THE SIM CLASS. Indeed, this method "just" gathers existing information and put it in one place (`obs` object). 
         *  But the "sensors" are defined in the configuration file (either by the user or everything by default). 
         *  Therefore, it may make more sense to use a sensors concept and take this method out ***
         * 
         *  \returns Updated observer
        */
        void output(const StateType& x, Observer& obs, double t, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& discrete_systems) const;
        ssc::data_source::DataSource& get_command_listener() const;

        void set_bodystates(const State& state_history);

        std::map<std::string,std::vector<ForcePtr> > get_forces() const;
        std::vector<BodyPtr> get_bodies() const;
        EnvironmentAndFrames get_env() const;

        void set_command_listener(const std::map<std::string, double>& new_commands);

        /** \brief Sets the value of one of the system's discrete states. In our case, these discrete states are the command values calculated by the controllers. This method is used by the controllers to store the updated command values in the DataSource, for use by controlled forces (e.g. propellers).
         */
        void set_discrete_state(const std::string &state_name, const double value);

        /** \brief Gets the value of the given input from the datasource
         *
         * Used by controllers to get the inputs they need (setpoints or commands) to compute a command.
         */
        double get_input_value(const std::string &name) const;

        /** \brief Gets the value of a Sim state ("x", "u", "qr", "phi", ...)
         *
         * Used by controllers to get the states they need to compute a command.
         */
        double get_state_value(const std::string &name) const;

        void reset_history();
        std::vector<std::string> get_command_names() const;

        /** \brief This function calls all force models from the current body states.
          * \detail This should be called before the first call to Sim::output if the initial state (at construction) is to be recorded.
          */
        void initialize_system_outputs_before_first_observation();

    private:
        ssc::kinematics::UnsafeWrench sum_of_forces(const StateType& x, const BodyPtr& body, const double t);

        /**  \brief Make sure quaternions can be converted to Euler angles
          *  \details Normalization takes place at each time step, which is not
          *  ideal because it means the model does not see the state values set
          *  by the stepper.
          */
        StateType normalize_quaternions(const StateType& all_states
                                       ) const;

        class Impl;
        TR1(shared_ptr)<Impl> pimpl;
};

#endif /* SIM_HPP_ */
