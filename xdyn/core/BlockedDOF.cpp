/*
 * BlockedDOF.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: cady
 */

#include <algorithm>
#include <map>
#include <vector>

#include <Eigen/Dense>

#include "BlockedDOF.hpp"
#include "xdyn/exceptions/InvalidInputException.hpp"
#include "xdyn/exceptions/NumericalErrorException.hpp"
#include "xdyn/yaml_parser/external_data_structures_parsers.hpp"

#include <ssc/csv_file_reader.hpp>
#include <ssc/interpolation.hpp>
#include <ssc/text_file_reader.hpp>

typedef YamlDOF<std::vector<double> > Table;

// std::ostream& operator<<(std::ostream& os, const BlockableState& s);
std::ostream& operator<<(std::ostream& os, const BlockableState& s)
{
    switch(s)
    {
        case BlockableState::X:
            os << "x";
            break;
        case BlockableState::Y:
            os << "y";
            break;
        case BlockableState::Z:
            os << "z";
            break;
        case BlockableState::U:
            os << "u";
            break;
        case BlockableState::V:
            os << "v";
            break;
        case BlockableState::W:
            os << "w";
            break;
        case BlockableState::P:
            os << "p";
            break;
        case BlockableState::Q:
            os << "q";
            break;
        case BlockableState::R:
            os << "r";
            break;
        case BlockableState::PHI:
            os << "phi";
            break;
        case BlockableState::THETA:
            os << "theta";
            break;
        case BlockableState::PSI:
            os << "psi";
            break;
        default:
            break;
    }
    return os;
}

typedef TR1(shared_ptr)<ssc::interpolation::Interpolator> Interpolator;

class Builder
{
    public:
        Builder(const YamlBlockedDOF& yaml) : input(yaml), tables(build_tables())
        {
            check_states_are_not_defined_twice(input);
        }

        std::vector<Table> build_tables() const
        {
            std::vector<Table> t = input.from_yaml;
            for (const auto& y:input.from_csv)
            {
                t.push_back(read_table_from_csv(y));
            }
            return t;
        }

        std::map<BlockableState, Interpolator> get_forced_states() const
        {
            std::map<BlockableState, Interpolator> ret;
            for (const auto& table:tables)
            {
                ret[table.state] = build_interpolator(table);
            }
            return ret;
        }

        std::map<BlockableState, double> get_tmin() const
        {
            std::map<BlockableState, double> tmin;
            for (const auto& table:tables)
            {
                const auto min = std::min_element(table.t.begin(), table.t.end());
                tmin[table.state] = *min;
            }
            return tmin;
        }

        std::map<BlockableState, double> get_tmax() const
        {
            std::map<BlockableState, double> tmax;
            for (const auto& table:tables)
            {
                const auto max = std::max_element(table.t.begin(), table.t.end());
                tmax[table.state] = *max;
            }
            return tmax;
        }

        std::map<BlockableState, std::array<double,6>> getHowApplied() const
        {
            std::map<BlockableState, std::array<double,6>> howApplied;
            for (const auto& DoF:input.from_yaml)
            {
                std::array<double,6> temp;
                for (std::size_t i=0; i<6; ++i) {temp[i] = DoF.howApplied[i];}
                howApplied[DoF.state] = temp;
            }
            return howApplied;
        }

    private:
        Builder();

        void throw_if_already_defined(const BlockableState& state, std::map<BlockableState, bool>& defined) const
        {
            if (defined[state])
            {
                THROW(__PRETTY_FUNCTION__, InvalidInputException, "Attempting to define state " << state << " twice in 'blocked dof' section of the YAML file.");
            }
            defined[state] = true;
        }

