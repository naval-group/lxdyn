/*
 * current_model_table.cpp
 */

#include "current_model_table.hpp"

#include <array>

#include "xdyn/core/ModelTable.hpp"
#include "xdyn/current_models/ConstantUWCurrentModel.hpp"
#include "xdyn/current_models/DefaultUWCurrentModel.hpp"
#include "xdyn/current_models/EkmanUWCurrentModel.hpp"

namespace {
const auto& table()
{
    static const std::array table{
        parsed_model<UWCurrentModelPtr, ConstantUWCurrentModel>(),
        parsed_model<UWCurrentModelPtr, DefaultUWCurrentModel>(),
        parsed_model<UWCurrentModelPtr, EkmanUWCurrentModel>(),
    };
    return table;
}
}  // namespace

UWCurrentModelFactory find_current_model(const std::string& name)
{
    return find_model(table(), name);
}

std::vector<std::string> current_model_names()
{
    return model_names(table());
}
