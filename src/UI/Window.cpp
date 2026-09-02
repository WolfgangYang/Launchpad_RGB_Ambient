// Window Implementation
#include "Window.h"
#include "../Renderer/Renderer.h"

HWND Window::mainWindow = nullptr;
HINSTANCE Window::hInstance = nullptr;

HWND Window::Create(const wchar_t* className, const wchar_t* title) {
    // Register window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    
    RegisterClassW(&wc);
    
    // Create window
    mainWindow = CreateWindowW(
        className,
        title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    
    if (!mainWindow) return nullptr;
    
    ShowWindow(mainWindow, SW_SHOW);
    UpdateWindow(mainWindow);
    
    // Initialize Direct2D renderer
    Renderer::Initialize(mainWindow);
    
    return mainWindow;
}

void Window::Run() {
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Create pages
            EffectPage* effectPage = new EffectPage();
            StatusPage* statusPage = new StatusPage();
            TextPage* textPage = new TextPage();
            SettingsPage* settingsPage = new SettingsPage();
            
            pages.push_back(effectPage);
            pages.push_back(statusPage);
            pages.push_back(textPage);
            pages.push_back(settingsPage);
            
            // Initialize first page (氛围灯效)
            effectPage->Initialize(hwnd);
            
            return 0;
        }
        
        case WM_TIMER: {
            if (wParam == 1) {
                // Update animation and LED
                if (currentPage < pages.size()) {
                    pages[currentPage]->Update();
                }
            }
            return 0;
        }
        
        case WM_PAINT: {
            // Render the current page
            if (currentPage < pages.size()) {
                pages[currentPage]->Render();
            }
            
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Draw using Direct2D
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY: {
            // Cleanup
            Renderer::Cleanup();
            for (auto* page : pages) delete page;
            PostQuitMessage(0);
            return 0;
        }
        
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}