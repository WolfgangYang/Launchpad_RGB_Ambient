#include "core/SystemMonitor.h"
#include "effects/EffectBase.h"
#include <algorithm>

namespace Core {

class DataDrivenController {
public:
    DataDrivenController() : m_currentFactor(0.0f) {}

    // Maps system stats to a single 0.0-1.0 factor for the effect
    void UpdateMapping(const SystemStats& stats, bool useCPU = true, bool useRAM = false) {
        if (useCPU) {
            m_currentFactor = stats.cpuUsage / 100.0f;
        } else if (useRAM) {
            m_currentFactor = stats.ramUsagePercent / 100.0f;
        }
    }

    float GetCurrentFactor() const { return m_currentFactor; }

private:
    float m_currentFactor; // Normalized value from system data (0.0 to 1.0)
};

} // namespace Core
