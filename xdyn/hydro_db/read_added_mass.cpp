/*
 * read_added_mass.cpp
 */

#include "read_added_mass.hpp"

#include "xdyn/exceptions/InternalErrorException.hpp"
#include "xdyn/hydro_db/HDBParser.hpp"
#include "xdyn/hydro_db/PrecalParser.hpp"

namespace {
Eigen::Matrix<double, 6, 6> read_matrix(const YamlDynamics6x6Matrix& added_mass)
{
    if (not added_mass.hdb_filename.empty()) {
        return HDBParser::from_file(added_mass.hdb_filename).get_added_mass();
    }
    if (not added_mass.precal_filename.empty()) {
        return PrecalParser::from_file(added_mass.precal_filename).get_added_mass();
    }
    THROW(
        __PRETTY_FUNCTION__,
        InternalErrorException,
        "Error reading the added mass matrix: xdyn was expecting a filename (as value for key 'from PRECAL_R' or 'from HDB') but an empty string was found.");
    return {};
}
}  // namespace

YamlDynamics6x6Matrix read_added_mass(const YamlDynamics6x6Matrix& added_mass)
{
    if (not added_mass.read_from_file) {
        return added_mass;
    }
    const Eigen::Matrix<double, 6, 6> Ma = read_matrix(added_mass);
    YamlDynamics6x6Matrix ret;
    ret.frame = added_mass.frame;
    for (auto [i, row] : {
             std::pair{0, &ret.row_1},
             {1, &ret.row_2},
             {2, &ret.row_3},
             {3, &ret.row_4},
             {4, &ret.row_5},
             {5, &ret.row_6},
         }) {
        for (Eigen::Index j = 0; j < 6; ++j) {
            row->push_back(Ma(i, j));
        }
    }
    return ret;
}
