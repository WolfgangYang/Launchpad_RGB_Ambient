// Launchpad RGB Ambient v0.2
// Modular architecture for Windows + C++

#include "UI/Page.h"
#include "MIDI/MIDIDevice.h"
#include "LED/LEDController.h"
#include "Effects/Effect.h"
#include "Config/Config.h"
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <pdh.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "pdh.lib")

static const wchar_t* WINDOW_TITLE = L"Launchpad RGB Ambient v0.2";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // Initialize config
    Config::Initialize();
    
    // Register window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LaunchpadRGBAmbientV02";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    
    RegisterClassW(&wc);
    
    // Create window
    HWND hwnd = CreateWindowW(
        L"LaunchpadRGBAmbientV02",
        WINDOW_TITLE,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);
    
    if (!hwnd) return 1;
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    return 0;
}

// Window procedure - simplified for compilation test
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Create pages
            EffectPage* effectPage = new EffectPage();
            StatusPage* statusPage = new StatusPage();
            TextPage* textPage = new TextPage();
            SettingsPage* settingsPage = new SettingsPage();
            
            return 0;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}