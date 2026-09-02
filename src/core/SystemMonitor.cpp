#include "core/SystemMonitor.h"
#include <windows.h>
#include <pdhmsg.h>

namespace Core {

SystemMonitor::SystemMonitor() : cpuQuery(nullptr), currentStats{0.0f, 0.0f} {}

SystemMonitor::~SystemMonitor() {
    if (cpuQuery) {
        PdhCloseQuery(cpuQuery);
    }
}

bool SystemMonitor::Initialize() {
    // Initialize PDH Query for CPU usage
    if (PdhOpenQuery(nullptr, nullptr, &cpuQuery) != ERROR_SUCCESS) {
        return false;
    }

    // Path for total CPU usage: \Processor(_Total)\% Processor Time
    PDH_STATUS status = PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0, &cpuCounter);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    PdhCollectQueryData(cpuQuery);
    return true;
}

void SystemMonitor::Update() {
    // Update CPU Counter
    PDH_FMT_COUNTERVALUE counterVal;
    if (PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, nullptr, &counterVal) == ERROR_SUCCESS) {
        currentStats.cpuUsage = static_cast<float>(counterVal.doubleValue);
    }

    // Update RAM Usage via GlobalMemoryStatusEx
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        currentStats.ramUsagePercent = static_cast<float>(memInfo.dwMemoryLoad);
    }
}

SystemStats SystemMonitor::GetCurrentStats() const {
    return currentStats;
}

} // namespace Core
