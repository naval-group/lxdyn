/*
 * Body.cpp
 *
 *  Created on: Jun 16, 2014
 *      Author: cady
 */

#include <cmath>
#include <iostream>

#include "Body.hpp"
#include "EnvironmentAndFrames.hpp"
#include "Observer.hpp"
#include "xdyn/exceptions/NumericalErrorException.hpp"
#include "yaml2eigen.hpp"

Body::Body(const size_t i, const BlockedDOF& blocked_states_, const YamlFilteredStates& filtered_states)
    : states(filtered_states)
    , idx(i)
    , blocked_states(blocked_states_)
    , states_filter(filtered_states)
    , m_T_x(blocked_states_.get_T_x())
{
}

Body::Body(const BodyStates& s, const size_t i, const BlockedDOF& blocked_states_, const YamlFilteredStates& filtered_states)
    : states(s)
    , idx(i)
    , blocked_states(blocked_states_)
    , states_filter(filtered_states)
    , m_T_x(blocked_states_.get_T_x())
{
}

Body::Body(const size_t i, const BlockedDOF& blocked_states_, const StatesFilter& states_filter_)
    : states(states_filter_)
    , idx(i)
    , blocked_states(blocked_states_)
    , states_filter(states_filter_)
    , m_T_x(blocked_states_.get_T_x())
{
}

Body::Body(const BodyStates& states_, const size_t i, const BlockedDOF& blocked_states_, const StatesFilter& states_filter_)
    : states(states_)
    , idx(i)
    , blocked_states(blocked_states_)
    , states_filter(states_filter_)
    , m_T_x(blocked_states_.get_T_x())
{
}

Body::~Body()
{
}

BodyStates Body::get_states() const
{
    return states;
}

#define SQUARE(x) ((x)*(x))

ssc::kinematics::Point Body::get_origin(const StateType& x) const
{
    return ssc::kinematics::Point("NED", *_X(x,idx),
                                         *_Y(x,idx),
                                         *_Z(x,idx));
}

ssc::kinematics::Point Body::get_position_of_body_relative_to_mesh() const
{
    return ssc::kinematics::Point(std::string("mesh(")+states.name+")",
                                  states.x_relative_to_mesh,
                                  states.y_relative_to_mesh,
                                  states.z_relative_to_mesh);
}

ssc::kinematics::Transform Body::get_transform_from_mesh_to_body() const
{
    return ssc::kinematics::Transform(get_position_of_body_relative_to_mesh(), states.mesh_to_body, states.name);
}

ssc::kinematics::Transform Body::get_transform_from_ned_to_body(const StateType& x) const
{
    return ssc::kinematics::Transform(get_origin(x), states.get_rot_from_ned_to(x, idx), states.name);
}

ssc::kinematics::Transform Body::get_transform_from_ned_to_local_ned(const StateType& x) const
{
    return ssc::kinematics::Transform(get_origin(x), std::string("NED(") + states.name + ")");
}

void Body::update_kinematics(const StateType& x, const ssc::kinematics::KinematicsPtr& k) const
{
    k->add(get_transform_from_ned_to_body(x));
    k->add(get_transform_from_ned_to_local_ned(x));
}

StateType Body::block_states_if_necessary(StateType x, const double t) const
{
    force_states(x,t);
    return x;
}

void Body::update_body_states(StateType x, const double t)
{
    force_states(x,t);
    states.x.record(t, *_X(x,idx));
    states.y.record(t, *_Y(x,idx));
    states.z.record(t, *_Z(x,idx));
    states.u.record(t, *_U(x,idx));
    states.v.record(t, *_V(x,idx));
    states.w.record(t, *_W(x,idx));
    states.p.record(t, *_P(x,idx));
    states.q.record(t, *_Q(x,idx));
    states.r.record(t, *_R(x,idx));
    states.qr.record(t, *_QR(x,idx));
    states.qi.record(t, *_QI(x,idx));
    states.qj.record(t, *_QJ(x,idx));
    states.qk.record(t, *_QK(x,idx));
}

