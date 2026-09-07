#pragma once

#include <windows.h>
#include <vector>
#include <memory>
#include "../core/AppContext.h"
#include "../UI/Page.h"

namespace Core {

class AppCore {
public:
    AppCore(AppContext& context);
    ~AppCore();

    // 初始化所有模块 (MIDI, UI Pages 等)
    bool Initialize(HINSTANCE hInstance);

    // 处理 Windows 消息循环中的更新逻辑
    void RunMessageLoop();

    // 处理窗口过程 (WndProc)
    LRESULT HandleWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    AppContext& m_context;
    std::vector<std::unique_ptr<UI::IPage>> m_pages;
    HINSTANCE m_hInstance;
    HWND m_mainWindow;

    void InitializePages();
};

} // namespace Core
