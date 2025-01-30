#include "MMGManeuveringForceModelTest.hpp"
#include "MMGManeuveringForceModel.hpp"

#define PI M_PI
#define EPS 1E-6

MMGManeuveringForceModelTest::MMGManeuveringForceModelTest()
{
}

MMGManeuveringForceModelTest::~MMGManeuveringForceModelTest()
{
}

std::string get_input_3dof();
std::string get_input_3dof()
{
    std::stringstream ss; // Data from H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y
    ss << "calculation point in body frame:\n"
        <<"    x: {value: -11.1, unit: m}\n"
        <<"    y: {value: 0, unit: m}\n"
        <<"    z: {value: 0, unit: m}\n"
        <<"Lpp: {value: 320, unit: m}\n"
        <<"T: {value: 20.8, unit: m}\n"
        <<"R0: 0.022\n"
        <<"Xvv: -0.04\n"
        <<"Xrr: 0.011\n"
        <<"Xvr: 0.002\n"
        <<"Xvvvv: 0.771\n"
        <<"Yv: -0.315\n"
        <<"Yr: 0.083\n"
        <<"Yvvv: -1.607\n"
        <<"Yrvv: 0.379\n"
        <<"Yvrr: -0.391\n"
        <<"Yrrr: 0.008\n"
        <<"Nv: -0.137\n"
        <<"Nr: -0.049\n"
        <<"Nvvv: -0.03\n"
        <<"Nrvv: -0.294\n"
        <<"Nvrr: 0.055\n"
        <<"Nrrr: -0.013\n"
        <<"GM: {value: 0, unit: m}\n"
        <<"zH: {value: 0, unit: m}\n"
        <<"Xvphi: 0\n"
        <<"Xrphi: 0\n"
        <<"Xphiphi: 0\n"
        <<"Yphi: 0\n"
        <<"Yphivv: 0\n"
        <<"Yvphiphi: 0\n"
        <<"Yphirr: 0\n"
        <<"Yrphiphi: 0\n"
        <<"Nphi: 0\n"
        <<"Nphivv: 0\n"
        <<"Nvphiphi: 0\n"
        <<"Nphirr: 0\n"
        <<"Nrphiphi: 0\n"
        <<"a: 0\n"
        <<"b: 0\n";

    return ss.str();
}

std::string get_input_4dof();
std::string get_input_4dof()
{
    std::stringstream ss; // Data from R. Okuda, H. Yasukawa, and A. Matsuda, “Validation of maneuvering simulations for a KCS at different forward speeds using the 4-DOF MMG method,” Ocean Engineering, vol. 284, p. 115174, Sep. 2023, doi: 10.1016/j.oceaneng.2023.115174.
    ss << "calculation point in body frame:\n"
        <<"    x: {value: -3.39, unit: m}\n"
        <<"    y: {value: 0, unit: m}\n"
        <<"    z: {value: 0, unit: m}\n"
        <<"Lpp: {value: 230, unit: m}\n"
        <<"T: {value: 10.8, unit: m}\n"
        <<"R0: 0.02\n"
        <<"Xvv: -0.087\n"
        <<"Xvr: -0.093\n"
        <<"Xrr: -0.023\n"
        <<"Xvvvv: 0.809\n"
        <<"Yv: -0.243\n"
        <<"Yr: 0.055\n"
        <<"Yvvv: -1.675\n"
        <<"Yrvv: -0.083\n"
        <<"Yvrr: -0.447\n"
        <<"Yrrr: 0.02\n"
        <<"Nv: -0.105\n"
        <<"Nr: -0.036\n"
        <<"Nvvv: -0.288\n"
        <<"Nrvv: -0.579\n"
        <<"Nvrr: -0.003\n"
        <<"Nrrr: -0.042\n"
        <<"GM: {value: 0.6, unit: m}\n"
        <<"zH: {value: 3.704, unit: m}\n"
        <<"Xvphi: 0.01\n"
        <<"Xrphi: 0.009\n"
        <<"Xphiphi: -0.002\n"
        <<"Yphi: 0.001\n"
        <<"Yphivv: 0.161\n"
        <<"Yvphiphi: -0.243\n"
        <<"Yphirr: 0\n"// unused in the article
        <<"Yrphiphi: -0.13\n"
        <<"Nphi: -0.009\n"
        <<"Nphivv: -0.327\n"
        <<"Nvphiphi: -0.232\n"
        <<"Nphirr: 0\n"// unused in the article
        <<"Nrphiphi: 0.041\n"
        <<"a: 0.031\n"
        <<"b: 0.002\n";

    return ss.str();
}

