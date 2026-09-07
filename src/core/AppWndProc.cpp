#include "AppCore.h"

namespace Core {

LRESULT CALLBACK AppCore::AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppCore* pApp = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        pApp = static_cast<AppCore*>(pCreateStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
    } else {
        pApp = reinterpret_cast<AppCore*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pApp) {
        return pApp->HandleWindowProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace Core
