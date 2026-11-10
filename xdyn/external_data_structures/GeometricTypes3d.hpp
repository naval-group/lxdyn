#ifndef GEOMETRICTYPES3D_HPP_
#define GEOMETRICTYPES3D_HPP_

#include <array>
#include <vector>

#include <Eigen/Dense>

using EPoint = Eigen::Vector3d;
using VectorOfPoints = std::vector<EPoint>;
using VectorOfVectorOfPoints = std::vector<VectorOfPoints>;
using Matrix3x = Eigen::Matrix<double,3,Eigen::Dynamic>;

using ArrayOfEdges = std::array<std::vector<size_t>,2>;

/**
 * \details Holds the indices of the vertices in the mesh rather than duplicating the coordinates
 * \ingroup mesh
 */
struct Facet
{
    Facet():vertex_index(),unit_normal(Eigen::MatrixXd::Zero(3,1)),centre_of_gravity(Eigen::MatrixXd::Zero(3,1)),area(0) {}
    Facet(const std::vector<size_t> &vertex_index_,const Eigen::Vector3d &unit_normal_,const Eigen::Vector3d &barycenter_,double area_)
    :vertex_index(vertex_index_)
    ,unit_normal(unit_normal_)
    ,centre_of_gravity(barycenter_)
    ,area(area_) {}
    std::vector<size_t> vertex_index;
    Eigen::Vector3d unit_normal;
    Eigen::Vector3d centre_of_gravity;
    double area;
};

using VectorOfFacet = std::vector<Facet>;

#endif
