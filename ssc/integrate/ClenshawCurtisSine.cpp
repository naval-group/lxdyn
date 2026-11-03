/*
 * ClenshawCurtisSine.cpp
 *
 * \date Dec 9, 2014
 * \author cec
 */

#include "ssc/integrate/ClenshawCurtisSine.hpp"

ssc::integrate::ClenshawCurtisSine::ClenshawCurtisSine(const double tau) : ClenshawCurtis(tau)
{
}

ssc::integrate::ClenshawCurtisSine::ClenshawCurtisSine(const Function& f, const double tau) : ClenshawCurtis(f,tau)
{
}

int ssc::integrate::ClenshawCurtisSine::get_internal_type_number() const
{
    return 2;
}

