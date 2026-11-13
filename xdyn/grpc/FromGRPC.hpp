/*
 * FromGRPC.hpp
 *
 *  Created on: Jun 26, 2019
 *      Author: cady
 */

#ifndef GRPC_INC_FROMGRPC_HPP_
#define GRPC_INC_FROMGRPC_HPP_

#include <grpcpp/grpcpp.h>

#include <ssc/kinematics.hpp>

#include "GRPCTypes.hpp"
#include "force.grpc.pb.h"
#include "force.pb.h"

class FromGRPC {
public:
    WaveRequest to_wave_request(const RequiredWaveInformationResponse& response) const;
    ssc::kinematics::Vector6d to_force(const ForceResponse& response) const;
};

#endif /* GRPC_INC_FROMGRPC_HPP_ */
