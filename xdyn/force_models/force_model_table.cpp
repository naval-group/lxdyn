/*
 * force_model_table.cpp
 */

#include "force_model_table.hpp"

#include "AeroPolarForceModel.hpp"
#include "BasicBuoyancyForceModel.hpp"
#include "ConstantForceModel.hpp"
#include "DiffractionForceModel.hpp"
#include "ExactHydrostaticForceModel.hpp"
#include "FastHydrostaticForceModel.hpp"
#include "FlettnerRotorForceModel.hpp"
#include "FroudeKrylovForceModel.hpp"
#include "GMForceModel.hpp"
#include "GravityForceModel.hpp"
#include "HoltropMennenForceModel.hpp"
#include "HydroPolarForceModel.hpp"
#include "KtKqForceModel.hpp"
#include "LinearDampingForceModel.hpp"
#include "LinearFroudeKrylovForceModel.hpp"
#include "LinearHydrostaticForceModel.hpp"
#include "LinearStiffnessForceModel.hpp"
#include "MMGManeuveringForceModel.hpp"
#include "MMGPropellerForceModel.hpp"
#include "MMGRudderForceModel.hpp"
#include "ManeuveringForceModel.hpp"
#include "QuadraticDampingForceModel.hpp"
#include "RadiationDampingForceModel.hpp"
#include "ResistanceCurveForceModel.hpp"
#include "RudderForceModel.hpp"
#include "SimpleHeadingKeepingController.hpp"
#include "SimpleStationKeepingController.hpp"
#include "WageningenControlledForceModel.hpp"
#include "xdyn/core/ModelTable.hpp"

namespace {
template <typename T>
ModelEntry<ForceModelFactory> parsed()
{
    return {
        T::model_name(),
        [](const std::string& yaml, const std::string& body_name, const EnvironmentAndFrames& env)
            -> ForcePtr { return std::make_shared<T>(T::parse(yaml), body_name, env); }};
}

template <typename T>
ModelEntry<ForceModelFactory> parameterless()
{
    return {
        T::model_name(),
        [](const std::string&, const std::string& body_name, const EnvironmentAndFrames& env)
            -> ForcePtr { return std::make_shared<T>(body_name, env); }};
}

const auto& force_models()
{
    static const std::array table{
        parsed<AeroPolarForceModel>(),
        parsed<BasicBuoyancyForceModel>(),
        parsed<ConstantForceModel>(),
        parsed<DiffractionForceModel>(),
        parameterless<ExactHydrostaticForceModel>(),
        parameterless<FastHydrostaticForceModel>(),
        parsed<FlettnerRotorForceModel>(),
        parameterless<FroudeKrylovForceModel>(),
        parsed<GMForceModel>(),
        parameterless<GravityForceModel>(),
        parsed<HoltropMennenForceModel>(),
        parsed<HydroPolarForceModel>(),
        parsed<KtKqForceModel>(),
        parsed<LinearDampingForceModel>(),
        parsed<LinearFroudeKrylovForceModel>(),
        parsed<LinearHydrostaticForceModel>(),
        parsed<LinearStiffnessForceModel>(),
        parsed<MMGManeuveringForceModel>(),
        parsed<MMGPropellerForceModel>(),
        parsed<MMGRudderForceModel>(),
        parsed<ManeuveringForceModel>(),
        parsed<QuadraticDampingForceModel>(),
        parsed<RadiationDampingForceModel>(),
        parsed<ResistanceCurveForceModel>(),
        parsed<RudderForceModel>(),
        parsed<SimpleHeadingKeepingController>(),
        parsed<SimpleStationKeepingController>(),
        parsed<WageningenControlledForceModel>(),
    };
    return table;
}
}  // namespace

ForceModelFactory find_force_model(const std::string& name)
{
    return find_model(force_models(), name);
}

std::vector<std::string> force_model_names()
{
    return model_names(force_models());
}
