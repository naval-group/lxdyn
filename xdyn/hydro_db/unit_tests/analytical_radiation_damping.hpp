#ifndef HDB_INTERPOLATORS_UNIT_TESTS_ANALYTICAL_RADIATION_DAMPING_HPP_
#define HDB_INTERPOLATORS_UNIT_TESTS_ANALYTICAL_RADIATION_DAMPING_HPP_

#include <cmath>

// A damping Br and its retardation function K in closed form, to check the numerical transforms.
namespace test_data {
inline double analytical_Br(const double omega)
{
    return 0.5 * (0.1 / (0.01 + (0.5 - omega) * (0.5 - omega)) +
                  0.1 / (0.01 + (0.5 + omega) * (0.5 + omega)));
}

inline double analytical_K(const double tau)
{
    return std::exp(-0.1 * tau) * std::cos(0.5 * tau);
}
}  // namespace test_data

#endif
