#include "TracyGpuRocmSmi.hpp"

#ifdef TRACY_HAS_GPU_ROCM_SMI

#include <sys/types.h>
#include <dirent.h>
#include <chrono>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "TracyDebug.hpp"
#include "TracyProfiler.hpp"
#include "../common/TracyAlloc.hpp"

namespace tracy
{

GpuRocmSmi::GpuRocmSmi()
    : m_metrics( 1 )
    , m_lastTime( 0 )
{
    // TODO: initialize

    m_metrics[0].name = "ROCm test";
}

GpuRocmSmi::~GpuRocmSmi()
{
    for( auto& v : m_metrics )
    {
        // Do not release v.name, as it may be still needed
    }
}

void GpuRocmSmi::Tick()
{
    auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if( t - m_lastTime > 10000000 )    // 10 ms
    {
        m_lastTime = t;

        TracyLfqPrepare( QueueType::SysPowerReport );
        MemWrite( &item->sysPower.time, Profiler::GetTime() );
        MemWrite( &item->sysPower.delta, /*delta=*/10 );
        MemWrite( &item->sysPower.name, (uint64_t)m_metrics[0].name );
        TracyLfqCommit;
    }
}

}

#endif