TEST_F(MMGManeuveringForceModelTest, can_parse)
{
    const auto data = MMGManeuveringForceModel::parse(get_input_4dof());
    ASSERT_DOUBLE_EQ(data.application_point.x, -3.39);
    ASSERT_DOUBLE_EQ(data.application_point.y, 0);
    ASSERT_DOUBLE_EQ(data.application_point.z, 0);
    ASSERT_DOUBLE_EQ(data.Lpp, 230);
    ASSERT_DOUBLE_EQ(data.T, 10.8);
    ASSERT_DOUBLE_EQ(data.R0, 0.02);
    ASSERT_DOUBLE_EQ(data.Xvv, -0.087);
    ASSERT_DOUBLE_EQ(data.Xrr, -0.023);
    ASSERT_DOUBLE_EQ(data.Xvr, -0.093);
    ASSERT_DOUBLE_EQ(data.Xvvvv, 0.809);
    ASSERT_DOUBLE_EQ(data.Yv, -0.243);
    ASSERT_DOUBLE_EQ(data.Yr, 0.055);
    ASSERT_DOUBLE_EQ(data.Yvvv,  -1.675);
    ASSERT_DOUBLE_EQ(data.Yrvv, -0.083);
    ASSERT_DOUBLE_EQ(data.Yvrr, -0.447);
    ASSERT_DOUBLE_EQ(data.Yrrr, 0.02);
    ASSERT_DOUBLE_EQ(data.Nv, -0.105);
    ASSERT_DOUBLE_EQ(data.Nr, -0.036);
    ASSERT_DOUBLE_EQ(data.Nvvv, -0.288);
    ASSERT_DOUBLE_EQ(data.Nrvv, -0.579);
    ASSERT_DOUBLE_EQ(data.Nvrr, -0.003);
    ASSERT_DOUBLE_EQ(data.Nrrr, -0.042);
    ASSERT_DOUBLE_EQ(data.GM, 0.6);
    ASSERT_DOUBLE_EQ(data.zH, 3.704);
    ASSERT_DOUBLE_EQ(data.Xvphi, 0.01);
    ASSERT_DOUBLE_EQ(data.Xrphi, 0.009);
    ASSERT_DOUBLE_EQ(data.Xphiphi, -0.002);
    ASSERT_DOUBLE_EQ(data.Yphi, 0.001);
    ASSERT_DOUBLE_EQ(data.Yphivv, 0.161);
    ASSERT_DOUBLE_EQ(data.Yvphiphi, -0.243);
    ASSERT_DOUBLE_EQ(data.Yphirr, 0.);
    ASSERT_DOUBLE_EQ(data.Yrphiphi, -0.13);
    ASSERT_DOUBLE_EQ(data.Nphi, -0.009);
    ASSERT_DOUBLE_EQ(data.Nphivv, -0.327);
    ASSERT_DOUBLE_EQ(data.Nvphiphi, -0.232);
    ASSERT_DOUBLE_EQ(data.Nphirr, 0.);
    ASSERT_DOUBLE_EQ(data.Nrphiphi, 0.041);
    ASSERT_DOUBLE_EQ(data.a, 0.031);
    ASSERT_DOUBLE_EQ(data.b, 0.002);
}

BodyStates get_states();
BodyStates get_states()
{
    BodyStates states;
    states.convention = YamlRotation("angle", {"z","y'","x''"});
    // Eigen leaves fixed-size matrices uninitialised, and the roll damping terms read both.
    states.solid_body_inertia = Matrix66::Zero();
    states.added_mass_matrix = Matrix66::Zero();
    states.x.record(0, 0);
    states.y.record(0, 0);
    states.z.record(0, 0);
    states.u.record(0, 1);
    states.v.record(0, 2);
    states.w.record(0, -.5);
    states.p.record(0, 1);
    states.q.record(0, -2);
    states.r.record(0, 3);
    states.qr.record(0, 1);
    states.qi.record(0, 0);
    states.qj.record(0, 0);
    states.qk.record(0, 0);
    return states;
}

