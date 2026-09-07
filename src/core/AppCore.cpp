#include "AppCore.h"
#include "../UI/EffectPage.h"
#include "../UI/StatusPage.h"
#include "../UI/TextPage.h"
#include "../UI/SettingsPage.h"
#include <commctrl.h>

namespace Core {

// 内部使用的静态窗口过程，用于解决 Windows API 回调无法访问类成员的问题
LRESULT CALLBACK AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppCore* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        // 从 CREATESTRUCT 中获取我们在 CreateWindow 时传入的 this 指针
        CREATESTRUCT* pCreateStruct = static_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<AppCore*>(pCreateStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<AppCore*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleWindowProc(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

AppCore::AppCore(AppContext& context) : m_context(context), m_hInstance(nullptr), m_mainWindow(nullptr) {}

AppCore::~AppCore() {
    if (m_mainWindow) DestroyWindow(m_mainWindow);
}

bool AppCore::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;

    // 1. 初始化 UI 页面
    InitializePages();

    // 2. 注册窗口类
    const wchar_t* windowClass = L"LaunchpadRGBAmbientV02";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = AppWndProc; // 使用我们的静态转发函数
    wc.hInstance = hInstance;
    wc.lpszClassName = windowClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc)) return false;

    // 3. 创建窗口，并将 this 指针通过 lpParam 传入
    m_mainWindow = CreateWindowW(
        windowClass,
        L"Launchpad RGB Ambient v0.2",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 440, 350,
        nullptr, nullptr, hInstance, this); // <--- 关键：传入 this

    if (!m_mainWindow) return false;

    ShowWindow(m_mainWindow, SW_SHOW);
    UpdateWindow(m_mainWindow);

    return true;
}

void AppCore::InitializePages() {
    // 创建并管理页面生命周期
    m_pages.push_back(std::make_unique<UI::EffectPage>());
    m_pages.push_back(std::make_unique<UI::StatusPage>());
    m_pages.push_back(std::make_unique<UI::TextPage>());
    m_pages.push_back(std::make_unique<UI::SettingsPage>());

    // 初始化页面，并传入 context 以便它们能访问数据
    for (auto& page : m_pages) {
        page->Initialize(m_mainWindow, m_context);
    }
}

LRESULT AppCore::HandleWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER: {
            if (wParam == 1) {
                // 更新动画相位并驱动所有页面更新
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
