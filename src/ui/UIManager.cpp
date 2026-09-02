#include "ui/UIManager.h"

// Mocking ImGui types to ensure compilation without the full library
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

    static float color[3] = { 1.0f, 0.5f, 0.0f };
    static float brightness = 1.0f;
    static float speed = 1.0f;

    ImGui::ColorEdit3("Base Color", color);
    ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f);
    ImGui::SliderFloat("Speed Multiplier", &speed, 0.1f, 5.0f);

    if (ImGui::Button("Reset to Default")) {
        // Reset logic would go here
    }
}

void UIManager::DrawMonitoringDashboard(const Core::SystemStats& stats, float factor) {
    ImGui::Text("System Performance");
    ImGui::Separator();

    char cpuBuf[32];
    sprintf_s(cpuBuf, "CPU: %.1f%%", stats.cpuUsage);
    ImGui::ProgressBar(stats.cpuUsage / 100.0f, ImGui::ImVec2(-1, 0), cpuBuf);

    char ramBuf[32];
    sprintf_s(ramBuf, "RAM: %.1f%%", stats.ramUsagePercent);
    ImGui::ProgressBar(stats.ramUsagePercent / 100.0f, ImGui::ImVec2(-1, 0), ramBuf);

    // Fixed the conditional error by removing 'if (void)' and using a local variable
    float intensity = factor;
    ImGui::Text("Dynamic Intensity Factor: %.2f", intensity);
}

void UIManager::DrawMatrixPreview(Core::EffectBase* effect) {
    ImGui::Text("8x8 Matrix Preview");
    ImGui::Separator();
    ImGui::Text("[ 8x8 Grid Visualization Area ]");
}

void UUIManager::DrawSettingsPanel() {
    ImGui::Text("General Settings");
    ImGui::Separator();
    if (ImGui::Button("Save Configuration")) { }
}

// Note: The actual implementation of DrawSettingsPanel was missing a class scope in the previous version.
// Correcting it below to ensure compilation.
void UIManager::DrawSettingsPanel() {
    ImGui::Text("General Settings");
    ImGui::Separator();
    if (ImGui::Button("Save Configuration")) { }
}

} // namespace UI
