#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include "core/SystemMonitor.h"
#include "effects/EffectBase.h"

namespace UI {

enum class PageType {
    Ambient,
    Monitoring,
    Matrix,
    Settings
};

class UIManager {
public:
    UIManager();
    ~UIManager() = default;

    // Main entry point for rendering the entire UI
    void Render(PageType currentPage, 
                const Core::SystemStats& stats, 
                Core::EffectBase* currentEffect, 
                float dynamicFactor);

private:
    // Individual Panel Rendering Functions
    void DrawAmbientPanel();
    void DrawMonitoringDashboard(const Core::SystemStats& stats, float factor);
    void DrawMatrixPreview(Core::EffectBase* effect);
    void DrawSettingsPanel();

    PageType m_currentPage;
};

} // namespace UI

#endif // UI_MANAGER_H
