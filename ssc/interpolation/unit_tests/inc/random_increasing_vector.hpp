/*
 * random_increasing_vector.hpp
 *
 * \date 11 avr. 2013, 08:54:45
 * \author cec
 */

#ifndef RANDOM_INCREASING_VECTOR_HPP_
#define RANDOM_INCREASING_VECTOR_HPP_

#include <cstddef>
#include <vector>

namespace ssc
{
    namespace random_data_generator
    {
        class DataGenerator;
    }
}

std::vector<double> a_random_vector_of_doubles_in_increasing_order_of_size(const ssc::random_data_generator::DataGenerator& a, const size_t& n);


#endif /* RANDOM_INCREASING_VECTOR_HPP_ */
