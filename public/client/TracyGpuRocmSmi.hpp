#ifndef __TRACYGPUROCMSMI_HPP__
#define __TRACYGPUROCMSMI_HPP__

// TODO: detect rocm smi availability
#if defined __linux__
#  define TRACY_HAS_GPU_ROCM_SMI
#endif

#ifdef TRACY_HAS_GPU_ROCM_SMI

#include <stdint.h>
#include <stdio.h>
#include <rocm_smi/rocm_smi.h>

#include "TracyFastVector.hpp"

namespace tracy
{

class GpuRocmSmi
{
    // TODO: generalize to "GpuDevice", track various values
    struct Metric
    {
        // uint64_t value;
        const char* name;
    };

public:
    GpuRocmSmi();
    ~GpuRocmSmi();

    void Tick();

private:
    bool m_initialized;
    FastVector<Metric> m_metrics;
    uint64_t m_lastTime;
};

}
#endif

#endif
