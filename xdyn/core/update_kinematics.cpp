/*
 * update_kinematics.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */
#include "update_kinematics.hpp"
#include "Body.hpp"
#include "BodyBuilder.hpp"
#include "StateMacros.hpp"
#include "yaml2eigen.hpp"
#include "xdyn/external_data_structures/YamlAngle.hpp"
#include "xdyn/external_data_structures/YamlBody.hpp"

#include <ssc/kinematics.hpp>

#define SQUARE(x) ((x)*(x))

StateType get_initial_states(const YamlRotation& convention, const std::vector<YamlBody>& bodies)
{
    StateType ret(state::N*bodies.size(),0);
    BodyBuilder builder(convention);
    for (size_t i = 0 ; i < bodies.size() ; ++i)
    {
        const YamlAngle angle = bodies[i].initial_position_of_body_frame_relative_to_NED_projected_in_NED.angle;
        const ssc::kinematics::RotationMatrix R = angle2matrix(angle, convention);
        const Eigen::Quaternion<double> q(R);
        BodyStateView s(ret, i);
        s.x() = bodies[i].initial_position_of_body_frame_relative_to_NED_projected_in_NED.coordinates.x;
        s.y() = bodies[i].initial_position_of_body_frame_relative_to_NED_projected_in_NED.coordinates.y;
        s.z() = bodies[i].initial_position_of_body_frame_relative_to_NED_projected_in_NED.coordinates.z;
        s.u() = bodies[i].initial_velocity_of_body_frame_relative_to_NED_projected_in_body.u;
        s.v() = bodies[i].initial_velocity_of_body_frame_relative_to_NED_projected_in_body.v;
        s.w() = bodies[i].initial_velocity_of_body_frame_relative_to_NED_projected_in_body.w;
        s.p() = bodies[i].initial_velocity_of_body_frame_relative_to_NED_projected_in_body.p;
        s.q() = bodies[i].initial_velocity_of_body_frame_relative_to_NED_projected_in_body.q;
        s.r() = bodies[i].initial_velocity_of_body_frame_relative_to_NED_projected_in_body.r;
        const auto norm = sqrt((double)SQUARE(q.w())+(double)SQUARE(q.x())+(double)SQUARE(q.y())+(double)SQUARE(q.z()));
        s.qr() = q.w()/norm;
        s.qi() = q.x()/norm;
        s.qj() = q.y()/norm;
        s.qk() = q.z()/norm;
    }
    return ret;
}
