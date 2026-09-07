#include <windows.h>
#include "core/AppContext.h"
#include "core/AppCore.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // 1. 初始化上下文 (Application Context)
    Core::AppContext context;

    // 2. 创建核心控制器 (App Core)
    Core::AppCore app(context);

    // 3. 初始化并运行
    if (!app.Initialize(hInstance)) {
        return -1;
    }

    app.RunMessageLoop();

    return 0;
}
