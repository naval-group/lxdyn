/*
 * PhysicalQuantity.hpp
 *
 * \date 13 mars 2013, 16:09:27
 * \author cec
 */

#ifndef PHYSICALQUANTITY_HPP_
#define PHYSICALQUANTITY_HPP_

#include <list>
#include <vector>

/** \author cec
 *  \brief This class was created to store the international standard's value of a physical quantity.
 *  \details It has the following responsibility:
 *  - Store & retrieve the USI value of a quantity (eg. seconds for Time, meters for Length, etc.)
 *  \section ex1 Example
 *  \snippet physical_types/unit_tests/src/LengthTest.cpp LengthTest example
 *  \section ex2 Expected output
 *  \snippet physical_types/unit_tests/src/LengthTest.cpp LengthTest expected output
 */

namespace ssc
{
    namespace data_source
    {
        class PhysicalQuantity
        {
            public:
                PhysicalQuantity();
                virtual ~PhysicalQuantity();
                void coerce(std::list<double>& ret) const;
                void decoerce(std::list<double>& ret);

            protected:
                PhysicalQuantity(const std::vector<double>& v);
                std::vector<double> val;
        };
    }
}



#endif /* PHYSICALQUANTITY_HPP_ */
