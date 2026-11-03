/*
 * transform_double_equal.hpp
 *
 * \date Apr 28, 2014
 * \author cec
 */

#ifndef TRANSFORM_DOUBLE_EQUAL_HPP_
#define TRANSFORM_DOUBLE_EQUAL_HPP_

namespace ssc
{
    namespace kinematics
    {
        class Transform;
    }
}
bool double_equal(const ssc::kinematics::Transform& t1, const ssc::kinematics::Transform& t2, const double eps=0);

#endif  /* TRANSFORM_DOUBLE_EQUAL_HPP_ */
