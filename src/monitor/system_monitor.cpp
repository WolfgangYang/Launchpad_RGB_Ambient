#include "system_monitor.h"

namespace lra {

SystemMonitor::~SystemMonitor()
{
    shutdown();
}

bool SystemMonitor::initialize()
{
    if (PdhOpenQueryW(nullptr, 0, &cpuQuery_) != ERROR_SUCCESS) {
        cpuQuery_ = nullptr;
        return false;
    }

    if (PdhAddEnglishCounterW(
            cpuQuery_,
            L"\\Processor(_Total)\\% Processor Time",
            0,
            &cpuCounter_) != ERROR_SUCCESS) {
        shutdown();
        return false;
    }

    PdhCollectQueryData(cpuQuery_);
    return true;
}

void SystemMonitor::shutdown()
{
    if (cpuQuery_) {
        PdhCloseQuery(cpuQuery_);
        cpuQuery_ = nullptr;
        cpuCounter_ = nullptr;
    }
}

void SystemMonitor::update(MonitoringState& state)
{
    if (cpuQuery_ && cpuCounter_) {
        PDH_FMT_COUNTERVALUE value{};
        if (PdhCollectQueryData(cpuQuery_) == ERROR_SUCCESS &&
            PdhGetFormattedCounterValue(
                cpuCounter_, PDH_FMT_DOUBLE, nullptr, &value) == ERROR_SUCCESS) {
            state.cpuUsage = value.doubleValue;
        }
    }

    MEMORYSTATUSEX memory{};
    memory.dwLength = sizeof(memory);
    if (GlobalMemoryStatusEx(&memory)) {
        state.ramUsage =
            100.0 * (1.0 -
                static_cast<double>(memory.ullAvailPhys) /
                static_cast<double>(memory.ullTotalPhys));
    }

    // GPU/temperature are deliberately not faked. They remain 0 until
    // a real hardware data source is added.
}

} // namespace lra