        void check_states_are_not_defined_twice(const YamlBlockedDOF& input) const
        {
            std::map<BlockableState, bool> defined_in_yaml, defined_in_csv;
            for (const auto& state : input.from_yaml)
            {
                throw_if_already_defined(state.state, defined_in_yaml);
            }
            for (const auto& state : input.from_csv)
            {
                throw_if_already_defined(state.state, defined_in_yaml);
                throw_if_already_defined(state.state, defined_in_csv);
            }
        }
        Interpolator build_interpolator(const Table& y) const
        {
            try
            {
                switch(y.interpolation)
                {
                    case InterpolationType::LINEAR:
                        return Interpolator(new ssc::interpolation::LinearInterpolationVariableStep(y.t, y.value));
                        break;
                    case InterpolationType::PIECEWISE_CONSTANT:
                        return Interpolator(new ssc::interpolation::PiecewiseConstantVariableStep<double>(y.t, y.value));
                        break;
                    case InterpolationType::SPLINE:
                        return Interpolator(new ssc::interpolation::SplineVariableStep(y.t, y.value));
                        break;
                    default:
                        break;
                }
                return Interpolator();
            }
            catch(const ssc::exception_handling::Exception& e)
            {
                THROW(__PRETTY_FUNCTION__, InvalidInputException, "Error when building forced state '" << y.state << "' defined in 'forced DOF/from CSV': " << e.get_message());
            }
            catch(const std::exception& e)
            {
                THROW(__PRETTY_FUNCTION__, InvalidInputException, "Error when building forced state '" << y.state << "' defined in 'forced DOF/from CSV': " << e.what());
            }
            return Interpolator();
        }

        Table read_table_from_csv(const YamlCSVDOF& y) const
        {
            Table table;
            try
            {
                const ssc::text_file_reader::TextFileReader txt(y.filename);
                const ssc::csv_file_reader::CSVFileReader reader(txt.get_contents());
                auto m = reader.get_map();
                const auto it1 = m.find(y.t);
                if (it1 == m.end())
                {
                    THROW(__PRETTY_FUNCTION__, InvalidInputException, "Unable to find column " << y.t << " in CSV file " << y.filename);
                }
                const auto it2 = m.find(y.value);
                if (it2 == m.end())
                {
                    THROW(__PRETTY_FUNCTION__, InvalidInputException, "Unable to find column " << y.value << " in CSV file " << y.filename);
                }
                table.interpolation = y.interpolation;
                table.t = it1->second;
                table.value = it2->second;
            }
            catch(const ssc::exception_handling::Exception& e)
            {
                THROW(__PRETTY_FUNCTION__, InvalidInputException, "Error when building forced state '" << y.state << "' defined in 'forced DOF/from CSV': " << e.get_message());
            }
            catch(const std::exception& e)
            {
                THROW(__PRETTY_FUNCTION__, InvalidInputException, "Error when building forced state '" << y.state << "' defined in 'forced DOF/from CSV': " << e.what());
            }
            return table;
        }

        YamlBlockedDOF input;
        std::vector<Table> tables;
};

