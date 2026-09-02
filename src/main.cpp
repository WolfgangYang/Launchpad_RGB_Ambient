#include <windows.h>
#include "UI/Page.h"
#include "MIDI/MIDIDevice.h"
#include "LED/LEDController.h"
#include "Effects/Effect.h"
#include "Config/Config.h"
#include "Renderer/Renderer.h"

// 全局指针，方便页面访问
Effect* g_CurrentEffect = nullptr;

// 窗口过程函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    Config::Initialize();
    MIDIDevice::RefreshDevices();
    Renderer::Initialize(nullptr); // 初始化 D2D 引擎

    // 创建窗口
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LaunchpadApp";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"LaunchpadApp", L"Launchpad RGB Ambient v0.2", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 500, nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    ShowWindow(hwnd, nCmdShow);

    // 初始化页面
    EffectPage effectPage;
    StatusPage statusPage;
    TextPage textPage;
    SettingsPage settingsPage;
    std::vector<Page*> pages = { &effectPage, &statusPage, &textPage, &settingsPage };
    int currentPage = 0;

    // 设置初始效果
    g_CurrentEffect = Effect::Create(Effect::Type::Rainbow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Renderer::Cleanup();
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::vector<Page*> pages; // 这里在实际项目中应由 App 类管理
    // 为了演示，我们简化处理：直接通过消息切换页面（你可以用 Tab 键测试）
    static int currentPage = 0;

    switch (msg) {
        case WM_CREATE:
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_TAB) { // 按下 TAB 键切换页面
                currentPage = (currentPage + 1) % 4;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            // 这里应该调用 Renderer::BeginDraw() 和 EndDraw()
            // 为了演示，我们直接在 WndProc 里处理简单的重绘逻辑
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}
