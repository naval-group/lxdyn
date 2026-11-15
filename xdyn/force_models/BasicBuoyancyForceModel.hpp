/*
 * BasicBuoyancyForceModel.hpp
 *
 * Archimedes on a fixed displaced volume: no mesh, no partial immersion.
 */

#ifndef BASICBUOYANCYFORCEMODEL_HPP_
#define BASICBUOYANCYFORCEMODEL_HPP_

#include <ssc/kinematics.hpp>
#include "xdyn/core/ForceModel.hpp"

class Body;
struct EnvironmentAndFrames;

class BasicBuoyancyForceModel : public ForceModel
{
    public:
        struct Input
        {
            double V{};
        };
        BasicBuoyancyForceModel(const Input& input, const std::string& body_name, const EnvironmentAndFrames& env);
        Wrench get_force(const BodyStates& states, const double t, const EnvironmentAndFrames& env, const std::map<std::string,double>& commands) const override;
        static Input parse(const std::string& yaml);
        double potential_energy(const BodyStates& states, const std::vector<double>& x) const;
        static std::string model_name();

    private:
        const double g;
        const double rho;
        const double V;
};

#endif /* BASICBUOYANCYFORCEMODEL_HPP_ */