TEST_F(MMGManeuveringForceModelTest, example_3dof)
/*
    Verification of the 3 DOF case with KVLCC input data
    The CoG is in (0,0,0) and the MMG frame origin is in (-11.1,0,0)
*/
{
    EnvironmentAndFrames env;
    env.rho = 1000;
    env.rot = YamlRotation("angle", {"z","y'","x''"});
    auto force_model = MMGManeuveringForceModel(MMGManeuveringForceModel::parse(get_input_3dof()), "body", env);
    auto states = get_states();
    states.added_mass_matrix(0,0)=0.5*env.rho*pow(320,2)*20.8*0.022; // mx'=0.022
    states.added_mass_matrix(1,1)=0.5*env.rho*pow(320,2)*20.8*0.223; // my'=0.223

    auto F = force_model.get_force(states, 0, env, {});
    ASSERT_DOUBLE_EQ(F.X(), 35918728522.330902-500*320*20.8*0.022*(1+pow((2-11.1*3),2))+0.223*500*320*320*20.8*(2-11.1*3)*3);
    ASSERT_DOUBLE_EQ(F.Y(), 2003246596640.8945-0.022*500*320*320*20.8*1*3);
    ASSERT_DOUBLE_EQ(F.Z(), 0);
    ASSERT_DOUBLE_EQ(F.K(), 0);
    ASSERT_DOUBLE_EQ(F.M(), 0);
    ASSERT_DOUBLE_EQ(F.N(), -455883858001617.88);
}

TEST_F(MMGManeuveringForceModelTest, test_get_inertia_matrix_in_Ob)
/*
    This test checks that the transport of an inertia matrix from the CoG to the MMG frame origin is well executed.
The data used is from:
R. Okuda, H. Yasukawa, and A. Matsuda, “Validation of maneuvering simulations for a KCS at different forward speeds using the 4-DOF MMG method,” Ocean Engineering, vol. 284, p. 115174, Sep. 2023, doi: 10.1016/j.oceaneng.2023.115174.
*/
{
    EnvironmentAndFrames env;
    env.rho = 1000;
    env.g=9.81;
    BodyStates states;

    // Define inputs as in the article
    const double mx=0.5*env.rho*pow(230,2)*10.8*0.006;//mx'=0.006
    const double my=0.5*env.rho*pow(230,2)*10.8*0.152;// my'=0.152
    const double Jx=0;// Jx included in Ix
    const double Jz=0.5*env.rho*pow(230,4)*10.8*0.009;//Jz'=0.009
    const double alphaZ=3.704;//alpha_Z=zH
    const double xG=-3.39;
    const double zG=-2.7;

    // Added mass matrix in G
    Matrix66 added_mass_matrix_in_G=Matrix66::Zero();
    added_mass_matrix_in_G(0,0)= mx;
    added_mass_matrix_in_G(1,1)= my;
    added_mass_matrix_in_G(1,3)= my*(zG-alphaZ);
    added_mass_matrix_in_G(1,5)= -my*xG;
    added_mass_matrix_in_G(3,3)= Jx + my*zG*zG - 2*my*zG*alphaZ;
    added_mass_matrix_in_G(3,5)= -my*xG*(zG-alphaZ);
    added_mass_matrix_in_G(5,5)= Jz + my*xG*xG;
    added_mass_matrix_in_G(3,1)= added_mass_matrix_in_G(1,3);
    added_mass_matrix_in_G(5,1)= added_mass_matrix_in_G(1,5);
    added_mass_matrix_in_G(5,3)= added_mass_matrix_in_G(3,5);

    Matrix66 added_mass_matrix_in_Ob=MMGManeuveringForceModel::get_inertia_matrix_in_Ob(added_mass_matrix_in_G,Eigen::Vector3d(xG,0,zG));
    ASSERT_NEAR(added_mass_matrix_in_Ob(0,0), mx, EPS);
    ASSERT_NEAR(added_mass_matrix_in_Ob(1,1), my, EPS);
    ASSERT_NEAR(added_mass_matrix_in_Ob(1,3), -my*alphaZ, EPS);
    ASSERT_NEAR(added_mass_matrix_in_Ob(3,1), -my*alphaZ, EPS);
    ASSERT_NEAR(added_mass_matrix_in_Ob(3,3), Jx, EPS);
    ASSERT_NEAR(added_mass_matrix_in_Ob(5,5), Jz, EPS);
}


