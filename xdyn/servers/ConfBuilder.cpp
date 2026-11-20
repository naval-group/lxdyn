#include "ConfBuilder.hpp"

#include "xdyn/simulator_api/simulator_api.hpp"
#include "xdyn/yaml_parser/SimulatorYamlParser.hpp"

namespace {
Sim sim_with_mesh(const std::string& yaml, const VectorOfVectorOfPoints& mesh)
{
    const YamlSimulatorInput input = SimulatorYamlParser(yaml).parse();
    return get_system(input, {{input.bodies.front().name, mesh}}, 0);
}
}  // namespace

ConfBuilder::ConfBuilder(const std::string& yaml)
    : sim(get_system(yaml, 0)), Tmax(sim.get_bodies().front()->get_states().x.get_Tmax())
{
}

ConfBuilder::ConfBuilder(const std::string& yaml, const VectorOfVectorOfPoints& mesh)
    : sim(sim_with_mesh(yaml, mesh)), Tmax(sim.get_bodies().front()->get_states().x.get_Tmax())
{
}
