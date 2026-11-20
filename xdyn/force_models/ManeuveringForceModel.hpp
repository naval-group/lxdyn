/*
 * ManeuveringForceModel.hpp
 *
 *  Created on: Jan 19, 2015
 *      Author: cady
 */

#ifndef MANEUVERINGFORCEMODEL_HPP_
#define MANEUVERINGFORCEMODEL_HPP_

#include <map>
#include <memory>
#include <ssc/data_source.hpp>
#include <ssc/macros.hpp>
#include <string>
#include <vector>

#include "ManeuveringInternal.hpp"
#include "xdyn/core/ForceModel.hpp"
#include "xdyn/input/YamlPosition.hpp"

class ManeuveringForceModel : public ForceModel {
public:
    struct Yaml {
        std::string name;
        YamlPosition frame_of_reference;
        std::vector<std::string> commands;
        std::map<std::string, std::string> var2expr;
    };
    ManeuveringForceModel(
        const Yaml& data,
        const std::string& body_name,
        const EnvironmentAndFrames& env);
    static Yaml parse(const std::string& yaml);
    Wrench get_force(
        const BodyStates& states,
        const double t,
        const EnvironmentAndFrames& env,
        const std::map<std::string, double>& commands) const override;
    static std::string model_name();

    double get_Tmax() const override;

private:
    ManeuveringForceModel();
    std::map<std::string, maneuvering::NodePtr> m;
    std::shared_ptr<ssc::data_source::DataSource> ds;
};

#endif /* MANEUVERINGFORCEMODEL_HPP_ */
