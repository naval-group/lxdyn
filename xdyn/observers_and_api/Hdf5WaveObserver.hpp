#ifndef HDF5WAVEOBSERVER_HPP_
#define HDF5WAVEOBSERVER_HPP_

#include <string>
#include <ssc/macros.hpp>
#include <memory>
#include "H5Cpp.h"
#include "xdyn/core/SurfaceElevationGrid.hpp"

class Hdf5WaveObserver
{
    public:
        Hdf5WaveObserver(
                const H5::H5File& h5File,
                const std::string& datasetName = "WaveElevation",
                const std::size_t nx = 0,
                const std::size_t ny = 0);

        Hdf5WaveObserver(
                const std::string& fileName,
                const std::string& datasetName = "WaveElevation",
                const std::size_t nx = 0,
                const std::size_t ny = 0);

        Hdf5WaveObserver& operator<<(const SurfaceElevationGrid& waveElevationGrid);

    private:
        Hdf5WaveObserver(); // Disabled
        class Impl;
        std::shared_ptr<Impl> pimpl;
};

using Hdf5WaveObserverPtr = std::shared_ptr<Hdf5WaveObserver>;

#endif
