#ifndef FORCE_MODELS_INC_MMGMANOEUVRINGFORCEMODEL_HPP_
#define FORCE_MODELS_INC_MMGMANOEUVRINGFORCEMODEL_HPP_

#include "xdyn/external_data_structures/YamlCoordinates.hpp"
#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"

#include "xdyn/core/ForceModel.hpp"

/** \details This class was created to define the maneuvering model used in the MMG model
 * 
 *  It was originally based on the 3 DOF MMG model as described in
 * H. Yasukawa and Y. Yoshimura, “Introduction of MMG standard method for ship maneuvering predictions,” Journal of Marine Science and Technology, vol. 20, no. 1, pp. 37–52, Nov. 2014, doi: 10.1007/s00773-014-0293-y.
 * 
 * More recently, it has been extended to 4 DOF, as described in
 * R. Okuda, H. Yasukawa, and A. Matsuda, “Validation of maneuvering simulations for a KCS at different forward speeds using the 4-DOF MMG method,” Ocean Engineering, vol. 284, p. 115174, Sep. 2023, doi: 10.1016/j.oceaneng.2023.115174.
 * 
 *  \addtogroup ForceModels
 *  \ingroup module
 *  \section 
 *  \snippet module/unit_tests/MMGManeuveringForceModelTest.cpp
 */
class MMGManeuveringForceModel : public ForceModel
{
public:
    struct Input
    {
        YamlCoordinates application_point;
        double Lpp{0.0};
        double T{0.0};
        double R0{0.0};
        double Xvv{0.0};
        double Xrr{0.0};
        double Xvr{0.0};
        double Xvvvv{0.0};
        double Xvphi{0.0};
        double Xrphi{0.0};
        double Xphiphi{0.0};
        double Yv{0.0};
        double Yr{0.0};
        double Yvvv{0.0};
        double Yrvv{0.0};
        double Yvrr{0.0};
        double Yrrr{0.0};
        double Yphi{0.0};
        double Yphivv{0.0};
        double Yvphiphi{0.0};
        double Yphirr{0.0};
        double Yrphiphi{0.0};
        double Nv{0.0};
        double Nr{0.0};
        double Nvvv{0.0};
        double Nrvv{0.0};
        double Nvrr{0.0};
        double Nrrr{0.0};
        double Nphi{0.0};
        double Nphivv{0.0};
        double Nvphiphi{0.0};
        double Nphirr{0.0};
        double Nrphiphi{0.0};
        double a{0.0};
        double b{0.0};//<! Caution: b is in "1/deg"
        double GM{0.0};
        double zH{0.0};//<! Vertical position of the center of effort in body frame
    };
    MMGManeuveringForceModel(const Input& input, const std::string& body_name, const EnvironmentAndFrames& env);
    MMGManeuveringForceModel() = delete;
    Wrench get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const override;
    static Input parse(const std::string& yaml);
    static std::string model_name();
    static Matrix66 get_inertia_matrix_in_Ob(Matrix66 inertia_matrix_in_G, Eigen::Vector3d G_in_MMG_frame);

private:
    Input input;
};

#endif /* FORCE_MODELS_INC_MMGMANOEUVRINGFORCEMODEL_HPP_ */