void Body::force_states(StateType& x, const double t) const
{
    // Forcing matrices (forced DoFs) at current time
    Eigen::Matrix<double,6,6> delta_x = blocked_states.get_delta_x(t);
    Eigen::Matrix<double,6,6> delta_v = blocked_states.get_delta_v(t);
    // If motion is forced
    if (delta_x.maxCoeff() > 0)
    {
        // Without forcing
        // Velocities
        Eigen::Vector3d uvw=get_uvw(x); // initial [u, v, w]
        Eigen::Vector3d pqr=get_pqr(x); // initial [p, q, r]
        Eigen::Matrix<double,6,1> Vu;
        Vu.block<3,1>(0,0) = uvw;
        Vu.block<3,1>(3,0) = pqr;
        // Get the angles of rotations without forcing
        ssc::kinematics::EulerAngles rot(get_angles(x, states.convention));
        // Vector of forced motions
        Eigen::Matrix<double,6,1> x_f;
        x_f << *_X(x,idx), *_Y(x,idx), *_Z(x,idx), rot.phi, rot.theta, rot.psi ; // Initialize forced rotations vector
        blocked_states.get_forced_x_k(t, x_f, 0); // Get the forced motions
        if (delta_x(0,0) == 1) {*_X(x,idx) = x_f(0);} // Update x if forced
        if (delta_x(1,1) == 1) {*_Y(x,idx) = x_f(1);} // Update y if forced
        if (delta_x(2,2) == 1) {*_Z(x,idx) = x_f(2);} // Update z if forced
        if (delta_x(3,3) == 1) {rot.phi = x_f(3);} // Update phi if forced
        if (delta_x(4,4) == 1) {rot.theta = x_f(4);} // Update theta if forced
        if (delta_x(5,5) == 1) {rot.psi = x_f(5);} // Update psi if forced
        Eigen::Quaternion<double> q1;
        if ((states.convention.order_by == "angle") && (match(states.convention.convention, "z", "y'", "x''")))
        {
            // Update quaternion with forced motion
            Eigen::Quaternion<double> q_temp(ssc::kinematics::rotation_matrix<ssc::kinematics::INTRINSIC, ssc::kinematics::CHANGING_ANGLE_ORDER, 3, 2, 1>(rot));
            q1=q_temp;
            *_QR(x,idx)=q1.w();
            *_QI(x,idx)=q1.x();
            *_QJ(x,idx)=q1.y();
            *_QK(x,idx)=q1.z();
        }
        else
        {
            THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "This rotation convention is not currently supported for forced angles.");
        }

        blocked_states.get_forced_x_k(t, x_f, 1); // Get the forced motions
        // Velocities
        // Calculate the derivatives of the angles of rotations without forcing but with forced quaternions
        ssc::kinematics::EulerAngles drot; // Initialization : drot = [0, 0, 0]
        Eigen::Quaternion<double> dq1; // Define dq1 for convenience
        dq1.w()=0.5*(-*_P(x,idx)*q1.x()-*_Q(x,idx)*q1.y()-*_R(x,idx)*q1.z());
        dq1.x()=0.5*(*_P(x,idx)*q1.w()-*_Q(x,idx)*q1.z()+*_R(x,idx)*q1.y());
        dq1.y()=0.5*(*_Q(x,idx)*q1.w()-*_R(x,idx)*q1.x()+*_P(x,idx)*q1.z());
        dq1.z()=0.5*(*_R(x,idx)*q1.w()-*_P(x,idx)*q1.y()+*_Q(x,idx)*q1.x());
        if ((states.convention.order_by == "angle") && (match(states.convention.convention, "z", "y'", "x''")))
        {
            const double eps = 1e-13;
            // dtheta/dt
            if (fabs(cos(rot.theta)) > eps)
            {
                drot.theta=1./cos(rot.theta)*2.*(dq1.w()*q1.y()+q1.w()*dq1.y()-dq1.x()*q1.z()-q1.x()*dq1.z());
            }
            else
            {
                THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "Pitch angle is 90 degrees.");
            }
            // dphi/dt
            if (fabs(cos(rot.theta)) > eps)
            {
                drot.phi=1./cos(rot.theta)*(2.*(dq1.w()*q1.x()+q1.w()*dq1.x()+dq1.y()*q1.z()+q1.y()*dq1.z())*cos(rot.phi)
                        +4.*(q1.x()*dq1.x()+q1.y()*dq1.y())*sin(rot.phi));
            }
            else
            {
                THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "Pitch angle is 90 degrees.");
            }
            // dpsi/dt
            if (fabs(cos(rot.theta)) > eps)
            {
                drot.psi=1./cos(rot.theta)*(2.*(dq1.x()*q1.y()+q1.x()*dq1.y()+dq1.w()*q1.z()+q1.w()*dq1.z())*cos(rot.psi)
                        +4.*(q1.y()*dq1.y()+q1.z()*dq1.z())*sin(rot.psi));
            }
            else
            {
                THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "Pitch angle is 90 degrees.");
            }
        }
        else
        {
            THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "This rotation convention is not currently supported for forced angles.");
        }
        // Update angular velocity vector (pqr)
        Eigen::Matrix<double,6,1> dx_f;  // Vector of forced rotations (if any)
        dx_f << 0.,0.,0.,drot.phi, drot.theta, drot.psi; // Initialization
        blocked_states.get_forced_x_k(t, dx_f, 1); // Get the forced rotations
        if (delta_x(3,3) == 1) {drot.phi = dx_f(3);} // Update phi if forced
        if (delta_x(4,4) == 1) {drot.theta = dx_f(4);} // Update theta if forced
        if (delta_x(5,5) == 1) {drot.psi = dx_f(5);} // Update psi if forced
        // Calculate dq taking into accound forced rotations
        Eigen::Matrix<double,4,4> Aq;
        Aq(0,0) = q1.w();
        Aq(0,1) = q1.x();
        Aq(0,2) = q1.y();
        Aq(0,3) = q1.z();
        Aq(1,0) = 2.*q1.x()*cos(rot.phi);
        Aq(1,1) = 2.*q1.w()*cos(rot.phi)+4.*q1.x()*sin(rot.phi);
        Aq(1,2) = 2.*q1.z()*cos(rot.phi)+4.*q1.y()*sin(rot.phi);
        Aq(1,3) = 2.*q1.y()*cos(rot.phi);
        Aq(2,0) = 2.*q1.y();
        Aq(2,1) = -2.*q1.z();
        Aq(2,2) = 2.*q1.w();
        Aq(2,3) = -2.*q1.x();
        Aq(3,0) = 2.*q1.z()*cos(rot.psi);
        Aq(3,1) = 2.*q1.y()*cos(rot.psi);
        Aq(3,2) = 2.*q1.x()*cos(rot.psi)+4.*q1.y()*sin(rot.psi);
        Aq(3,3) = 2.*q1.w()*cos(rot.psi)+4.*q1.z()*sin(rot.psi);
        Eigen::Matrix<double,4,1> fq;
        fq(0)=0.;
        fq(1)=drot.phi*cos(rot.theta);
        fq(2)=drot.theta*cos(rot.theta);
        fq(3)=drot.psi*cos(rot.theta);
        Eigen::Matrix<double,4,1> dq_f = Aq.colPivHouseholderQr().solve(fq);
        // Calculate p, q, r taking into forced rotations
        Eigen::Matrix3d A;
        A(0,0) = 0.5*q1.w();
        A(0,1) = -0.5*q1.z();
        A(0,2) = 0.5*q1.y();
        A(1,0) = 0.5*q1.z();
        A(1,1) = 0.5*q1.w();
        A(1,2) = -0.5*q1.x();
        A(2,0) = -0.5*q1.y();
        A(2,1) = 0.5*q1.x();
        A(2,2) = 0.5*q1.w();
        Eigen::Vector3d RHS;
        RHS(0)=dq_f(1);
        RHS(1)=dq_f(2);
        RHS(2)=dq_f(3);
        pqr = A.colPivHouseholderQr().solve(RHS);
        Vu.block<3,1>(3,0) = pqr;
        // Update uvw
        // -  (Updated = with forcing) rotation matrix R_{NED,b}
        const ssc::kinematics::RotationMatrix& R = states.get_rot_from_ned_to(x, idx);
        // - Operator T_pg
        Eigen::Matrix<double,3,6> T_pg;
        T_pg <<     1., 0., 0., 0.          , -states.G.z(), states.G.y(),
                    0., 1., 0., states.G.z(), 0., -states.G.x(),
                    0., 0., 1., -states.G.y(), states.G.x(), 0.;
        Eigen::Vector3d dx(R*(T_pg*Vu));    // Vector of forced positions
        dx_f(0)=dx(0);
        dx_f(1)=dx(1);
        dx_f(2)=dx(2);
        blocked_states.get_forced_x_k(t, dx_f, 1); // Get the forced velocities
        dx(0)=dx_f(0);
        dx(1)=dx_f(1);
        dx(2)=dx_f(2);
        Eigen::Matrix<double,6,1> zpqr;
        zpqr << 0.,0.,0.,pqr(0),pqr(1),pqr(2);
        uvw=R.transpose()*dx-T_pg*zpqr;
        Vu.block<3,1>(0,0) = uvw;
    }

     // If forced Velocities
    if (delta_v.maxCoeff() > 0)
    {
        // Update velocities
        blocked_states.force_states(x,t);
    }

}

