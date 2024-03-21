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

void check_rsmi_status(rsmi_status_t status) {
    const char *status_string;
    if (status != RSMI_STATUS_SUCCESS) {
        (void)rsmi_status_string(status, &status_string);
        fprintf(stderr, "RSMI failure: '%s'\n", status_string);
    }
}

GpuRocmSmi::GpuRocmSmi()
    : m_initialized ( false )
    // , m_metrics( 1 )
    , m_lastTime( 0 )
{
    // m_metrics[0].name = "GPU socket power (Watts)";
    // m_metrics[0].value = 0;

    rsmi_status_t ret;
    uint32_t numDevices;
    uint16_t devId;

    ret = rsmi_init(0);
    ret = rsmi_num_monitor_devices(&numDevices);

    for (int i=0; i < numDevices; ++i) {
        ret = rsmi_dev_id_get(i, &devId);
        check_rsmi_status(ret);
        fprintf(stdout, "Found ROCm device id: %d\n", (int)devId);

        // constexpr size_t MAX_NAME_LEN = 128;
        // char name[MAX_NAME_LEN];
        // ret = rsmi_dev_name_get(i, name, MAX_NAME_LEN);
        // name[MAX_NAME_LEN - 1] = '\0';
        // const char *status_string;
        // if (ret != RSMI_STATUS_SUCCESS) {
        //     ret = rsmi_status_string(ret, &status_string);
        //     fprintf(stderr, "Failed to get name of the GPU: %s", status_string);
        // }
        // fprintf(stdout, "Got ROCm device name: '%s'\n", name);

        // uint64_t socketPower;
        // // rsmi_status_t ret;
        // ret = rsmi_dev_current_socket_power_get(0, &socketPower);
        // check_rsmi_status(ret);
        // fprintf(stdout, "socket power: %d\n", (int)socketPower);
    }

}

GpuRocmSmi::~GpuRocmSmi()
{
    // for( auto& v : m_metrics )
    // {
    //     // Do not release v.name, as it may be still needed
    // }

    fprintf(stdout, "destructor\n");

    // Can't do much if shut down fails here.
    (void)rsmi_shut_down();
}

void GpuRocmSmi::Tick()
{
    if (!m_initialized) {
        // GPU socket power plot.
        TracyLfqPrepare( QueueType::PlotConfig );
        MemWrite( &item->plotConfig.name, (uint64_t)m_nameSocketPower );
        MemWrite( &item->plotConfig.type, (uint8_t)PlotFormatType::Number );
        MemWrite( &item->plotConfig.step, (uint8_t)false );
        MemWrite( &item->plotConfig.fill, (uint8_t)true );
        MemWrite( &item->plotConfig.color, 0 );
        TracyLfqCommit;

        // GPU utilization (GFX) plot.
        TracyLfqPrepare( QueueType::PlotConfig );
        MemWrite( &item->plotConfig.name, (uint64_t)m_nameUtilizationGFX );
        MemWrite( &item->plotConfig.type, (uint8_t)PlotFormatType::Number );
        MemWrite( &item->plotConfig.step, (uint8_t)false );
        MemWrite( &item->plotConfig.fill, (uint8_t)true );
        MemWrite( &item->plotConfig.color, 0 );
        TracyLfqCommit;

        // GPU utilization (MEM) plot.
        TracyLfqPrepare( QueueType::PlotConfig );
        MemWrite( &item->plotConfig.name, (uint64_t)m_nameUtilizationMEM );
        MemWrite( &item->plotConfig.type, (uint8_t)PlotFormatType::Number );
        MemWrite( &item->plotConfig.step, (uint8_t)false );
        MemWrite( &item->plotConfig.fill, (uint8_t)true );
        MemWrite( &item->plotConfig.color, 0 );
        TracyLfqCommit;

        m_initialized = true;
    }

    auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if( t - m_lastTime > 10000000 )    // 10 ms
    {
        m_lastTime = t;

        rsmi_status_t ret;

        // We could use the GPU timestamps (which could be backdated), if
        // time domains are sufficiently overlapping.
        int64_t profilerTime = Profiler::GetTime();

        // GPU socket power plot.
        uint64_t socketPowerMicrowatts;
        ret = rsmi_dev_current_socket_power_get(0, &socketPowerMicrowatts);
        check_rsmi_status(ret);
        uint64_t socketPowerWatts = socketPowerMicrowatts / 1000000;
        TracyLfqPrepare( QueueType::PlotDataInt );
        MemWrite( &item->plotDataInt.name, (uint64_t)(uint64_t)m_nameSocketPower );
        MemWrite( &item->plotDataInt.time, profilerTime );
        MemWrite( &item->plotDataInt.val, socketPowerWatts );
        TracyLfqCommit;

        // GPU utilization plots.
        uint64_t timestamp;
        rsmi_utilization_counter_t utilizationCounters[2];
        utilizationCounters[0].type = RSMI_COARSE_GRAIN_GFX_ACTIVITY;
        utilizationCounters[1].type = RSMI_COARSE_GRAIN_MEM_ACTIVITY;
        ret = rsmi_utilization_count_get(0, utilizationCounters, 2, &timestamp);
        check_rsmi_status(ret);
        TracyLfqPrepare( QueueType::PlotDataInt );
        MemWrite( &item->plotDataInt.name, (uint64_t)(uint64_t)m_nameUtilizationGFX );
        MemWrite( &item->plotDataInt.time, profilerTime );
        MemWrite( &item->plotDataInt.val, utilizationCounters[0].value );
        TracyLfqCommit;
        TracyLfqPrepare( QueueType::PlotDataInt );
        MemWrite( &item->plotDataInt.name, (uint64_t)(uint64_t)m_nameUtilizationMEM );
        MemWrite( &item->plotDataInt.time, profilerTime );
        MemWrite( &item->plotDataInt.val, utilizationCounters[1].value );
        TracyLfqCommit;
    }
}

}

#endif
