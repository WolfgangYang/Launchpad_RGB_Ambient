#define UNICODE
#define _UNICODE

#include "app/application.h"
#include "ui/main_window.h"

#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "pdh.lib")

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPWSTR,
    int showCommand)
{
    INITCOMMONCONTROLSEX controls{
        sizeof(INITCOMMONCONTROLSEX),
        ICC_BAR_CLASSES
    };
    InitCommonControlsEx(&controls);

    if (!lra::MainWindow::registerClass(instance)) {
        return 1;
    }

    lra::Application app;
    HWND window = lra::MainWindow::create(instance, showCommand, &app);
    if (!window) {
        return 1;
    }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
