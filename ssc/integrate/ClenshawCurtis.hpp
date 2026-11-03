/*
 * ClenshawCurtis.hpp
 *
 * \date Dec 9, 2014
 * \author cec
 */

#ifndef CLENSHAWCURTIS_HPP_
#define CLENSHAWCURTIS_HPP_

#include "ssc/integrate/QuadPack.hpp"

namespace ssc
{
    namespace integrate
    {
        class ClenshawCurtis : public QuadPack
        {
            public:
                ClenshawCurtis(const double tau);
                ClenshawCurtis(const Function& f, const double tau);
                ~ClenshawCurtis();
                ClenshawCurtis(const ClenshawCurtis& rhs);
                ClenshawCurtis& operator=(const ClenshawCurtis& rhs);
                double compute_for(const double tau, const double a, const double b, const double eps);

            private:
                ClenshawCurtis();
                double integrate_impl(const Function& f, double a, double b, double eps=1e-6);
                virtual int get_internal_type_number() const = 0;
                double tau;
                int nb_of_chebychev_moments;
                int nb_of_calls;
                double *chebychev_moments;
                double *alist;
                double *blist;
                double *rlist;
                double *elist;
                int *iord;
                int *nnlog;
        };
    }
}

#endif /* CLENSHAWCURTIS_HPP_ */