struct BlockedDOF::Impl
{
    Impl(Builder builder, const size_t body_idx_)
            : blocked_dof(builder.get_forced_states())
            , body_idx(body_idx_)
            , tmin(builder.get_tmin())
            , tmax(builder.get_tmax())
            , m_delta_x(Eigen::Matrix<double,6,6>::Zero())
            , m_delta_v(Eigen::Matrix<double,6,6>::Zero())
            , m_T_x(Eigen::Matrix<double,6,6>::Zero())
    {
        // Initialize matrices of forced DoFs
        m_delta_x <<    0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0;
        m_delta_v <<    0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0;
        m_T_x <<        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0;
        for (const auto &dof:blocked_dof)
        {
            switch(dof.first)
            {
            case BlockableState::X:
                    {m_delta_x(0,0)+=1;
                    std::array<double,6> howApplied = builder.getHowApplied()[BlockableState::X];
                    for (std::size_t i=0; i<6; ++i) {m_T_x(i,0) = howApplied[i];}
                    break;}
            case BlockableState::Y:
                    {m_delta_x(1,1)+=1;
                    std::array<double,6> howApplied = builder.getHowApplied()[BlockableState::Y];
                    for (std::size_t i=0; i<6; ++i) {m_T_x(i,1) = howApplied[i];}
                    break;}
                case BlockableState::Z:
                    {m_delta_x(2,2)+=1;
                    std::array<double,6> howApplied = builder.getHowApplied()[BlockableState::Z];
                    for (std::size_t i=0; i<6; ++i) {m_T_x(i,2) = howApplied[i];}
                    break;}
                case BlockableState::U:
                    m_delta_v(0,0)+=1;
                    break;
                case BlockableState::V:
                    m_delta_v(1,1)+=1;
                    break;
                case BlockableState::W:
                    m_delta_v(2,2)+=1;
                    break;
                case BlockableState::P:
                    m_delta_v(3,3)+=1;
                    break;
                case BlockableState::Q:
                    m_delta_v(4,4)+=1;
                    break;
                case BlockableState::R:
                    m_delta_v(5,5)+=1;
                    break;
                case BlockableState::PHI:
                    {m_delta_x(3,3)+=1;
                    std::array<double,6> howApplied = builder.getHowApplied()[BlockableState::PHI];
                    for (std::size_t i=0; i<6; ++i) {m_T_x(i,3) = howApplied[i];}
                    break;}
                case BlockableState::THETA:
                    {m_delta_x(4,4)+=1;
                    std::array<double,6> howApplied = builder.getHowApplied()[BlockableState::THETA];
                    for (std::size_t i=0; i<6; ++i) {m_T_x(i,4) = howApplied[i];}
                    break;}
                case BlockableState::PSI:
                    {m_delta_x(5,5)+=1;
                    std::array<double,6> howApplied = builder.getHowApplied()[BlockableState::PSI];
                    for (std::size_t i=0; i<6; ++i) {m_T_x(i,5) = howApplied[i];}
                    break;}
                default:
                    break;
            }
        }
        // Are there mistakes in input data?
        if ((m_delta_x.maxCoeff() > 1) || (m_delta_v.maxCoeff() > 1))
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "Multiple definitions of at least one state in 'blocked dof' section of the YAML file.");
        }
        if (m_delta_x.sum()*m_delta_v.sum() != 0)
        {
            THROW(__PRETTY_FUNCTION__, InvalidInputException, "Positions and velocities cannot be forced at the same time. Check 'blocked dof' section of the YAML file.");
        }
     }

    std::map<BlockableState, Interpolator> blocked_dof;
    size_t body_idx;
    std::map<BlockableState, double> tmin;
    std::map<BlockableState, double> tmax;

    // Matrices of forced DoFs
    Eigen::Matrix<double,6,6> m_delta_x; // Positions
    Eigen::Matrix<double,6,6> m_delta_v; // Velocities
    // T operator
    Eigen::Matrix<double,6,6> m_T_x;

    bool state_is_blocked_at_that_date(const BlockableState& s, const double t) const
    {

        const auto min_ = tmin.find(s);
        const auto max_ = tmax.find(s);
        if ((min_ != tmin.end()) && (max_ != tmax.end()))
        {

            return (min_->second <= t) && (t <= max_->second);
        }
        return false;
    }

    bool state_is_blocked_at_that_date(const BlockableState& s, const double t, const size_t k) const
    {

        const auto min_ = tmin.find(s);
        const auto max_ = tmax.find(s);
        if ((min_ != tmin.end()) && (max_ != tmax.end()))
        {
            return (k == 0)? ((min_->second <= t) && (t <= max_->second)):((min_->second <= t) && (t < max_->second)); // if t = max_, consider blocked only if we are looking for the 0 order derivatives. Indeed, it is a point of discontinuity.
        }
        return false;
    }

    size_t state_index(const BlockableState& s)
    {
        switch (s)
        {
            case BlockableState::X:
                return XIDX(body_idx);
                break;
            case BlockableState::Y:
                return YIDX(body_idx);
                break;
            case BlockableState::Z:
                return ZIDX(body_idx);
                break;
            case BlockableState::U:
                return UIDX(body_idx);
                break;
            case BlockableState::V:
                return VIDX(body_idx);
                break;
            case BlockableState::W:
                return WIDX(body_idx);
                break;
            case BlockableState::P:
                return PIDX(body_idx);
                break;
            case BlockableState::Q:
                return QIDX(body_idx);
                break;
            case BlockableState::R:
                return RIDX(body_idx);
                break;
            case BlockableState::PHI:
                return QIIDX(body_idx);
                break;
            case BlockableState::THETA:
                return QJIDX(body_idx);
                break;
            case BlockableState::PSI:
                return QKIDX(body_idx);
                break;
            default:
                break;
        }
        return 0;
    }


    private:Impl();
};

