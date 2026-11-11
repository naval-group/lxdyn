#ifndef POINTMATRIX_HPP_
#define POINTMATRIX_HPP_

#include <memory>
#include <string>
#include <Eigen/Dense>

namespace ssc
{
    namespace kinematics
    {
        using Matrix3Xd = Eigen::Matrix<double,3,Eigen::Dynamic>;

        class Point;
        class PointMatrix
        {
            public:
                PointMatrix();
                PointMatrix(const std::string& frame, const size_t nb_of_columns);
                PointMatrix(const Matrix3Xd& m,const std::string& frame);
                PointMatrix(const PointMatrix& other);
                PointMatrix(PointMatrix&& other) noexcept;
                PointMatrix& operator=(PointMatrix other);
                Matrix3Xd m;
                std::string get_frame() const;
                PointMatrix operator+(const Point& P) const;
                void swap(PointMatrix& other) noexcept;

            private:
                std::string frame;
        };

        PointMatrix operator+(const Point& P, const PointMatrix& M);

        void swap(PointMatrix& one, PointMatrix& the_other) noexcept;
        using PointMatrixPtr = std::shared_ptr<PointMatrix>;
    }
}


#endif