void Body::update_projection_of_z_in_mesh_frame(const double g,
                                                const ssc::kinematics::KinematicsPtr& k)
{
    const ssc::kinematics::Point g_in_NED("NED", 0, 0, g);
    const ssc::kinematics::RotationMatrix ned2mesh = k->get("NED", std::string("mesh(") + states.name + ")").get_rot();
    states.g_in_mesh_frame = ned2mesh*g_in_NED.v;
}

#define CHECK(x,y,t) if (std::isnan(x)) {THROW(__PRETTY_FUNCTION__,NumericalErrorException,"NaN detected in state " << y << ", at t = " << t);}
void Body::update(const EnvironmentAndFrames& env, const StateType& x, const double t)
{
    CHECK(*_X(x,idx),"X",t);
    CHECK(*_Y(x,idx),"Y",t);
    CHECK(*_Z(x,idx),"Z",t);
    CHECK(*_U(x,idx),"U",t);
    CHECK(*_V(x,idx),"V",t);
    CHECK(*_W(x,idx),"W",t);
    CHECK(*_P(x,idx),"P",t);
    CHECK(*_Q(x,idx),"Q",t);
    CHECK(*_R(x,idx),"R",t);
    CHECK(*_QR(x,idx),"QR",t);
    CHECK(*_QI(x,idx),"QI",t);
    CHECK(*_QJ(x,idx),"QJ",t);
    CHECK(*_QK(x,idx),"QK",t);
    update_kinematics(x,env.k);
    update_body_states(x, t);
    update_intersection_with_free_surface(env, t);
    update_projection_of_z_in_mesh_frame(env.g, env.k);
}

