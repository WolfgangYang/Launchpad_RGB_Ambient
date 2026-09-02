#include <windows.h>
#include <iostream>

// Core Modules
#include "midi/MidiEngine.h"
#include "core/LEDController.h"
#include "core/SystemMonitor.h"
#include "core/DataDrivenController.h"
#include "effects/EffectBase.h"
#include "ui/UIManager.h"

// ImGui Mocking for compilation (In real build, these are provided by imgui_impl_dx11)
namespace ImGui {
    void NewFrame();
    void Render();
    void EndFrame();
}

/* ============================================================
   Application State (Global Singleton)
   ============================================================ */
struct AppState {
    bool isRunning = true;
    Core::SystemMonitor monitor;
    Core::DataDrivenController dataController;
    std::unique_ptr<Core::EffectBase> currentEffect;
    
    // User Settings (Controlled via UI)
    float baseBrightness = 1.0f;
    float effectSpeed = 1.0f;
    bool useCpuMonitoring = true;
    UI::PageType currentPage = UI::PageType::Ambient;

} g_App;

/* ============================================================
   Main Application Logic (The Heartbeat)
   ============================================================ */
void UpdateApp(float deltaTime) {
    // 1. Data Acquisition: Get real-time system stats
    g_App.monitor.Update();
    auto stats = g_App.monitor.GetCurrentStats();

    // 2. Data Mapping: Transform CPU/RAM load into animation parameters
    g_App.dataController.UpdateMapping(stats, g_App.useCpuMonitoring);
    float dynamicFactor = g_App.dataController.GetCurrentFactor();

    // 3. Animation Update: Advance the effect's internal phase
    if (g_App.currentEffect) {
        g_App.currentEffect->Update(deltaTime);
        
        Core::EffectParams params;
        params.brightness = g_App.baseBrightness;
        // The "Magic": Speed is modulated by the dynamic system factor!
        params.speed = g_App.effectSpeed * (1.0f + dynamicFactor); 

        // 4. Hardware Rendering: Send calculated colors to Launchpad MK2
        Core::LEDController led;
        g_App.currentEffect->Render(led, params, dynamicFactor);
    }
}

/* ============================================================
   Win32 Entry Point & Main Loop
   ============================================================ */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // --- Initialization Phase ---
    if (!g_App.monitor.Initialize()) {
        MessageBoxW(nullptr, L"Failed to initialize System Monitor.", L"Critical Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    Midi::MidiEngine::Initialize();
    g_App.currentEffect = Core::EffectFactory::Create(Core::EffectType::Rainbow);

    // Window Registration
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProc; 
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LaunchpadRGBAmbient_v0.2";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"LaunchpadRGBAmbient_v0.2", L"Launchpad RGB Ambient v0.2", 
                              WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, 
                              nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    // --- UI Framework Initialization (ImGui/DX11 Placeholder) ---
    UI::Initialize(hwnd);

    // --- Main Execution Loop (The Engine Room) ---
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // A. Start ImGui Frame
        UI::NewFrame();

        // B. Update Logic & Hardware (The "Brain")
        UpdateApp(0.016f); // Target ~60 FPS

        // C. Render UI (The "Face" - User Interaction)
        auto stats = g_App.monitor.GetCurrentStats();
        float factor = g_App.dataController.GetCurrentFactor();
        UI::RenderUI(stats, g_App.currentEffect.get(), factor);

        // D. End ImGui Frame & Present to Screen (The "Eyes")
        // In real implementation: 
        // ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        // swapChain->Present(1, 0);
    }

    // --- Cleanup Phase ---
    Midi::MidiEngine::Shutdown();
    UI::Shutdown();

    return 0;
}
