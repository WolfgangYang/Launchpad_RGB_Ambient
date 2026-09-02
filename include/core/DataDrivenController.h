#ifndef DATA_DRIVEN_CONTROLLER_H
#define DATA_DRIVEN_CONTROLLER_H

#include "core/SystemMonitor.h"

namespace Core {

class DataDrivenController {
public:
    DataDrivenController();
    void UpdateMapping(const SystemStats& stats, bool useCPU = true, bool useRAM = false);
    float GetCurrentFactor() const;

private:
    float m_currentFactor; 
};

} // namespace Core

#endif // DATA_DRIVEN_CONTROLLER_H