BlockedDOF::BlockedDOF(const YamlBlockedDOF& input, const size_t body_idx) : pimpl(new Impl(Builder(input),body_idx))
{
}

BlockedDOF::BlockedDOF(const std::string& input, const size_t body_idx) : pimpl(new Impl(Builder(parse(input)),body_idx))
{
}

void BlockedDOF::force_states(StateType& x, const double t) const
{
    for (auto& dof: pimpl->blocked_dof)
    {
        if (pimpl->state_is_blocked_at_that_date(dof.first, t))
        {
           double forced_value = 0;
            try
            {
                forced_value = dof.second->f(t);
            }
            catch(const ssc::interpolation::IndexFinderException& e)
            {
                std::stringstream ss;
                ss << "Unable to interpolate value of forced state '" << dof.first << "' at t=" << t << " s: " << e.get_message();
                THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
            }
            x[pimpl->state_index(dof.first)] = forced_value;
        }
    }
}

void BlockedDOF::force_state_derivatives(StateType& dx_dt, const double t) const
{
    for (auto &dof:pimpl->blocked_dof)
        {
         // Check whether that DoF shall be forced at the current time
        if (pimpl->state_is_blocked_at_that_date(dof.first, t))
        {
            // Initialize forced value to 0
            double forced_value = 0;
            try
            {
                forced_value = dof.second->df(t); // Set forced value by interpolation in input data
            }
            catch(const ssc::interpolation::IndexFinderException& e)
            {
                std::stringstream ss;
                ss << "Unable to interpolate value of forced state derivative 'd" << dof.first << "/dt' at t=" << t << " s: " << e.get_message();
                THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
            }
            dx_dt[pimpl->state_index(dof.first)] = forced_value;
        }
    }
}

BlockedDOF::Vector BlockedDOF::get_delta_F(const StateType& dx_dt_, const Eigen::Matrix<double,6,6>& total_inertia, const ssc::kinematics::Wrench& sum_of_other_forces) const
{
    if (pimpl->blocked_dof.empty()) return Vector::Zero();
    BlockedDOF::Vector delta_F;
    BlockedDOF::Vector dx_dt;
    dx_dt(0) = dx_dt_.at(UIDX(pimpl->body_idx));
    dx_dt(1) = dx_dt_.at(VIDX(pimpl->body_idx));
    dx_dt(2) = dx_dt_.at(WIDX(pimpl->body_idx));
    dx_dt(3) = dx_dt_.at(PIDX(pimpl->body_idx));
    dx_dt(4) = dx_dt_.at(QIDX(pimpl->body_idx));
    dx_dt(5) = dx_dt_.at(RIDX(pimpl->body_idx));
    BlockedDOF::Vector F;
    F(0) = sum_of_other_forces.X();
    F(1) = sum_of_other_forces.Y();
    F(2) = sum_of_other_forces.Z();
    F(3) = sum_of_other_forces.K();
    F(4) = sum_of_other_forces.M();
    F(5) = sum_of_other_forces.N();
    delta_F = total_inertia*dx_dt - F;
    return delta_F;
}

