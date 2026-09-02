#include <windows.h>
#include <iostream>

// Core Modules - Fixed include paths to match actual directory structure
#include "midi/MidiEngine.h"
#include "core/LEDController.h"
#include "core/SystemMonitor.h"
#include "core/DataDrivenController.h"
#include "effects/EffectBase.h"
#include "ui/UIManager.h"

/* ============================================================
   Application State (Global Singleton for v0.2)
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
   Main Application Logic
   ============================================================ */
void UpdateApp(float deltaTime) {
    // 1. Data Acquisition (System Monitoring)
    g_App.monitor.Update();
    auto stats = g_App.monitor.GetCurrentStats();

    // 2. Data Mapping (Transforming system load to animation parameters)
    g_App.dataController.UpdateMapping(stats, g_App.useCpuMonitoring);
    float dynamicFactor = g_App.dataController.GetCurrentFactor();

    // 3. Animation Update (Logic Layer)
    if (g_App.currentEffect) {
        g_App.currentEffect->Update(deltaTime);
        
        Core::EffectParams params;
        params.brightness = g_App.baseBrightness;
        params.speed = g_App.effectSpeed * (1.0f + dynamicFactor); 

        // 4. Hardware Rendering (Driver Layer)
        Core::LEDController led;
        g_App.currentEffect->Render(led, params, dynamicFactor);
    }
}

/* ============================================================
   Win32 Entry Point
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

    // --- Main Execution Loop ---
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // In a real ImGui app:
        // UI::NewFrame();
        // UpdateApp(0.016f);
        // UI::RenderUI(...);
        // UI::EndFrame();
    }

    Midi::MidiEngine::Shutdown();
    UI::Shutdown();

    return 0;
}
