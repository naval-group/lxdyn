/*
 * ListOfObservers.cpp
 *
 *  Created on: Jan 12, 2015
 *      Author: cady
 */

#include "ListOfObservers.hpp"

#include <array>
#include <memory>

#include "CsvObserver.hpp"
#include "Hdf5Observer.hpp"
#include "JsonObserver.hpp"
#include "MapObserver.hpp"
#include "TsvObserver.hpp"
#include "WebSocketObserver.hpp"
#include "xdyn/core/ModelTable.hpp"
#include "xdyn/input/YamlOutput.hpp"

namespace {
using ObserverFactory = ObserverPtr (*)(const YamlOutput& output);

template <typename T>
ObserverPtr file_observer(const YamlOutput& output)
{
    if (output.full_output) {
        return std::make_shared<T>(output.filename);
    }
    return std::make_shared<T>(output.filename, output.data);
}

ObserverPtr map_observer(const YamlOutput& output)
{
    if (output.full_output) {
        return std::make_shared<MapObserver>();
    }
    return std::make_shared<MapObserver>(output.data);
}

ObserverPtr websocket_observer(const YamlOutput& output)
{
    if (output.full_output) {
        return std::make_shared<WebSocketObserver>(output.address, output.port);
    }
    return std::make_shared<WebSocketObserver>(output.address, output.port, output.data);
}

const auto& observers_by_format()
{
    static const std::array table{
        ModelEntry<ObserverFactory>{"csv", file_observer<CsvObserver>},
        ModelEntry<ObserverFactory>{"h5", file_observer<Hdf5Observer>},
        ModelEntry<ObserverFactory>{"hdf5", file_observer<Hdf5Observer>},
        ModelEntry<ObserverFactory>{"json", file_observer<JsonObserver>},
        ModelEntry<ObserverFactory>{"map", map_observer},
        ModelEntry<ObserverFactory>{"tsv", file_observer<TsvObserver>},
        ModelEntry<ObserverFactory>{"ws", websocket_observer},
    };
    return table;
}
}  // namespace

ObserverPtr ListOfObservers::parse_observer(const YamlOutput& output)
{
    const ObserverFactory factory = find_model(observers_by_format(), output.format);
    if (factory == nullptr) {
        throw_unknown_name("output format", output.format, model_names(observers_by_format()));
    }
    return factory(output);
}

ListOfObservers::ListOfObservers(const std::vector<YamlOutput>& yaml)
{
    for (const auto& output : yaml) {
        observers.push_back(parse_observer(output));
    }
}

ListOfObservers::ListOfObservers(const std::vector<ObserverPtr>& observers_) : observers(observers_)
{
}

void ListOfObservers::check_variables_to_serialize_are_available() const
{
    for (const auto& observer : observers) {
        observer->check_variables_to_serialize_are_available();
    }
}

void ListOfObservers::observe_before_solver_step(
    const Sim& sys,
    const double t,
    const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>>& discrete_systems)
{
    for (const auto& observer : observers) {
        observer->observe_before_solver_step(sys, t, discrete_systems);
    }
}

void ListOfObservers::observe_after_solver_step(
    const Sim& sys,
    const double t,
    const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>>& discrete_systems)
{
    for (const auto& observer : observers) {
        observer->observe_after_solver_step(sys, t, discrete_systems);
    }
}

void ListOfObservers::collect_available_serializations(
    const Sim& sys,
    const double t,
    const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem>>& discrete_systems)
{
    for (const auto& observer : observers) {
        observer->collect_available_serializations(sys, t, discrete_systems);
    }
}

void ListOfObservers::flush()
{
    for (const auto& observer : observers) {
        observer->flush();
    }
}

std::vector<ObserverPtr> ListOfObservers::get() const
{
    return observers;
}

bool ListOfObservers::empty() const
{
    return observers.empty();
}

void ListOfObservers::add_observer(const ObserverPtr& observer)
{
    observers.push_back(observer);
}
