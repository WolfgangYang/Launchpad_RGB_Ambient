#pragma once

#include "../core/types.h"
#include <windows.h>
#include <pdh.h>

namespace lra {

class SystemMonitor {
public:
    SystemMonitor() = default;
    ~SystemMonitor();

    bool initialize();
    void shutdown();
    void update(MonitoringState& state);

private:
    PDH_HQUERY cpuQuery_ = nullptr;
    PDH_HCOUNTER cpuCounter_ = nullptr;
};

} // namespace lra
