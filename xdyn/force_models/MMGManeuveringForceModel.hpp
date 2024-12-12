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
        Input();
        YamlCoordinates application_point;
        double Lpp;
        double T;
        double R0;
        double Xvv;
        double Xrr;
        double Xvr;
        double Xvvvv;
        double Xvphi;
        double Xrphi;
        double Xphiphi;
        double Yv;
        double Yr;
        double Yvvv;
        double Yrvv;
        double Yvrr;
        double Yrrr;
        double Yphi;
        double Yphivv;
        double Yvphiphi;
        double Yphirr;
        double Yrphiphi;        
        double Nv;
        double Nr;
        double Nvvv;
        double Nrvv;
        double Nvrr;
        double Nrrr;
        double Nphi;
        double Nphivv;
        double Nvphiphi;
        double Nphirr;
        double Nrphiphi;
        double Kphi;
        double Kphiphi;
        double mx;//<! non-dimensional added mass coefficient a11
        double my;//<! non-dimensional added mass coefficient a22
        double GM;
    };
    MMGManeuveringForceModel(const Input& input, const std::string& body_name, const EnvironmentAndFrames& env);
    MMGManeuveringForceModel() = delete;
    Wrench get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const override;
    static Input parse(const std::string& yaml);
    static std::string model_name();

private:
    Input input;
};

#endif /* FORCE_MODELS_INC_MMGMANOEUVRINGFORCEMODEL_HPP_ */