void BlockedDOF::get_forced_x_k(const double t, Eigen::Matrix<double,6,1>& x, size_t k) const
{
    for (auto &dof:pimpl->blocked_dof)
        {
            // Check whether that DoF shall be forced at the current time
            if (pimpl->state_is_blocked_at_that_date(dof.first, t, k))
            {
                switch(dof.first)
                {
                    case BlockableState::X:
                        try
                        {
                            x(0) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::Y:
                        try
                        {
                            x(1) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::Z:
                        try
                        {
                            x(2) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::PHI:
                        try
                        {
                            x(3) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::THETA:
                        try
                        {
                            x(4) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::PSI:
                        try
                        {
                            x(5) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    default:
                        break;
                }
            }
        }
}

void BlockedDOF::get_forced_v_k(const double t, Eigen::Matrix<double,6,1>& v, size_t k) const
{
    for (auto &dof:pimpl->blocked_dof)
        {
            // Check whether that DoF shall be forced at the current time
            if (pimpl->state_is_blocked_at_that_date(dof.first, t, k))
            {
                switch(dof.first)
                {
                    case BlockableState::U:
                        try
                        {
                            v(0) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::V:
                        try
                        {
                            v(1) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::W:
                        try
                        {
                            v(2) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::P:
                        try
                        {
                            v(3) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::Q:
                        try
                        {
                            v(4) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    case BlockableState::R:
                        try
                        {
                            v(5) = (k == 0)? (dof.second->f(t)):(dof.second->df(t,k)); // Set forced value by interpolation in input data
                        }
                        catch(const ssc::interpolation::IndexFinderException& e)
                        {
                            std::stringstream ss;
                            ss << "Unable to interpolate value of forced state derivative " << dof.first << " at t=" << t << " s: " << e.get_message();
                            THROW(__PRETTY_FUNCTION__, NumericalErrorException, ss.str());
                        }
                        break;
                    default:
                        break;
                }
            }
        }
}

Eigen::Matrix<double,6,6> BlockedDOF::get_delta_x(const double & t) const
{
    Eigen::Matrix<double,6,6> delta_x = Eigen::Matrix<double,6,6>::Zero();
    for (const auto &dof:pimpl->blocked_dof)
    {
        // Check whether that DoF shall be forced at the current time
        if (pimpl->state_is_blocked_at_that_date(dof.first, t))
        {
            switch(dof.first)
            {
                case BlockableState::X:
                    {delta_x(0,0)+=1;
                    break;}
                case BlockableState::Y:
                    {delta_x(1,1)+=1;
                    break;}
                case BlockableState::Z:
                    {delta_x(2,2)+=1;
                    break;}
                case BlockableState::PHI:
                    {delta_x(3,3)+=1;
                    break;}
                case BlockableState::THETA:
                    {delta_x(4,4)+=1;
                    break;}
                case BlockableState::PSI:
                    {delta_x(5,5)+=1;
                    break;}
                default:
                    break;
            }
        }
    }
    return delta_x;
}

Eigen::Matrix<double,6,6> BlockedDOF::get_delta_v(const double & t) const
{
    Eigen::Matrix<double,6,6> delta_v = Eigen::Matrix<double,6,6>::Zero();
    for (const auto &dof:pimpl->blocked_dof)
    {
        // Check whether that DoF shall be forced at the current time
        if (pimpl->state_is_blocked_at_that_date(dof.first, t))
        {
            switch(dof.first)
            {
                case BlockableState::U:
                    delta_v(0,0)+=1;
                    break;
                case BlockableState::V:
                    delta_v(1,1)+=1;
                    break;
                case BlockableState::W:
                    delta_v(2,2)+=1;
                    break;
                case BlockableState::P:
                    delta_v(3,3)+=1;
                    break;
                case BlockableState::Q:
                    delta_v(4,4)+=1;
                    break;
                case BlockableState::R:
                    delta_v(5,5)+=1;
                    break;
                default:
                    break;
            }
        }
    }
    return delta_v;
}

Eigen::Matrix<double,6,6> BlockedDOF::get_T_x() const
{
    return pimpl->m_T_x;
}


