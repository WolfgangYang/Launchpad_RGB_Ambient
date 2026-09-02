#include "ui/UIManager.h"

// In a real build environment, these would be actual ImGui headers
// We use them to define the structure of our UI implementation.
namespace ImGui {
    struct ImVec2 { float x; float y; };
    void Begin(const char* name);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    float ProgressBar(float fraction, ImVec2 size, const char* overlay = nullptr);
    bool ColorEdit3(const char* label, float col[3]);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label);
}

namespace UI {

UIManager::UIManager() : m_currentPage(PageType::Ambient) {}

void UIManager::Render(PageType currentPage, 
                       const Core::SystemStats& stats, 
                       Core::EffectBase* currentEffect, 
                       float dynamicFactor) {
    m_currentPage = currentPage;

    // Main Window Container
    ImGui::Begin("Launchpad RGB Ambient Control");

    switch (m_currentPage) {
        case PageType::Ambient:
            DrawAmbientPanel();
            break;
        case PageType::Monitoring:
            DrawMonitoringDashboard(stats, dynamicFactor);
            break;
        case PageType::Matrix:
            DrawMatrixPreview(currentEffect);
            break;
        case PageType::Settings:
            DrawSettingsPanel();
            break;
    }

    ImGui::End();
}

void UIManager::DrawAmbientPanel() {
    ImGui::Text("🎨 Ambient Light Settings");
    ImGui::Separator();

    // Static state variables (In a real app, these would be in AppState)
    static float color[3] = { 1.0f, 0.5f, 0.0f };
    static float brightness = 1.0f;
    static float speed = 1.0f;

    // --- The "Paint Tool" Experience ---
    // This provides the professional color picker you requested!
    ImGui::ColorEdit3("Base Color", color);
    
    ImGui::Spacing();
    ImGui::Text("Intensity & Speed");
    ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f);
    ImGui::SliderFloat("Speed Multiplier", &speed, 0.1f, 5.0f);

    if (ImGui::Button("Reset to Default")) {
        // Reset logic would be triggered here
    }
}

void UIManager::DrawMonitoringDashboard(const Core::SystemStats& stats, float factor) {
    ImGui::Text("📊 System Performance");
    ImGui::Separator();

    // CPU Progress Bar - Color changes based on load (Visual feedback)
    char cpuBuf[32];
    sprintf_s(cpuBuf, "CPU: %.1f%%", stats.cpuUsage);
    float cpuProgress = stats.cpuUsage / 100.0f;
    ImGui::ProgressBar(cpuProgress, ImVec2(-1, 0), cpuBuf);

    // RAM Progress Bar
    char ramBuf[32];
    sprintf_s(ramBuf, "RAM: %.1f%%", stats.ramUsagePercent);
    float ramProgress = stats.ramUsagePercent / 100.0f;
    ImGui::ProgressBar(ramProgress, ImVec2(-1, 0), ramBuf);

    // Dynamic Intensity Indicator (Visualizing the 'Factor')
    ImGui::Text("Dynamic Load Factor: %.2f", factor);
}

void UIManager::DrawMatrixPreview(Core::EffectBase* effect) {
    ImGui::Text("🖼️ 8x8 Matrix Preview");
    ImGui::Separator();
    // In a real implementation, we would use ImDrawList to render the grid here.
    ImGui::Text("[ Grid Visualization Placeholder ]");
}

void UIManager::DrawSettingsPanel() {
    ImGui::Text("⚙️ General Settings");
    ImGui::Separator();
    if (ImGui::Button("Save Configuration")) { }
}

} // namespace UI
