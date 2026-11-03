/*
 * Observers.hpp
 *
 * \date 21 mars 2014
 *      Author: cec
 */

#ifndef OBSERVERS_HPP_
#define OBSERVERS_HPP_

#include <memory>
#include <sstream>
#include <vector>

#include "ssc/solver/ContinuousSystem.hpp"
#include "ssc/solver/DiscreteSystem.hpp"

namespace ssc
{
    namespace solver
    {
        template <typename IteratorType, typename Stream> void serialize(Stream& os, const IteratorType& begin, const IteratorType& end)
        {
            if (end-begin>0)
            {
                IteratorType it = begin;
                os << *it;++it;
                for (;it!=end;++it)
                {
                    os << "," << *it;
                }
                os << "\n";
            }
        }

        class NullObserver
        {
            public:
                NullObserver() {}
                void check_variables_to_serialize_are_available() const {}
                void observe_before_solver_step(const ContinuousSystem& , const double, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& )
                {
                }
                void observe_after_solver_step(const ContinuousSystem& , const double, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& )
                {

                }
                void collect_available_serializations(const ssc::solver::ContinuousSystem &, const double, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >&)
                {
                }
                void flush()
                {}
        };

        class DefaultObserver
        {
            public:
                DefaultObserver(std::ostream& os_) : os(os_), initialized(false) {}
                void check_variables_to_serialize_are_available() const {}
                void observe(const ContinuousSystem& sys, const double t, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >&)
                {
                    const std::vector<double> x = sys.state;
                    if (not(initialized))
                    {
                        serialize_title(x.size());
                        initialized = true;
                    }
                    std::vector<double> v;
                    v.push_back(t);
                    v.insert(v.end(), x.begin(), x.end());
                    serialize(os, v.begin(), v.end());
                }
                void observe_before_solver_step(const ContinuousSystem& , const double , const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& )
                {
                    // Don't do anything: we don't care about observing forces in these tests ; only states matter.
                }
                void observe_after_solver_step(const ContinuousSystem& sys, const double t, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& d)
                {
                    observe(sys, t, d);
                }
                void flush()
                {}

            private:

                void serialize_title(const size_t nb_of_states)
                {
                    std::vector<std::string> v;
                    v.push_back("t");
                    for (size_t i = 0 ; i < nb_of_states ; ++i)
                    {
                        std::stringstream ss;
                        ss << "x" << i;
                        v.push_back(ss.str());
                    }
                    serialize(os, v.begin(), v.end());
                }

                std::ostream& os;
                bool initialized;
        };

        class VectorObserver
        {
            public:
                VectorObserver() : v(std::vector<std::pair<double,double> >()) {}
                void check_variables_to_serialize_are_available() const {}
                void observe(const ContinuousSystem& sys, const double t, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >&)
                {
                    v.push_back(std::make_pair(t, sys.state[0]));
                }
                void observe_before_solver_step(const ContinuousSystem& , const double , const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& )
                {
                    // Don't do anything: we don't care about observing forces in these tests ; only states matter.
                }
                void observe_after_solver_step(const ContinuousSystem& sys, const double t, const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& d)
                {
                    observe(sys, t, d);
                }

                std::vector<std::pair<double,double> > get() const
                {
                    return v;
                }

                void flush()
                {}

                void collect_available_serializations(const ssc::solver::ContinuousSystem &, const double , const std::vector<std::shared_ptr<ssc::solver::DiscreteSystem> >& )
                {
                }

            private:
                std::vector<std::pair<double,double> > v;
        };
    }
}

#endif /* OBSERVERS_HPP_ */
