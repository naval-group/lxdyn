#ifndef XDYN_WRAPPER_FMI_XDYN_FMU_H
#define XDYN_WRAPPER_FMI_XDYN_FMU_H

#include "fmi2Functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Not FMI: what make_fmu.py calls to write an FMU's modelDescription.xml from the model
    // itself. Returns nullptr if xdyn rejects the model, xdyn_fmu_error() then saying why. The
    // text stays valid until the next call.
    FMI2_Export const char* xdyn_fmu_model_description(const char* yaml_path,
                                                       const char* assets_root,
                                                       const char* model_name, double dt,
                                                       const char* solver);

    FMI2_Export const char* xdyn_fmu_error(void);

#ifdef __cplusplus
}
#endif

#endif
