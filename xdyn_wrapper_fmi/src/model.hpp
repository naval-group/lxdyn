#ifndef XDYN_WRAPPER_FMI_MODEL_HPP
#define XDYN_WRAPPER_FMI_MODEL_HPP

#include "xdyn/core/AssetPath.hpp"

#include <array>
#include <mutex>
#include <string>
#include <vector>

namespace xdyn_fmu
{
    // Value references, the contract between modelDescription.xml and the fmi2* functions.
    constexpr unsigned VR_STATE = 1;         // x y z u v w p q r qr qi qj qk
    constexpr unsigned VR_EULER = 14;        // phi theta psi
    constexpr unsigned VR_DT = 100;          // xdyn's integration step
    constexpr unsigned VR_SOLVER = 101;      // Enumeration xdyn.Solver, read as an Integer
    constexpr unsigned VR_RESET = 200;       // Boolean
    constexpr unsigned VR_RESET_STATE = 201; // 13 states
    constexpr unsigned VR_COMMAND = 1000;    // the model's commands, body by body, force by force

    // xdyn's solvers, in the order of the xdyn.Solver enumeration: solver k is item k + 1.
    const std::vector<std::string>& solvers();

    // A command input: xdyn's name for it, e.g. propeller(rpm), and the FMU's,
    // TestShip.propeller.rpm.
    struct Command
    {
        std::string xdyn_name;
        std::string name;
        double value = 0; // the YAML's own value, 0 where it has none
    };

    // What the FMU needs to know about a model before simulating it.
    struct Model
    {
        std::string body;
        std::string rotations; // the YAML's rotations convention, e.g. [psi, theta', phi'']
        std::vector<Command> commands;
        std::array<double, 13> state{};
    };

    std::mutex& assets_root_mutex();

    // Calls f with xdyn's asset root set to `assets_root`, the directory a YAML's relative paths
    // start from. The root is process-wide in xdyn and read while a model is built, so instances
    // built in parallel take turns; none is left set behind.
    template <typename F>
    auto with_assets_root(const std::string& assets_root, F f) -> decltype(f())
    {
        std::lock_guard<std::mutex> lock(assets_root_mutex());
        struct Restore
        {
            ~Restore() { xdyn::set_assets_root(""); }
        } restore;
        xdyn::set_assets_root(assets_root);
        return f();
    }

    Model read_model(const std::string& yaml, const std::string& assets_root);

    std::string model_description(const Model& model, const std::string& model_name, double dt,
                                  const std::string& solver);
} // namespace xdyn_fmu

#endif