void Body::set_history(const EnvironmentAndFrames& env, const State& states)
{
    set_states_history(states);
    if (not(states.x.is_empty()))
    {
        update_kinematics(states.get_StateType(states.x.size()-1), env.k);
        update_intersection_with_free_surface(env, states.x.get_current_time());
    }
    update_projection_of_z_in_mesh_frame(env.g, env.k);
}

void Body::calculate_state_derivatives(const ssc::kinematics::Wrench& sum_of_forces,
                                         const StateType& x,
                                         StateType& dx_dt,
                                         const double t,
                                         const EnvironmentAndFrames& env) const
{
    // 1. First, calculate the state derivatives without forcing
    // du/dt, dv/dt, dw/dt, dp/dt, dq/dt, dr/dt
    Eigen::Map<Eigen::Matrix<double,6,1> > dXdt(_U(dx_dt,idx)); // &dXdt = &dx_dt[13*idx+3]
    dXdt = states.inverse_of_the_total_inertia * sum_of_forces.to_vector(); // As &dXdt = &dx_dt[13*idx+3], the components of dx_dt corresponding to [du/dt, dv/dt, dw/dt, dp/dt, dq/dt, dr/dt] are updated after that.

    // dx/dt, dy/dt, dz/dt
    const ssc::kinematics::RotationMatrix& R = env.k->get("NED", states.name).get_rot(); // R_{NED,body}
    const Eigen::Map<const Eigen::Vector3d> uvw(_U(x,idx));
    const Eigen::Map<const Eigen::Vector3d> pqr(_P(x,idx));
    const Eigen::Vector3d XpYpZp(R*uvw);
    // The water the body moves through is itself moving: the current advects the hull. Both terms
    // are already in NED, so the current needs no rotation here.
    const Eigen::Vector3d current_in_ned = env.get_UWCurrent(Eigen::Vector3d(*_X(x,idx), *_Y(x,idx), *_Z(x,idx)), t);
    *_X(dx_dt,idx) = XpYpZp(0) + current_in_ned(0);
    *_Y(dx_dt,idx) = XpYpZp(1) + current_in_ned(1);
    *_Z(dx_dt,idx) = XpYpZp(2) + current_in_ned(2);

    // dqr/dt, dqi/dt, dqj/dt, dqk/dt
    const Eigen::Quaternion<double> q1(*_QR(x,idx),
                                       *_QI(x,idx),
                                       *_QJ(x,idx),
                                       *_QK(x,idx)); // q1 = [qr, qi, qj, qk]
    const Eigen::Quaternion<double> q2(0,*_P(x,idx),*_Q(x,idx),*_R(x,idx)); // q12= [0, p, q, r]
    const Eigen::Quaternion<double>& dq_dt = q1*q2; //
    *_QR(dx_dt,idx) = 0.5*(double)dq_dt.w(); // dx_dt[13*idx+10] = dqr/dt
    *_QI(dx_dt,idx) = 0.5*(double)dq_dt.x(); // dx_dt[13*idx+11] = dqi/dt
    *_QJ(dx_dt,idx) = 0.5*(double)dq_dt.y(); // dx_dt[13*idx+12] = dqj/dt
    *_QK(dx_dt,idx) = 0.5*(double)dq_dt.z(); // dx_dt[13*idx+13] = dqk/dt

    // 2. Second, calculate the state derivatives with forcing
    // 2.0 Forcing matrices (forced DoFs) at current time
    Eigen::Matrix<double,6,6> delta_x = blocked_states.get_delta_x(t);
    Eigen::Matrix<double,6,6> delta_v = blocked_states.get_delta_v(t);
    // 2.1 If motion is forced
    if (delta_x.maxCoeff() > 0)
    {
        // Define dq1 for convenience
        const Eigen::Quaternion<double> dq1(*_QR(dx_dt,idx),*_QI(dx_dt,idx),*_QJ(dx_dt,idx),*_QK(dx_dt,idx));
        // Get the angles of rotations
        ssc::kinematics::EulerAngles rot(states.get_angles());
        // Calculate the derivatives of the angles of rotations
        ssc::kinematics::EulerAngles drot; // Initialization : drot = [0, 0, 0]
        if ((states.convention.order_by == "angle") && (match(states.convention.convention, "z", "y'", "x''")))
        {
            const double eps=1e-13;
            // dtheta/dt
            if (fabs(cos(rot.theta)) > eps)
            {
                drot.theta=1./cos(rot.theta)*2.*(dq1.w()*q1.y()+q1.w()*dq1.y()-dq1.x()*q1.z()-q1.x()*dq1.z());
            }
            else
            {
                THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "Pitch angle is 90 degrees.");
            }
            // dphi/dt
            if (fabs(cos(rot.theta)) > eps)
            {
                drot.phi=1./cos(rot.theta)*(2.*(dq1.w()*q1.x()+q1.w()*dq1.x()+dq1.y()*q1.z()+q1.y()*dq1.z())*cos(rot.phi)
                        +4.*(q1.x()*dq1.x()+q1.y()*dq1.y())*sin(rot.phi));
            }
            else
            {
                THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "Pitch angle is 90 degrees.");
            }
            // dpsi/dt
            if (fabs(cos(rot.theta)) > eps)
            {
                drot.psi=1./cos(rot.theta)*(2.*(dq1.x()*q1.y()+q1.x()*dq1.y()+dq1.w()*q1.z()+q1.w()*dq1.z())*cos(rot.psi)
                        +4.*(q1.y()*dq1.y()+q1.z()*dq1.z())*sin(rot.psi));
            }
            else
            {
                THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "Pitch angle is 90 degrees.");
            }
        }
        else
        {
            THROW(__PRETTY_FUNCTION__, ssc::exception_handling::Exception, "This rotation convention is not currently supported for forced angles.");
        }
        // Calculate ddXi
        // - Operator T_pg
        Eigen::Matrix<double,3,6> T_pg;
        T_pg <<     1., 0., 0., 0.          , -states.G.z(), states.G.y(),
                    0., 1., 0., states.G.z(), 0., -states.G.x(),
                    0., 0., 1., -states.G.y(), states.G.x(), 0.;
        // - Operator T_qw
        Eigen::Matrix<double,4,3> T_qw;
        T_qw <<    -q1.x()    , -q1.y()  , -q1.z(),
                    q1.w()     , -q1.z()  , q1.y(),
                    q1.z()     , q1.w()   , -q1.x(),
                    -q1.y()    , q1.x()   , q1.w();
        // - Operator dT_qw
        Eigen::Matrix<double,4,3> dT_qw;
        dT_qw <<    -dq1.x()    , -dq1.y()  , -dq1.z(),
                    dq1.w()     , -dq1.z()  , dq1.y(),
                    dq1.z()     , dq1.w()   , -dq1.x(),
                    -dq1.y()    , dq1.x()   , dq1.w();
        // - Operator Lambda_theta
        Eigen::Matrix<double,1,4> Lambda_theta;
        Lambda_theta << q1.y(), -q1.z(), q1.w(), -q1.x();
        // - Operator Lambda_phi
        Eigen::Matrix<double,1,4> Lambda_phi;
        Lambda_phi << q1.x()*cos(rot.phi), q1.w()*cos(rot.phi)+2.*q1.x()*sin(rot.phi), q1.z()*cos(rot.phi)+2.*q1.y()*sin(rot.phi), q1.y()*cos(rot.phi);
        // - Operator Lambda_psi
        Eigen::Matrix<double,1,4> Lambda_psi;
        Lambda_psi << q1.z()*cos(rot.psi), q1.y()*cos(rot.psi), q1.x()*cos(rot.psi)+2.*q1.y()*sin(rot.psi), q1.w()*cos(rot.psi)+2.*q1.z()*sin(rot.psi);
        // - Operator Lambda
        Eigen::Matrix<double,3,6> Lambda = Eigen::Matrix<double,3,6>::Zero();
        Eigen::Matrix<double,3,4> temp;
        temp.block<1,4>(0,0) = Lambda_phi;
        temp.block<1,4>(1,0) = Lambda_theta;
        temp.block<1,4>(2,0) = Lambda_psi;
        Lambda.block<3,3>(0,3) = temp * T_qw;
        // - Operator dR_NED
        Eigen::Matrix3d dR_NED;
        dR_NED <<   2.*(q1.w()*dq1.w()+q1.x()*dq1.x()-q1.y()*dq1.y()-q1.z()*dq1.z()),
        2.*(dq1.x()*q1.y()+q1.x()*dq1.y()-dq1.w()*q1.z()-q1.w()*dq1.z()),
        2.*(dq1.x()*q1.z()+q1.x()*dq1.z()+dq1.w()*q1.y()+q1.w()*dq1.y()),
        2.*(dq1.x()*q1.y()+q1.x()*dq1.y()+dq1.w()*q1.z()+q1.w()*dq1.z()),
        2.*(q1.w()*dq1.w()+q1.y()*dq1.y()-q1.x()*dq1.x()-q1.z()*dq1.z()),
        2.*(dq1.y()*q1.z()+q1.y()*dq1.z()-dq1.w()*q1.x()-q1.w()*dq1.x()),
        2.*(dq1.x()*q1.z()+q1.x()*dq1.z()-dq1.w()*q1.y()-q1.w()*dq1.y()),
        2.*(dq1.y()*q1.z()+q1.y()*dq1.z()+dq1.w()*q1.x()+q1.w()*dq1.x()),
        2.*(q1.w()*dq1.w()+q1.z()*dq1.z()-q1.x()*dq1.x()-q1.y()*dq1.y());
        // - Operator sigma
        Eigen::Vector3d sigma;
        sigma(0)=-(Lambda_phi*dT_qw)*pqr
             -4.*(dq1.w()*dq1.x()+dq1.y()*dq1.z())*cos(rot.phi)
             +2.*drot.phi*sin(rot.phi)*(dq1.w()*q1.x()+q1.w()*dq1.x()+dq1.y()*q1.z()+q1.y()*dq1.z())
             -4.*(dq1.x()*dq1.x()+dq1.y()*dq1.y())*sin(rot.phi)
             -4.*drot.phi*cos(rot.phi)*(q1.x()*dq1.x()+q1.y()*dq1.y())
             -2.*drot.phi*tan(rot.theta)*(dq1.w()*q1.y()+q1.w()*dq1.y()-dq1.x()*q1.z()-q1.x()*dq1.z());
        sigma(1)=-(Lambda_theta*dT_qw)*pqr-rot.theta*rot.theta*sin(rot.theta)-4.*(dq1.w()*dq1.y()-dq1.x()*dq1.z());
        sigma(2)=-(Lambda_psi*dT_qw)*pqr
             -4.*(dq1.x()*dq1.y()+dq1.w()*dq1.z())*cos(rot.psi)
             +2.*drot.psi*sin(rot.psi)*(dq1.x()*q1.y()+q1.x()*dq1.y()+dq1.w()*q1.z()+q1.w()*dq1.z())
             -4.*(dq1.y()*dq1.y()+dq1.z()*dq1.z())*sin(rot.psi)
             -4.*drot.psi*cos(rot.psi)*(q1.y()*dq1.y()+q1.z()*dq1.z())
             -2.*drot.psi*tan(rot.theta)*(dq1.w()*q1.y()+q1.w()*dq1.y()-dq1.x()*q1.z()-q1.x()*dq1.z());
