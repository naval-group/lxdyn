/*
 * ClenshawCurtisCosine.hpp
 *
 * \date Dec 9, 2014
 * \author cec
 */

#ifndef CLENSHAWCURTISCOSINE_HPP_
#define CLENSHAWCURTISCOSINE_HPP_

#include "ssc/integrate/ClenshawCurtis.hpp"

namespace ssc
{
    namespace integrate
    {
        class ClenshawCurtisCosine : public ClenshawCurtis
        {
            public:
                ClenshawCurtisCosine(const double tau);
                ClenshawCurtisCosine(const Function& f, const double tau);

            private:
                ClenshawCurtisCosine();
                int get_internal_type_number() const;
        };
    }
}


#endif /* CLENSHAWCURTISCOSINE_HPP_ */
