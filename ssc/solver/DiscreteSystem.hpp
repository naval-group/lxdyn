/*
 * DiscreteSystem.hpp
 *
 *  Created on: Feb 24, 2021
 *      Author: cady
 */

#ifndef SOLVER_DISCRETESYSTEM_HPP_
#define SOLVER_DISCRETESYSTEM_HPP_

#include <vector>

#include "ssc/solver/ContinuousSystem.hpp"
#include "ssc/solver/Scheduler.hpp"

namespace ssc
{
    namespace solver
    {
        class DiscreteSystem
        {
          public:
            DiscreteSystem(const double date_of_first_call, const double dt);
            virtual ~DiscreteSystem() = default;
            /**
             * @brief This is the callback that is called by the scheduler.
             *
             * Updates the discrete states and schedules the next discrete update.
             * @param scheduler Used to get the current time & schedule the next update.
             * @param system This can be used by the 'update' method to retrieve the continuous
             * states of the system.
             */
            void callback(Scheduler &scheduler, ContinuousSystem& system);

            /**
             * @brief Initialize the discrete states and add the first callabck to the scheduler.
             *
             * @param scheduler Used to get the start time & schedule the next update.
             * @param system This can be used by the 'update' method to retrieve the continuous
             * states of the system.
             */
            void initialize(Scheduler &scheduler, ContinuousSystem& system);

            /**
             * @brief Ask the scheduler to call the 'callback' method some time in the future.
             *
             * @param t Time (in seconds) at which the call should be made.
             * @param scheduler Used to schedule the discrete state update.
             */
            void schedule_update(const double t, Scheduler &scheduler);

            /**
             * @brief Returns the list of variables calculated by this system. Mainly used in case
             * the continuous system is an xdyn Sim class which contains a DataSource instance.
             * 
             */
            virtual std::vector<std::string> get_outputs() const;

            /**
             * @brief Get the name of the discrete system. By default, returns the 'default' string.
             * This is when the continuous system is an xdyn Sim class which contains a DataSource 
             * instance we wish to populate.
             * 
             * @return std::string 
             */
            virtual std::string get_name() const;

          private:
            DiscreteSystem();
            /**
             * @brief Get the date of next discrete state update.
             *
             * The default implementation is current_time+constant (most common case), but this
             * behaviour can be overriden. This method will be called by the "callback" public
             * method. If the date of the next update is equal to the current time, the controller
             * will not be called again.
             *
             * @param current_time Current simulation time, in seconds.
             * @return Date at which "callback" will be called (in seconds).
             */
            virtual double get_date_of_next_update(const double current_time) const;

            /**
             * @brief Update the discrete states.
             *
             * This method will be called by the public "callback" method.
             * @param time Current simulation time (in seconds).
             * @param system The continuous system. Used to retrieve the continuous states.
             */
            virtual void update_discrete_states(const double time, ContinuousSystem& system) = 0;

            double date_of_first_call; //!< Date of the first time the discrete system will be called
            double dt;
        };
    }
}

#endif /* SOLVER_DISCRETESYSTEM_HPP_ */
