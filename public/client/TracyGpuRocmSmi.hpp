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

#include "../common/TracyQueue.hpp"

namespace tracy
{

class GpuRocmSmi
{
    // TODO: generalize to "GpuDevice", track various values
    // struct Metric
    // {
    //     // uint64_t value;
    //     const char* name;
    // };

public:
    GpuRocmSmi();
    ~GpuRocmSmi();

    void Tick();

private:
    void SetupPlot(const char* name, PlotFormatType formatType);
    void PlotInt(const char* name, int64_t time, uint64_t value);

    bool m_initialized;
    uint64_t m_lastTime;

    // FastVector<Metric> m_metrics;
    const char* m_nameSocketPower = "GPU socket power (Watts)";
    const char* m_nameDeviceMemoryUsage = "GPU memory usage (VRAM)";
    const char* m_nameDeviceBusyPercent = "GPU device busy percentage";
    const char* m_nameUtilizationGFX = "GPU utilization (GFX)";
    const char* m_nameUtilizationMEM = "GPU utilization (MEM)";
};

}
#endif

#endif