/*        // - Matrix equation cos theta * ddXi = A * dVu + B
        Eigen::Matrix<double,6,6> A;
        A.block<3,6>(0,0) = R*T_pg*cos(rot.theta);
        A.block<3,6>(3,0) = Lambda;
        Eigen::Matrix<double,6,1> Vu;
        Vu << uvw(0), uvw(1), uvw(2), pqr(0), pqr(1), pqr(2);
        Eigen::Matrix<double,6,1> B;
        B.block<3,1>(0,0) = dR_NED*T_pg*Vu*cos(rot.theta);
        B.block<3,1>(3,0) = -sigma;
        Eigen::Matrix<double,6,1> ddXi;
        ddXi = (A*dXdt+B)/cos(rot.theta);
         // - Update with the forced DoFs
        blocked_states.get_forced_x_k(t, ddXi, 2);
        // - Update the forced acceleration vector
        dXdt = (A.colPivHouseholderQr().solve(ddXi*cos(rot.theta)-B)); */
        // - Matrix A
        Eigen::Matrix<double,6,6> A;
        A.block<3,6>(0,0) = R*T_pg*cos(rot.theta);
        A.block<3,6>(3,0) = Lambda;
        // - Vector B
        Eigen::Matrix<double,6,1> Vu;
        Vu << uvw(0), uvw(1), uvw(2), pqr(0), pqr(1), pqr(2);
        Eigen::Matrix<double,6,1> B;
        B.block<3,1>(0,0) = dR_NED*T_pg*Vu*cos(rot.theta);
        B.block<3,1>(3,0) = -sigma;
        // - tilde_ddXi
        Eigen::Matrix<double,6,1> ddXi = Eigen::Matrix<double,6,1>::Zero();
        blocked_states.get_forced_x_k(t, ddXi, 2);
        // - RHS
        Eigen::Matrix<double,6,1> RHS( ddXi*cos(rot.theta) - A*dXdt - B);
        // - Operator T_Xi_Y
        Eigen::Matrix<double,6,6> T_Xi_Y = Eigen::Matrix<double,6,6>::Zero();
        int j = 0;
        for (int i = 0; i < 6; i++) {if ((delta_x.rowwise().maxCoeff())(i) == 0) {T_Xi_Y(i,j)=1.;j++;}}
        // - Operator T_tau_Y
        Eigen::Matrix<double,6,6> T_tau_Y = Eigen::Matrix<double,6,6>::Zero();
        j = 5;
        for (int i = 0; i < 6; i++) {
            if ((delta_x.rowwise().maxCoeff())(i) > 0) {
                for (int k=0; k<6; k++) {T_tau_Y(k,j)=m_T_x(k,i);} // Put the forcing pattern at the end of the T_tau_Y operator
                j--;
            }
        }
        // - Matrix LHS
        Eigen::Matrix<double,6,6> LHS( A*states.inverse_of_the_total_inertia * T_tau_Y - T_Xi_Y * cos(rot.theta));
        // - Calculate Y
        Eigen::Matrix<double,6,1> Y;
        Y = LHS.colPivHouseholderQr().solve(RHS);
        // - Update the forced acceleration vector
        dXdt = dXdt + states.inverse_of_the_total_inertia * T_tau_Y*Y;
    }

    // 2.2 If velocity is forced
    if (delta_v.maxCoeff() > 0)
    {
        // Update with forced accelerations
        Eigen::Matrix<double,6,1> dV_f(dXdt);
        blocked_states.get_forced_v_k(t, dV_f, 1);
        dXdt(0)=dV_f(0);
        dXdt(1)=dV_f(1);
        dXdt(2)=dV_f(2);
        dXdt(3)=dV_f(3);
        dXdt(4)=dV_f(4);
        dXdt(5)=dV_f(5);
    }
}

