/*
 * wind_model_table.cpp
 */

#include "wind_model_table.hpp"

#include <array>

#include "xdyn/core/ModelTable.hpp"
#include "xdyn/wind_models/DefaultWindModel.hpp"
#include "xdyn/wind_models/LogWindVelocityProfile.hpp"
#include "xdyn/wind_models/PowerLawWindVelocityProfile.hpp"
#include "xdyn/wind_models/UniformWindVelocityProfile.hpp"

namespace {
const auto& table()
{
    static const std::array table{
        parsed_model<WindModelPtr, DefaultWindModel>(),
        parsed_model<WindModelPtr, LogWindVelocityProfile>(),
        parsed_model<WindModelPtr, PowerLawWindVelocityProfile>(),
        parsed_model<WindModelPtr, UniformWindVelocityProfile>(),
    };
    return table;
}
}  // namespace

WindModelFactory find_wind_model(const std::string& name)
{
    return find_model(table(), name);
}

std::vector<std::string> wind_model_names()
{
    return model_names(table());
}
