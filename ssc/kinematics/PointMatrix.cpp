#include "ssc/kinematics/PointMatrix.hpp"
#include "ssc/kinematics/Point.hpp"

using namespace ssc::kinematics;

PointMatrix::PointMatrix() :  frame("")
{
}

PointMatrix::PointMatrix(const std::string& frame_, const size_t nb_of_columns) :  frame(frame_)
{
    m.resize(3,(int)nb_of_columns);
}

PointMatrix::PointMatrix(const Matrix3Xd& m_, const std::string& frame_) : m(m_), frame(frame_)
{
}

void PointMatrix::swap(PointMatrix& other) noexcept
{
    m.swap(other.m);
    frame.swap(other.frame);
}

std::string PointMatrix::get_frame() const
{
    return frame;
}

PointMatrix PointMatrix::operator+(const Point& P) const
{
    const Matrix3Xd M = m.colwise() + P.v;
    return PointMatrix(M, frame);
}

PointMatrix ssc::kinematics::operator+(const Point& P, const PointMatrix& M)
{
    return M+P;
}

void ssc::kinematics::swap(PointMatrix& one, PointMatrix& the_other) noexcept
{
    one.swap(the_other);
}