Eigen::Vector3d Body::get_uvw(const StateType& x) const
{
    return Eigen::Vector3d::Map(_U(x,idx));
}

Eigen::Vector3d Body::get_pqr(const StateType& x) const
{
    return Eigen::Vector3d::Map(_P(x,idx));
}

ssc::kinematics::Wrench Body::get_delta_F(const StateType& dx_dt, const ssc::kinematics::Wrench& sum_of_other_forces) const
{
    Eigen::Matrix<double, 6, 1> dV;
    Eigen::Vector3d uvw(get_uvw(dx_dt));
    dV(0)=uvw(0);
    dV(1)=uvw(1);
    dV(2)=uvw(2);
    Eigen::Vector3d pqr(get_pqr(dx_dt));
    dV(3)=pqr(0);
    dV(4)=pqr(1);
    dV(5)=pqr(2);
    Eigen::Matrix<double, 6, 1> Tau_F;
    Tau_F=states.total_inertia*dV-sum_of_other_forces.to_vector();
    ssc::kinematics::Wrench Tau(sum_of_other_forces.get_point(),Tau_F);
    return Tau;
}

void Body::feed(const StateType& x, Observer& observer, const YamlRotation& c) const
{
    observer.write_after_solver_step(*_X(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"X"},std::string("x(")+states.name+")"));
    observer.write_after_solver_step(*_Y(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"Y"},std::string("y(")+states.name+")"));
    observer.write_after_solver_step(*_Z(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"Z"},std::string("z(")+states.name+")"));
    observer.write_after_solver_step(*_U(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"U"},std::string("u(")+states.name+")"));
    observer.write_after_solver_step(*_V(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"V"},std::string("v(")+states.name+")"));
    observer.write_after_solver_step(*_W(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"W"},std::string("w(")+states.name+")"));
    observer.write_after_solver_step(*_P(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"P"},std::string("p(")+states.name+")"));
    observer.write_after_solver_step(*_Q(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"Q"},std::string("q(")+states.name+")"));
    observer.write_after_solver_step(*_R(x,idx), DataAddressing(std::vector<std::string>{"states",states.name,"R"},std::string("r(")+states.name+")"));
    observer.write_after_solver_step(*_QR(x,idx),DataAddressing(std::vector<std::string>{"states",states.name,"Quat","Qr"},std::string("qr(")+states.name+")"));
    observer.write_after_solver_step(*_QI(x,idx),DataAddressing(std::vector<std::string>{"states",states.name,"Quat","Qi"},std::string("qi(")+states.name+")"));
    observer.write_after_solver_step(*_QJ(x,idx),DataAddressing(std::vector<std::string>{"states",states.name,"Quat","Qj"},std::string("qj(")+states.name+")"));
    observer.write_after_solver_step(*_QK(x,idx),DataAddressing(std::vector<std::string>{"states",states.name,"Quat","Qk"},std::string("qk(")+states.name+")"));
    const auto angles = get_angles(x, c);
    observer.write_after_solver_step(angles.phi, DataAddressing(std::vector<std::string>{"states",states.name,"PHI"},std::string("phi(")+states.name+")"));
    observer.write_after_solver_step(angles.theta, DataAddressing(std::vector<std::string>{"states",states.name,"THETA"},std::string("theta(")+states.name+")"));
    observer.write_after_solver_step(angles.psi, DataAddressing(std::vector<std::string>{"states",states.name,"PSI"},std::string("psi(")+states.name+")"));

    const auto filtered_states = get_filtered_states(states, x);
    observer.write_after_solver_step(filtered_states.x, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"X"},std::string("x_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.y, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"Y"},std::string("y_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.z, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"Z"},std::string("z_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.u, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"U"},std::string("u_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.v, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"V"},std::string("v_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.w, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"W"},std::string("w_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.p, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"P"},std::string("p_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.q, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"Q"},std::string("q_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.r, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"R"},std::string("r_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.phi, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"PHI"},std::string("phi_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.theta, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"THETA"},std::string("theta_filtered(")+states.name+")"));
    observer.write_after_solver_step(filtered_states.psi, DataAddressing(std::vector<std::string>{"filtered_states",states.name,"PSI"},std::string("psi_filtered(")+states.name+")"));
}

