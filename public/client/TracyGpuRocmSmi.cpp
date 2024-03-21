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

    rsmi_status_t ret;
    uint32_t num_devices;
    uint16_t dev_id;

    ret = rsmi_init(0);
    ret = rsmi_num_monitor_devices(&num_devices);

    for (int i=0; i < num_devices; ++i) {
        ret = rsmi_dev_id_get(i, &dev_id);
        fprintf(stdout, "got device id: %d\n", (int)dev_id);

        constexpr size_t MAX_NAME_LEN = 2048;
        char name[MAX_NAME_LEN];
        ret = rsmi_dev_name_get(i, name, MAX_NAME_LEN);
        name[MAX_NAME_LEN - 1] = '\0';
        const char *status_string;
        if (ret != RSMI_STATUS_SUCCESS) {
            ret = rsmi_status_string(ret, &status_string);
            fprintf(stderr, "Failed to get name of the GPU: %s", status_string);
        }
        fprintf(stdout, "got device name: '%s'\n", name);
    }
    ret = rsmi_shut_down();
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
