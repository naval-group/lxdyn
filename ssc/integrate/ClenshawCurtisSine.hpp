/*
 * ClenshawCurtisSine.hpp
 *
 * \date Dec 9, 2014
 * \author cec
 */

#ifndef CLENSHAWCURTISSINE_HPP_
#define CLENSHAWCURTISSINE_HPP_

#include "ssc/integrate/ClenshawCurtis.hpp"

namespace ssc
{
    namespace integrate
    {
        class ClenshawCurtisSine : public ClenshawCurtis
        {
            public:
                ClenshawCurtisSine(const double tau);
                ClenshawCurtisSine(const Function& f, const double tau);

            private:
                ClenshawCurtisSine();
                int get_internal_type_number() const;
        };
    }
}

#endif /* CLENSHAWCURTISSINE_HPP_ */
