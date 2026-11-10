#ifndef CORE_STATELAYOUT_HPP_
#define CORE_STATELAYOUT_HPP_

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using StateType = std::vector<double>;

namespace state
{
    // Thirteen consecutive slots per body: NED position, body-frame velocities, quaternion.
    enum Var { X, Y, Z, U, V, W, P, Q, R, QR, QI, QJ, QK, N };

    constexpr size_t index(const size_t body, const Var v)
    {
        return body * N + v;
    }
}

// The states of one body, by name. Use it through BodyStateView and ConstBodyStateView below.
template <typename Vector> class BodyStateViewOf
{
    public:
        using Value = std::conditional_t<std::is_const_v<Vector>, const double, double>;

        BodyStateViewOf(Vector& states, const size_t body) : base(first_state_of(states, body))
        {
        }

        Value& x() const { return base[state::X]; }
        Value& y() const { return base[state::Y]; }
        Value& z() const { return base[state::Z]; }
        Value& u() const { return base[state::U]; }
        Value& v() const { return base[state::V]; }
        Value& w() const { return base[state::W]; }
        Value& p() const { return base[state::P]; }
        Value& q() const { return base[state::Q]; }
        Value& r() const { return base[state::R]; }
        Value& qr() const { return base[state::QR]; }
        Value& qi() const { return base[state::QI]; }
        Value& qj() const { return base[state::QJ]; }
        Value& qk() const { return base[state::QK]; }

        // For an Eigen::Map over consecutive states, such as u, v, w.
        Value* ptr(const state::Var var) const { return base + var; }

    private:
        static Value* first_state_of(Vector& states, const size_t body)
        {
            if (states.size() < state::index(body + 1, state::X))
            {
                throw std::out_of_range("A state vector of size " + std::to_string(states.size())
                                        + " has no body " + std::to_string(body));
            }
            return states.data() + state::index(body, state::X);
        }

        Value* base;
};

using BodyStateView = BodyStateViewOf<StateType>;
using ConstBodyStateView = BodyStateViewOf<const StateType>;

#endif /* CORE_STATELAYOUT_HPP_ */
