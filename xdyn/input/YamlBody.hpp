/*
 * YamlBody.hpp
 *
 *  Created on: 16 avr. 2014
 *      Author: cady
 */
/** \addtogroup Body
 *  \brief Input data for body initialization
 @{ */
#ifndef YAMLBODY_HPP_
    #define YAMLBODY_HPP_

    #include <string>
    #include <vector>

    #include "xdyn/input/YamlDynamics.hpp"
    #include "xdyn/input/YamlModel.hpp"
    #include "xdyn/input/YamlPosition.hpp"
    #include "xdyn/input/YamlSpeed.hpp"

enum class InterpolationType
{
    PIECEWISE_CONSTANT,
    LINEAR,
    SPLINE
};
enum class BlockableState
{
    X,
    Y,
    Z,
    U,
    V,
    W,
    P,
    Q,
    R,
    PHI,
    THETA,
    PSI
};
enum class FilterableState
{
    X,
    Y,
    Z,
    U,
    V,
    W,
    P,
    Q,
    R,
    PHI,
    THETA,
    PSI
};

template <typename T>
struct YamlDOF {
    YamlDOF() : t(), value(), howApplied({0., 0., 0., 0., 0., 0.}) {}
    virtual ~YamlDOF() = default;
    BlockableState state{BlockableState::U};
    T t;
    T value;
    InterpolationType interpolation{InterpolationType::PIECEWISE_CONSTANT};
    std::vector<double> howApplied;
};

struct YamlCSVDOF : public YamlDOF<std::string> {
    std::string filename;
};

struct YamlBlockedDOF {
    std::vector<YamlDOF<std::vector<double>>> from_yaml;
    std::vector<YamlCSVDOF> from_csv;
};

/** \brief One state's filter: an empty `type_of_filter` leaves the state as it is */
struct YamlStateFilter {
    std::string type_of_filter;
    double duration_in_seconds{};
};

struct YamlFilteredStates {
    YamlStateFilter x;
    YamlStateFilter y;
    YamlStateFilter z;
    YamlStateFilter u;
    YamlStateFilter v;
    YamlStateFilter w;
    YamlStateFilter p;
    YamlStateFilter q;
    YamlStateFilter r;
    YamlStateFilter phi;
    YamlStateFilter theta;
    YamlStateFilter psi;
};

struct YamlBody {
    std::string name;
    std::string mesh;
    YamlPosition position_of_body_frame_relative_to_mesh;
    YamlPosition initial_position_of_body_frame_relative_to_NED_projected_in_NED;
    YamlSpeed initial_velocity_of_body_frame_relative_to_NED_projected_in_body;
    YamlDynamics dynamics;
    std::vector<YamlModel> external_forces;
    YamlBlockedDOF blocked_dof;
    YamlFilteredStates filtered_states;
};

#endif /* YAMLBODY_HPP_ */

/** @} */
