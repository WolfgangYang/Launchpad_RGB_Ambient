#include "ui/UIManager.h"

// In a real build environment, these would be actual ImGui headers
// We use them to define the structure of our UI implementation.
namespace ImGui {
    struct ImVec2 { float x; float y; };
    void Begin(const char* name);
    void End();
    void Text(const char* fmt, ...);
    void Separator();
    void ProgressBar(float fraction, ImVec2 size, const char* overlay = nullptr);
    void ColorEdit3(const char* label, float col[3]);
    void SliderFloat(const char* label, float* v, float v_min, float v_max);
    void Button(const char* label);
}

namespace UI {

UIManager::UIManager() : m_currentPage(PageType::Ambient) {}

void UIManager::Render(PageType currentPage, 
                       const Core::SystemStats& stats, 
                       Core::EffectBase* currentEffect, 
                       float dynamicFactor) {
    m_currentPage = currentPage;

    // ImGui Window Container
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
    ImGui::Text("Ambient Light Settings");
    ImGui::Separator();

    // In a real app, these would be linked to g_App state via pointers/references
    static float color[3] = { 1.0f, 0.5f, 0.0f };
    static float brightness = 1.0f;
    static float speed = 1.0f;

    ImGui::ColorEdit3("Base Color", color);
    ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f);
    ImGui::SliderFloat("Speed Multiplier", &speed, 0.1f, 5.0f);

    if (ImGui::Button("Reset to Default")) {
        // Reset logic here
    }
}

void UIManager::DrawMonitoringDashboard(const Core::SystemStats& stats, float factor) {
    ImGui::Text("System Performance");
    ImGui::Separator();

    // CPU Monitoring with dynamic color based on load
    char cpuBuf[32];
    sprintf(cpuBuf, "CPU: %.1f%%", stats.cpuUsage);
    float cpuProgress = stats.cpuUsage / 100.0f;
    ImGui::ProgressBar(cpuProgress, ImVec2(-1, 0), cpuBuf);

    // RAM Monitoring
    char ramBuf[32];
    sprintf(ramBuf, "RAM: %.1f%%", stats.ramUsagePercent);
    float ramProgress = stats.ramUsagePercent / 100.0f;
    ImGui::ProgressBar(ramProgress, ImVec2(-1, 0), ramBuf);

    // Visual indicator for the dynamic factor (e.g., Pulse intensity)
    ImGui::Text("Dynamic Intensity Factor: %.2f", factor);
}

void UIManager::DrawMatrixPreview(Core::EffectBase* effect) {
    ImGui::Text("8x8 Matrix Preview");
    ImGui::Separator();

    // Placeholder for a grid-based preview. 
    // In real ImGui, we would use ImDrawList to draw actual colored squares.
    ImGui::Text("[ 8x8 Grid Visualization Area ]");
    for (int i = 0; i < 64; ++i) {
        if (i % 8 != 0) ImGui::Text("|");
        // In real implementation, we'd draw a colored square here
        ImGui::Text("■ "); 
        if ((i + 1) % 8 == 0) ImGui::Text("");
    }
}

void UIManager::DrawSettingsPanel() {
    ImGui::Text("General Settings");
    ImGui::Separator();
    if (ImGui::Button("Save Configuration")) { /* Save logic */ }
    if (ImGui::Button("Load Configuration")) { /* Load logic */ }
}

} // namespace UI
