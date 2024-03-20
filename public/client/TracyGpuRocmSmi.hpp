#ifndef __TRACYGPUROCMSMI_HPP__
#define __TRACYGPUROCMSMI_HPP__

// TODO: detect rocm smi availability
#if defined __linux__
#  define TRACY_HAS_GPU_ROCM_SMI
#endif

#ifdef TRACY_HAS_GPU_ROCM_SMI

#include <stdint.h>
#include <stdio.h>

#include "TracyFastVector.hpp"

namespace tracy
{

class GpuRocmSmi
{
    struct Domain
    {
        uint64_t value;
        uint64_t overflow;
        FILE* handle;
        const char* name;
    };

public:
    GpuRocmSmi();
    ~GpuRocmSmi();

    void Tick();

private:
    void ScanDirectory( const char* path, int parent );

    FastVector<Domain> m_domains;
    uint64_t m_lastTime;
};

}
#endif

#endif
