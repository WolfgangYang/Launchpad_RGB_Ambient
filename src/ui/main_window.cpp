#include "main_window.h"
#include "../app/application.h"
#include "../localization/localization.h"

#include <commctrl.h>

namespace lra {

namespace {

enum ControlId {
    ID_PORT = 100,
    ID_REFRESH,
    ID_CONNECT,
    ID_RAINBOW = 110,
    ID_BREATHE,
    ID_WAVE,
    ID_STARS,
    ID_SOLID,
    ID_CPU = 120,
    ID_GPU,
    ID_RAM,
    ID_TEMP,
    ID_BRIGHTNESS = 130,
    ID_SPEED,
    ID_STATUS
};

struct WindowData {
    Application* app = nullptr;
    HWND portCombo = nullptr;
    HWND status = nullptr;
    HWND brightness = nullptr;
    HWND speed = nullptr;
    HFONT font = nullptr;
};

HWND label(HWND parent, LPCWSTR value, int x, int y, int w, int h)
{
    return CreateWindowW(L"STATIC", value,
        WS_CHILD | WS_VISIBLE, x, y, w, h,
        parent, nullptr, GetModuleHandleW(nullptr), nullptr);
}

HWND button(HWND parent, int id, LPCWSTR value, int x, int y, int w, int h)
{
    return CreateWindowW(L"BUTTON", value,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr), nullptr);
}

void applyFont(HWND window, HFONT font)
{
    EnumChildWindows(window,
        [](HWND child, LPARAM param) -> BOOL {
            SendMessageW(child, WM_SETFONT, param, TRUE);
            return TRUE;
        },
        reinterpret_cast<LPARAM>(font));
}

void createControls(HWND window, WindowData& data)
{
    const auto& state = data.app->state();

    label(window, text(state.language, "title"), 18, 12, 380, 30);
    label(window, text(state.language, "midi"), 18, 50, 40, 22);

    data.portCombo = CreateWindowW(
        L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        60, 47, 340, 180, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_PORT)),
        GetModuleHandleW(nullptr), nullptr);

    button(window, ID_REFRESH, text(state.language, "refresh"), 18, 78, 95, 30);
    button(window, ID_CONNECT, text(state.language, "connect"), 120, 78, 95, 30);

    button(window, ID_RAINBOW, text(state.language, "rainbow"), 18, 120, 72, 30);
    button(window, ID_BREATHE, text(state.language, "breathe"), 96, 120, 72, 30);
    button(window, ID_WAVE, text(state.language, "wave"), 174, 120, 72, 30);
    button(window, ID_STARS, text(state.language, "stars"), 252, 120, 72, 30);
    button(window, ID_SOLID, text(state.language, "solid"), 330, 120, 72, 30);

    label(window, text(state.language, "monitor"), 18, 160, 80, 22);
    button(window, ID_CPU, text(state.language, "cpu"), 92, 155, 60, 30);
    button(window, ID_GPU, text(state.language, "gpu"), 158, 155, 60, 30);
    button(window, ID_RAM, text(state.language, "ram"), 224, 155, 60, 30);
    button(window, ID_TEMP, text(state.language, "temp"), 290, 155, 60, 30);

    label(window, text(state.language, "brightness"), 18, 198, 70, 22);
    data.brightness = CreateWindowW(
        TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE,
        80, 193, 320, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BRIGHTNESS)),
        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.brightness, TBM_SETRANGE, TRUE, MAKELONG(5, 100));
    SendMessageW(data.brightness, TBM_SETPOS, TRUE, state.brightness);

    label(window, text(state.language, "speed"), 18, 238, 70, 22);
    data.speed = CreateWindowW(
        TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE,
        80, 233, 320, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_SPEED)),
        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.speed, TBM_SETRANGE, TRUE, MAKELONG(1, 50));
    SendMessageW(data.speed, TBM_SETPOS, TRUE, state.speed);

    data.status = label(window, text(state.language, "notconnected"), 18, 280, 380, 24);

    data.app->refreshMidiPorts(data.portCombo);
}

} // namespace

bool MainWindow::registerClass(HINSTANCE instance)
{
    WNDCLASSW wc{};
    wc.lpfnWndProc = procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"LaunchpadRGBAmbientV02";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    return RegisterClassW(&wc) != 0;
}

HWND MainWindow::create(HINSTANCE instance, int showCommand, Application* app)
{
    HWND window = CreateWindowW(
        L"LaunchpadRGBAmbientV02",
        windowTitle(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 440, 350,
        nullptr, nullptr, instance, app);

    if (!window) {
        return nullptr;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);
    return window;
}

LRESULT CALLBACK MainWindow::procedure(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* data = reinterpret_cast<WindowData*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));

    switch (message) {
    case WM_NCCREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* app = reinterpret_cast<Application*>(cs->lpCreateParams);

        auto* newData = new WindowData{};
        newData->app = app;
        SetWindowLongPtrW(window, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(newData));
        data = newData;
        return TRUE;
    }

    case WM_CREATE:
        data->app->initialize(window);
        createControls(window, *data);
        data->font = CreateFontW(
            -15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Microsoft YaHei UI");
        applyFont(window, data->font);
        SetTimer(window, 1, 80, nullptr);
        SetTimer(window, 2, 1000, nullptr);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_REFRESH:
            data->app->refreshMidiPorts(data->portCombo);
            break;
        case ID_CONNECT:
            data->app->connectMidi(data->portCombo, data->status);
            break;
        case ID_RAINBOW:
            data->app->setEffect(Effect::Rainbow);
            break;
        case ID_BREATHE:
            data->app->setEffect(Effect::Breathe);
            break;
        case ID_WAVE:
            data->app->setEffect(Effect::Wave);
            break;
        case ID_STARS:
            data->app->setEffect(Effect::Stars);
            break;
        case ID_SOLID:
            data->app->setEffect(Effect::Solid);
            break;
        case ID_CPU:
            data->app->toggleCpu();
            break;
        case ID_GPU:
            data->app->toggleGpu();
            break;
        case ID_RAM:
            data->app->toggleRam();
            break;
        case ID_TEMP:
            data->app->toggleTemperature();
            break;
        }
        return 0;

    case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lParam) == data->brightness) {
            data->app->state().brightness =
                static_cast<int>(SendMessageW(data->brightness, TBM_GETPOS, 0, 0));
        } else if (reinterpret_cast<HWND>(lParam) == data->speed) {
            data->app->state().speed =
                static_cast<int>(SendMessageW(data->speed, TBM_GETPOS, 0, 0));
        }
        return 0;

    case WM_TIMER:
        if (wParam == 1) data->app->render();
        if (wParam == 2) data->app->updateStatistics();
        return 0;

    case WM_DESTROY:
        KillTimer(window, 1);
        KillTimer(window, 2);
        data->app->shutdown();
        if (data->font) DeleteObject(data->font);
        delete data;
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace lra
