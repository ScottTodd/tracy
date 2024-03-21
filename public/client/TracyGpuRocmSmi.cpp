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
    : m_metrics( 1 )
    , m_lastTime( 0 )
{
    m_metrics[0].name = "GPU socket power";
    // m_metrics[0].value = 0;

    rsmi_status_t ret;
    uint32_t numDevices;
    uint16_t devId;

    ret = rsmi_init(0);
    ret = rsmi_num_monitor_devices(&numDevices);

    for (int i=0; i < numDevices; ++i) {
        ret = rsmi_dev_id_get(i, &devId);
        check_rsmi_status(ret);
        fprintf(stdout, "Got ROCm device id: %d\n", (int)devId);

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

        uint64_t socketPower;
        // rsmi_status_t ret;
        ret = rsmi_dev_current_socket_power_get(0, &socketPower);
        check_rsmi_status(ret);
        fprintf(stdout, "socket power: %d\n", (int)socketPower);
    }
    // ret = rsmi_shut_down();

    //m_metrics[0].name
    TracyLfqPrepare( QueueType::PlotConfig );
    MemWrite( &item->plotConfig.name, (uint64_t)name );
    MemWrite( &item->plotConfig.type, (uint8_t)PlotFormatType::Number );
    MemWrite( &item->plotConfig.step, (uint8_t)false );
    MemWrite( &item->plotConfig.fill, (uint8_t)true );
    MemWrite( &item->plotConfig.color, 0 );
    TracyLfqCommit;
}

GpuRocmSmi::~GpuRocmSmi()
{
    // rsmi_status_t ret;

    for( auto& v : m_metrics )
    {
        // Do not release v.name, as it may be still needed
    }

    fprintf(stdout, "destructor\n");

    // Can't do much if shut down fails here.
    (void)rsmi_shut_down();
}

void GpuRocmSmi::Tick()
{
    auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if( t - m_lastTime > 10000000 )    // 10 ms
    {
        m_lastTime = t;

        // fprintf(stdout, "Tick()\n");

        // Tracy Syspower
        //   ev.delta is Microjoule
        //   power is Watt = J / s


        // Units: microwatts
        uint64_t socketPowerMicrowatts;
        rsmi_status_t ret;
        ret = rsmi_dev_current_socket_power_get(0, &socketPowerMicrowatts);
        check_rsmi_status(ret);
        uint64_t socketPowerWatts = socketPowerMicrowatts / 1000000;
        // fprintf(stdout, "  socket power: %d\n", (int)socket_power);
        // uint64_t delta;
        // delta = socketPowerWatts - m_metrics[0].value;
        // m_metrics[0].value = socketPowerWatts;
        // fprintf(stdout, "socket power: %dW, delta: %dW\n", (int)socketPowerWatts, (int)delta);
        fprintf(stdout, "socket power: %dW\n", (int)socketPowerWatts);

        // uint64_t socketEnergyMicrojoules;
        // float counterResolution;
        // uint64_t timestamp;
        // ret = rsmi_dev_energy_count_get(0, &socketEnergyMicrojoules, &counterResolution, &timestamp);
        // check_rsmi_status(ret);
        // uint64_t delta;
        // delta = socketEnergyMicrojoules - m_metrics[0].value;
        // m_metrics[0].value = socketEnergyMicrojoules;
        // fprintf(stdout, "socket energy: %" PRIu64 "uJ, delta: %" PRIu64 "uJ, resolution: %f, timestamp: %" PRIu64 "\n", socketEnergyMicrojoules, delta, counterResolution, timestamp);

        // TODO: rsmi_dev_power_cap_range_get()

        // GPU:  28061126986666
        // CPU:  21365180400
        // TracyLfqPrepare( QueueType::SysPowerReport );
        // MemWrite( &item->sysPower.time, timestamp );
        // MemWrite( &item->sysPower.delta, delta );
        // MemWrite( &item->sysPower.name, (uint64_t)m_metrics[0].name );

        TracyLfqPrepare( QueueType::PlotDataInt );
        MemWrite( &item->plotDataInt.name, (uint64_t)m_metrics[0].name );
        MemWrite( &item->plotDataInt.time, Profiler::GetTime() );
        MemWrite( &item->plotDataInt.val, socketPowerWatts );
        TracyLfqCommit;
    }
}

}

#endif