TEST_F(MMGManeuveringForceModelTest, example_4dof)
/*
    Verification of the 4 DOF case with KCS input data
*/
{
    MMGManeuveringForceModel::Input input = MMGManeuveringForceModel::parse(get_input_4dof());
    EnvironmentAndFrames env;
    env.rho = 1000;
    env.g=9.81;
    env.rot = YamlRotation("angle", {"z","y'","x''"});

    // Define states
    BodyStates states;
    states.convention = YamlRotation("angle", {"z","y'","x''"});
    states.u.record(0, 10);
    states.v.record(0, -2);
    states.r.record(0, 1);

    // Define inputs as in the article, except zG=0
    const double m=52030000;
    const double Ix=m*pow(0.44*32.2,2);// Ix=m*(ixx*B)^2
    const double mx=0.5*env.rho*pow(230,2)*10.8*0.006;//mx'=0.006
    const double my=0.5*env.rho*pow(230,2)*10.8*0.152;// my'=0.152
    const double Jx=0;// Jx included in Ix
    const double Jz=0.5*env.rho*pow(230,4)*10.8*0.009;//Jz'=0.009
    const double alphaZ=3.704;//alpha_Z=zH
    double xG=-3.39;
    double zG=0;

    // Needed term from the inertia matrix
    states.solid_body_inertia(2,2)=m;// for the calculation of the heeling righting moment
    states.solid_body_inertia(3,3)=Ix;// for the calculation of the roll damping coefficients

    // Defined an added mass matrix in G
    states.added_mass_matrix=Matrix66::Zero();
    states.added_mass_matrix(0,0)= mx;
    states.added_mass_matrix(1,1)= my;
    states.added_mass_matrix(1,3)= my*(zG-alphaZ);
    states.added_mass_matrix(1,5)= -my*xG;
    states.added_mass_matrix(3,3)= Jx + my*zG*zG - 2*my*zG*alphaZ;
    states.added_mass_matrix(3,5)= -my*xG*(zG-alphaZ);
    states.added_mass_matrix(5,5)= Jz + my*xG*xG;
    states.added_mass_matrix(3,1)= states.added_mass_matrix(1,3);
    states.added_mass_matrix(5,1)= states.added_mass_matrix(1,5);
    states.added_mass_matrix(5,3)= states.added_mass_matrix(3,5);

    ////////////////////////////////
    // TEST1 : no phi, no dphi_dt //
    ////////////////////////////////
    
    auto force_model = MMGManeuveringForceModel(input, "body", env);
    auto F1 = force_model.get_force(states, 0, env, {});
    ASSERT_DOUBLE_EQ(F1.X(), -1601757553.1071172);
    ASSERT_DOUBLE_EQ(F1.Y(), 40686696096.544792);
    ASSERT_DOUBLE_EQ(F1.Z(), 0);
    ASSERT_DOUBLE_EQ(F1.K(), -3.704*40703835696.544792);
    ASSERT_DOUBLE_EQ(F1.M(), 0);
    ASSERT_DOUBLE_EQ(F1.N(), -12949573903321.889);

    ///////////////////////////////////
    // TEST 2 : phi=-30°, no dphi_dt //
    ///////////////////////////////////

    const double phi=-PI/6;
    // Get quaternions corresponding corresponding to a 30° heel angle
    auto quaternions=states.convert(ssc::kinematics::EulerAngles(phi,0,0),env.rot);
    states.qr.record(0,std::get<0>(quaternions));
    states.qi.record(0,std::get<1>(quaternions));
    states.qj.record(0,std::get<2>(quaternions));
    states.qk.record(0,std::get<3>(quaternions));

    // Compute some quantities for the comparison
    double U_target=sqrt(100+pow(5.39,2));//vm=-2-3.39*1=-5.39
    double dimension_const=500*pow(U_target,2)*230*10.8;//0.5*rho*U*U*Lpp*d

    auto F2 = force_model.get_force(states, 0, env, {});
    ASSERT_DOUBLE_EQ(F2.X(), -1601757553.1071172+(0.01*-5.39/U_target*phi+0.009*230/U_target*phi-0.002*phi*phi)*dimension_const);
    ASSERT_DOUBLE_EQ(F2.Y(), 40686696096.544792+(0.001*phi+0.161*phi*pow(5.39/U_target,2)-0.243*phi*phi*(-5.39)/U_target-0.13*230/U_target*phi*phi)*dimension_const);
    ASSERT_DOUBLE_EQ(F2.Z(), 0);
    ASSERT_DOUBLE_EQ(F2.K(), -3.704*40590119214.855896+m*9.81*0.6*PI/6);
    ASSERT_DOUBLE_EQ(F2.M(), 0);
    ASSERT_DOUBLE_EQ(F2.N(), -12949573903321.889+(-0.009*phi-0.327*phi*pow(5.39/U_target,2)-0.232*phi*phi*(-5.39)/U_target+0.041*230/U_target*phi*phi)*dimension_const*230);
    
    ///////////////////////////////////////////////
    // TEST 3 : phi=-30°,  dphi_dt=-0.5 and zG=0 //
    ///////////////////////////////////////////////

    states.p.record(0, -.5);// define dphi_dt=-0.5
    force_model = MMGManeuveringForceModel(input, "body", env);
    auto F3 = force_model.get_force(states, 0, env, {});
    ASSERT_DOUBLE_EQ(F3.X(), F2.X());
    ASSERT_DOUBLE_EQ(F3.Y(), F2.Y());
    ASSERT_DOUBLE_EQ(F3.Z(), 0);
    ASSERT_DOUBLE_EQ(F3.K(), F2.K()+35295092.1219939*.5+897604071.219159*.25);
    ASSERT_DOUBLE_EQ(F3.M(), 0);
    ASSERT_DOUBLE_EQ(F3.N(), F2.N());

    ///////////////////////////////////////////
    // TEST 4 : phi=-30°,  dphi_dt<>0, zG<>0 //
    ///////////////////////////////////////////

    states.G.z()=2;
    states.G.x()=-1;// xG is modified to have the same vm than TEST 2

    // The added mass matrix in G is updated in order to have a correct value for the roll damping coefficients
    zG+=2;
    xG-=-1;// updated for consistency but does not impact the roll inertia moment transport
    states.added_mass_matrix=Matrix66::Zero();
    states.added_mass_matrix(0,0)= mx;
    states.added_mass_matrix(1,1)= my;
    states.added_mass_matrix(1,3)= my*(zG-alphaZ);
    states.added_mass_matrix(1,5)= -my*xG;
    states.added_mass_matrix(3,3)= Jx + my*zG*zG - 2*my*zG*alphaZ;
    states.added_mass_matrix(3,5)= -my*xG*(zG-alphaZ);
    states.added_mass_matrix(5,5)= Jz + my*xG*xG;
    states.added_mass_matrix(3,1)= states.added_mass_matrix(1,3);
    states.added_mass_matrix(5,1)= states.added_mass_matrix(1,5);
    states.added_mass_matrix(5,3)= states.added_mass_matrix(3,5);

    force_model = MMGManeuveringForceModel(input, "body", env);
    auto F4 = force_model.get_force(states, 0, env, {});
    ASSERT_DOUBLE_EQ(F4.X(), F2.X());
    ASSERT_DOUBLE_EQ(F4.Y(), F2.Y());
    ASSERT_DOUBLE_EQ(F4.Z(), 0);
    ASSERT_DOUBLE_EQ(F4.K(), F3.K());
    ASSERT_DOUBLE_EQ(F4.M(), 0);
    ASSERT_DOUBLE_EQ(F4.N(), F2.N());
}