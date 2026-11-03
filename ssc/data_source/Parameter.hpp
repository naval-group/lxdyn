/*
 * Parameter.hpp
 *
 * \date 15 mars 2013, 14:52:01
 * \author cec
 */

#ifndef PARAMETER_HPP_
#define PARAMETER_HPP_

#include <string>

/** \author: cec
 *  \ingroup data_source
 *  \brief Contains all states of a module
 *  \details It is not of a DataSourceModule's responsibility to store its own
 *  states: this was done to allow dynamic connections & disconnections of modules
 *  during the simulation. Therefore, if a module needs internal states, it has
 *  to declare them as Parameters.
*/
template <class T> class Parameter
{
    public:
        std::string module_name;
};


#endif /* PARAMETER_HPP_ */
