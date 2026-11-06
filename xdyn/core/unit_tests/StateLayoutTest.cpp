#include "xdyn/core/StateLayout.hpp"

#include "gtest/gtest.h"
#include <type_traits>
#include <utility>

namespace
{
    const state::Var all_vars[] = {state::X, state::Y, state::Z, state::U, state::V, state::W, state::P,
                                   state::Q, state::R, state::QR, state::QI, state::QJ, state::QK};

    template <typename View> typename View::Value& accessor(const View& s, const state::Var v)
    {
        switch (v)
        {
            case state::X: return s.x();
            case state::Y: return s.y();
            case state::Z: return s.z();
            case state::U: return s.u();
            case state::V: return s.v();
            case state::W: return s.w();
            case state::P: return s.p();
            case state::Q: return s.q();
            case state::R: return s.r();
            case state::QR: return s.qr();
            case state::QI: return s.qi();
            case state::QJ: return s.qj();
            case state::QK: return s.qk();
            case state::N: break;
        }
        throw std::logic_error("no accessor");
    }

    StateType numbered_states(const size_t nb_of_bodies)
    {
        StateType x(nb_of_bodies * state::N);
        for (size_t i = 0; i < x.size(); ++i) x[i] = static_cast<double>(i);
        return x;
    }
}

static_assert(std::is_same_v<decltype(std::declval<const ConstBodyStateView&>().x()), const double&>);
static_assert(std::is_same_v<decltype(std::declval<const ConstBodyStateView&>().ptr(state::U)), const double*>);
static_assert(std::is_same_v<decltype(std::declval<const BodyStateView&>().x()), double&>);
static_assert(!std::is_constructible_v<BodyStateView, const StateType&, size_t>);

TEST(StateLayoutTest, index_is_body_major)
{
    ASSERT_EQ(13, state::N);
    ASSERT_EQ(0u, state::index(0, state::X));
    ASSERT_EQ(12u, state::index(0, state::QK));
    ASSERT_EQ(2u * 13 + 3, state::index(2, state::U));
}

TEST(StateLayoutTest, each_accessor_reads_the_slot_index_names)
{
    const StateType x = numbered_states(3);
    for (const size_t body : {0, 2})
    {
        ConstBodyStateView s(x, body);
        for (const auto v : all_vars)
        {
            ASSERT_EQ(x[state::index(body, v)], accessor(s, v)) << "body " << body << ", var " << v;
            ASSERT_EQ(&x[state::index(body, v)], s.ptr(v)) << "body " << body << ", var " << v;
        }
    }
}

TEST(StateLayoutTest, each_accessor_writes_only_the_slot_index_names)
{
    for (const size_t body : {0, 2})
    {
        for (const auto v : all_vars)
        {
            StateType x = numbered_states(3);
            BodyStateView s(x, body);
            accessor(s, v) = -1;
            StateType expected = numbered_states(3);
            expected[state::index(body, v)] = -1;
            ASSERT_EQ(expected, x) << "body " << body << ", var " << v;
        }
    }
}

TEST(StateLayoutTest, a_body_beyond_the_vector_throws)
{
    StateType x = numbered_states(2);
    ASSERT_NO_THROW(BodyStateView(x, 1));
    ASSERT_THROW(BodyStateView(x, 2), std::out_of_range);
    x.pop_back();
    ASSERT_THROW(BodyStateView(x, 1), std::out_of_range);
}
