#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <windows.h>
#include <pdh.h>
#include <string>

namespace Core {

struct SystemStats {
    float cpuUsage;      // 0.0 to 100.0
    float ramUsagePercent; // 0.0 to 100.0
    // Future: float gpuTemp, float diskSpace, etc.
};

class SystemMonitor {
public:
    SystemMonitor();
    ~SystemMonitor();

    bool Initialize();
    void Update(); // Should be called once per frame/interval
    SystemStats GetCurrentStats() const;

private:
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuCounter;
    SystemStats currentStats;
};

} // namespace Core

#endif // SYSTEM_MONITOR_H
