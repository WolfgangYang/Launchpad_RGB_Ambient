#include "core/SystemMonitor.h"
#include <windows.h>
#include <pdh.h>

// Ensure we are using the Wide (Unicode) versions of Windows APIs
#pragma comment(lib, "pdh.lib")

namespace Core {

SystemMonitor::SystemMonitor() : cpuQuery(nullptr), currentStats{0.0f, 0.0f} {}

SystemMonitor::~SystemMonitor() {
    if (cpuQuery != nullptr) {
        PdhCloseQuery(cpuQuery);
        cpuQuery = nullptr;
    }
}

bool SystemMonitor::Initialize() {
    // Use PdhOpenQueryW for Unicode support
    PDH_STATUS status = PdhOpenQueryW(nullptr, nullptr, &cpuQuery);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    // Use the Wide version of the counter path: L"\\Processor(_Total)\\% Processor Time"
    status = PdhAddCounterW(cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0, &cpuCounter);
    if (status != ERROR_SUCCESS) {
        return false;
    }

    // Initial collection to prime the counter
    PdhCollectQueryData(cpuQuery);
    return true;
}

void SystemMonitor::Update() {
    if (cpuQuery == nullptr || cpuCounter == nullptr) return;

    // 1. Update CPU Usage
    PDH_FMT_COUNTERVALUE counterVal;
    // Use PdhGetFormattedCounterValueW for Unicode compatibility
    if (PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, nullptr, &counterVal) == ERROR_SUCCESS) {
        currentStats.cpuUsage = static_cast<float>(counterVal.doubleValue);
    }

    // 2. Update RAM Usage via GlobalMemoryStatusEx (This is already Unicode-safe in modern Windows)
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
