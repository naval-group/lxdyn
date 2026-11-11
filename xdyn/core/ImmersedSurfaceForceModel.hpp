/*
 * ImmersedSurfaceForceModel.hpp
 *
 *  Created on: Oct 2, 2014
 *      Author: cady
 */

#ifndef IMMERSEDSURFACEFORCEMODEL_HPP_
#define IMMERSEDSURFACEFORCEMODEL_HPP_

#include "SurfaceForceModel.hpp"

/** \brief This class implemented SurfaceForceModel's begin & end methods
 */
class ImmersedSurfaceForceModel : public SurfaceForceModel
{
    public:
        ImmersedSurfaceForceModel(const std::string& name, const std::string& body_name, const EnvironmentAndFrames& env);
        ~ImmersedSurfaceForceModel() override;

    private:
        ImmersedSurfaceForceModel();
        FacetIterator begin(const MeshIntersectorPtr& intersector) const override;
        FacetIterator end(const MeshIntersectorPtr& intersector) const override;
};

#endif /* IMMERSEDSURFACEFORCEMODEL_HPP_ */
