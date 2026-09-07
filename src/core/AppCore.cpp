#include "AppCore.h"
#include "../core/AppContext.h"

namespace Core {

// 实现 AppCore 的初始化逻辑，确保所有模块正确加载
bool AppCore::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;

    // 1. 初始化页面 (现在通过 context 管理生命周期)
    InitializePages();

    // 2. 创建窗口类
    const wchar_t* windowClass = L"LaunchpadRGBAmbientV02";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = AppCore::AppWndProc; // 使用我们在下面定义的静态转发函数
    wc.hInstance = hInstance;
    wc.lpszClassName = windowClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc)) return false;

    // 3. 创建窗口，并将 this 指针传入 lpParam 以便 WndProc 能找到 AppCore 实例
    m_mainWindow = CreateWindowW(
        windowClass,
        L"Launchpad RGB Ambient v0.2",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 440, 350,
        nullptr, nullptr, hInstance, this);

    if (!m_mainWindow) return false;

    // Start a timer for the animation loop (~60 FPS)
    SetTimer(m_mainWindow, 1, 16, nullptr);

    ShowWindow(m_mainWindow, SW_SHOW);
    UpdateWindow(m_mainWindow);

    return true;
}

void AppCore::InitializePages() {
    // 实例化页面并存入容器，通过 context 进行初始化
    m_pages.push_back(std::make_unique<UI::EffectPage>());
    m_pages.push_back(std::make_unique<UI::StatusPage>());
    m_pages.push_back(std::make_unique<UI::TextPage>());
    m_pages.push_back(std::make_unique<UI::SettingsPage>());

    for (auto& page : m_pages) {
        page->Initialize(m_mainWindow, m_context);
    }
}

LRESULT AppCore::HandleWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER: {
            if (wParam == 1) {
                m_context.UpdateAnimation(0.016); // ~60fps
                for (auto& page : m_pages) {
                    page->Update(m_context);
                }
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void AppCore::RunMessageLoop() {
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

} // namespace Core