std::string Body::get_name() const
{
    return states.name;
}

ssc::kinematics::RotationMatrix Body::get_rot_from_ned_to(const StateType& x) const
{
    return states.get_rot_from_ned_to(x,idx);
}

ssc::kinematics::EulerAngles Body::get_angles(const StateType& all_states, const YamlRotation& c) const
{
    return states.get_angles(all_states, idx, c);
}

std::tuple<double,double,double,double> Body::get_quaternions(const ssc::kinematics::EulerAngles& angle, const YamlRotation& c) const
{
    return states.convert(angle,c);
}

void Body::set_states_history(const AbstractStates<History>& s)
{
    states = s;
}

void Body::reset_history()
{
    states.x.reset();
    states.y.reset();
    states.z.reset();
    states.u.reset();
    states.v.reset();
    states.w.reset();
    states.p.reset();
    states.q.reset();
    states.r.reset();
    states.qr.reset();
    states.qi.reset();
    states.qj.reset();
    states.qk.reset();
}

FilteredStates Body::get_filtered_states() const
{
    return FilteredStates(states_filter, states, states.convention);
}

FilteredStates Body::get_filtered_states(AbstractStates<History> state_history, const StateType& x) const
{
    const double t = state_history.x.get_current_time();
    state_history.x.record(t, *_X(x,idx));
    state_history.y.record(t, *_Y(x,idx));
    state_history.z.record(t, *_Z(x,idx));
    state_history.u.record(t, *_U(x,idx));
    state_history.v.record(t, *_V(x,idx));
    state_history.w.record(t, *_W(x,idx));
    state_history.p.record(t, *_P(x,idx));
    state_history.q.record(t, *_Q(x,idx));
    state_history.r.record(t, *_R(x,idx));
    state_history.qr.record(t, *_QR(x,idx));
    state_history.qi.record(t, *_QI(x,idx));
    state_history.qj.record(t, *_QJ(x,idx));
    state_history.qk.record(t, *_QK(x,idx));
    return FilteredStates(states_filter, state_history, states.convention);
}