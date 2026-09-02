// Page Implementations
#include "Page.h"
#include "../Effects/Effect.h"
#include "../Config/Config.h"

// Effect Page implementation
void EffectPage::Initialize(HWND parent) {
    // Initialize effect page with Direct2D rendering
}

void EffectPage::Render() {
    // Render the current effect using Direct2D
    Renderer::Clear();
    
    // Create and render effect
    Effect* effect = Effect::Create(currentEffectType);
    if (effect) {
        effect->Render(animationPhase, Config::GetBrightness(), 
                      cpuIndicator, gpuIndicator, ramIndicator, temperatureIndicator);
        delete effect;
    }
}

void EffectPage::Update() {
    // Update animation phase (~60fps)
    animationPhase += 0.016;
    
    // Render effect
    Render();
}

void EffectPage::OnResize(int width, int height) {
    // Handle resize events
}

const wchar_t* EffectPage::GetTitle() { return L"氛围灯效"; }

// Status Page placeholder
void StatusPage::Initialize(HWND parent) {}
void StatusPage::Render() {}
void StatusPage::Update() {}
void StatusPage::OnResize(int width, int height) {}
const wchar_t* StatusPage::GetTitle() { return L"状态指示"; }

// Text Page placeholder
void TextPage::Initialize(HWND parent) {}
void TextPage::Render() {}
void TextPage::Update() {}
void TextPage::OnResize(int width, int height) {}
const wchar_t* TextPage::GetTitle() { return L"文字显示"; }

// Settings Page placeholder
void SettingsPage::Initialize(HWND parent) {}
void SettingsPage::Render() {}
void SettingsPage::Update() {}
void SettingsPage::OnResize(int width, int height) {}
const wchar_t* SettingsPage::GetTitle() { return L"设置"; }