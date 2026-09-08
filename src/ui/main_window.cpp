#include "main_window.h"
#include "../app/application.h"
#include "../core/color.h"
#include "../localization/localization.h"

#include <commctrl.h>
#include <algorithm>
#include <string>

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
    ID_STATUS,
    ID_TAB_EFFECTS = 200,
    ID_TAB_SYSTEM,
    ID_TAB_TEXT,
    ID_TAB_DEVICE,
    ID_TAB_SETTINGS,
    ID_PLACEHOLDER = 250,
    ID_PALETTE_BASE = 400
};

struct WindowData {
    Application* app = nullptr;
    HWND portCombo = nullptr;
    HWND status = nullptr;
    HWND brightness = nullptr;
    HWND speed = nullptr;
    HWND effectPage = nullptr;
    HWND systemPage = nullptr;
    HWND textPage = nullptr;
    HWND devicePage = nullptr;
    HWND settingsPage = nullptr;
    HFONT font = nullptr;
    int tab = 0;
};

void applyFont(HWND parent, HFONT font)
{
    if (!parent || !font) {
        return;
    }

    SendMessageW(
        parent,
        WM_SETFONT,
        reinterpret_cast<WPARAM>(font),
        TRUE
    );
}

HWND label(HWND parent, const std::wstring& textValue,
           int x, int y, int width, int height)
{
    HWND control = CreateWindowExW(
        0,
        L"STATIC",
        textValue.c_str(),
        WS_CHILD | WS_VISIBLE,
        x,
        y,
        width,
        height,
        parent,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    return control;
}

HWND button(HWND parent, const std::wstring& textValue,
            int id, int x, int y, int width, int height)
{
    HWND control = CreateWindowExW(
        0,
        L"BUTTON",
        textValue.c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x,
        y,
        width,
        height,
        parent,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(id)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    return control;
}

void createPlaceholderPage(
    HWND parent,
    const std::wstring& title,
    const std::wstring& description,
    HFONT font)
{
    HWND page = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        1,
        1,
        parent,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    SetWindowLongPtrW(
        page,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(font)
    );

    HWND titleLabel = label(
        page,
        title,
        20,
        20,
        400,
        28
    );

    HWND descriptionLabel = label(
        page,
        description,
        20,
        60,
        400,
        60
    );

    applyFont(titleLabel, font);
    applyFont(descriptionLabel, font);
}

void createEffectsPage(WindowData& data)
{
    data.effectPage = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        0,
        0,
        1,
        1,
        data.app ? nullptr : nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );
}

void positionPage(HWND page, int x, int y, int width, int height)
{
    if (!page) {
        return;
    }

    SetWindowPos(
        page,
        nullptr,
        x,
        y,
        width,
        height,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void selectTab(WindowData& data, int tab)
{
    data.tab = tab;

    if (data.effectPage) {
        ShowWindow(
            data.effectPage,
            tab == 0 ? SW_SHOW : SW_HIDE
        );
    }

    if (data.systemPage) {
        ShowWindow(
            data.systemPage,
            tab == 1 ? SW_SHOW : SW_HIDE
        );
    }

    if (data.textPage) {
        ShowWindow(
            data.textPage,
            tab == 2 ? SW_SHOW : SW_HIDE
        );
    }

    if (data.devicePage) {
        ShowWindow(
            data.devicePage,
            tab == 3 ? SW_SHOW : SW_HIDE
        );
    }

    if (data.settingsPage) {
        ShowWindow(
            data.settingsPage,
            tab == 4 ? SW_SHOW : SW_HIDE
        );
    }
}

void createTabs(WindowData& data, HWND window)
{
    const int x = 10;
    const int y = 10;
    const int width = 450;
    const int height = 30;

    button(window, L"Effects", ID_TAB_EFFECTS,
           x, y, 85, height);

    button(window, L"System", ID_TAB_SYSTEM,
           x + 90, y, 85, height);

    button(window, L"Text", ID_TAB_TEXT,
           x + 180, y, 85, height);

    button(window, L"Device", ID_TAB_DEVICE,
           x + 270, y, 85, height);

    button(window, L"Settings", ID_TAB_SETTINGS,
           x + 360, y, 85, height);
}

void createPalette(WindowData& data)
{
    if (!data.effectPage || !data.app) {
        return;
    }

    const int startX = 20;
    const int startY = 250;
    const int swatch = 18;
    const int gap = 3;

    for (int i = 0; i < 128; ++i) {
        const int row = i / 16;
        const int col = i % 16;

        const int x =
            startX + col * (swatch + gap);

        const int y =
            startY + row * (swatch + gap);

        HWND control = CreateWindowExW(
            0,
            L"BUTTON",
            nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x,
            y,
            swatch,
            swatch,
            data.effectPage,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(
                    ID_PALETTE_BASE + i
                )
            ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        if (control) {
            SendMessageW(
                control,
                WM_SETFONT,
                reinterpret_cast<WPARAM>(data.font),
                TRUE
            );
        }
    }
}

void createEffectControls(WindowData& data)
{
    if (!data.effectPage) {
        return;
    }

    label(
        data.effectPage,
        L"MIDI Device",
        20,
        20,
        100,
        22
    );

    data.portCombo = CreateWindowExW(
        0,
        L"COMBOBOX",
        nullptr,
        WS_CHILD | WS_VISIBLE |
        CBS_DROPDOWNLIST | WS_VSCROLL,
        20,
        45,
        300,
        250,
        data.effectPage,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(ID_PORT)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    button(
        data.effectPage,
        L"Refresh",
        ID_REFRESH,
        330,
        45,
        80,
        28
    );

    button(
        data.effectPage,
        L"Connect",
        ID_CONNECT,
        330,
        80,
        80,
        28
    );

    data.status = label(
        data.effectPage,
        L"Not connected",
        20,
        80,
        200,
        22
    );

    label(
        data.effectPage,
        L"Effect",
        20,
        120,
        100,
        22
    );

    button(
        data.effectPage,
        L"Rainbow",
        ID_RAINBOW,
        20,
        145,
        75,
        28
    );

    button(
        data.effectPage,
        L"Breathe",
        ID_BREATHE,
        100,
        145,
        75,
        28
    );

    button(
        data.effectPage,
        L"Wave",
        ID_WAVE,
        180,
        145,
        75,
        28
    );

    button(
        data.effectPage,
        L"Stars",
        ID_STARS,
        260,
        145,
        75,
        28
    );

    button(
        data.effectPage,
        L"Solid",
        ID_SOLID,
        340,
        145,
        75,
        28
    );

    label(
        data.effectPage,
        L"Monitoring",
        20,
        180,
        100,
        22
    );

    button(
        data.effectPage,
        L"CPU",
        ID_CPU,
        20,
        205,
        70,
        28
    );

    button(
        data.effectPage,
        L"GPU",
        ID_GPU,
        95,
        205,
        70,
        28
    );

    button(
        data.effectPage,
        L"RAM",
        ID_RAM,
        170,
        205,
        70,
        28
    );

    button(
        data.effectPage,
        L"Temp",
        ID_TEMP,
        245,
        205,
        70,
        28
    );

    label(
        data.effectPage,
        L"Palette",
        20,
        225,
        100,
        22
    );

    createPalette(data);

    data.brightness = CreateWindowExW(
        0,
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        330,
        120,
        120,
        30,
        data.effectPage,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(ID_BRIGHTNESS)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(
        data.brightness,
        TBM_SETRANGE,
        TRUE,
        MAKELONG(0, 100)
    );

    SendMessageW(
        data.brightness,
        TBM_SETPOS,
        TRUE,
        70
    );

    data.speed = CreateWindowExW(
        0,
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        330,
        160,
        120,
        30,
        data.effectPage,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(ID_SPEED)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(
        data.speed,
        TBM_SETRANGE,
        TRUE,
        MAKELONG(1, 100)
    );

    SendMessageW(
        data.speed,
        TBM_SETPOS,
        TRUE,
        20
    );
}

void createDevicePage(WindowData& data, HWND window)
{
    data.devicePage = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD,
        10,
        50,
        450,
        550,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    label(
        data.devicePage,
        L"Device",
        20,
        20,
        400,
        28
    );

    label(
        data.devicePage,
        L"Launchpad device information will appear here.",
        20,
        60,
        420,
        40
    );
}

void createControls(WindowData& data, HWND window)
{
    data.font = CreateFontW(
        -15,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Microsoft YaHei UI"
    );

    createTabs(data, window);

    data.effectPage = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        10,
        50,
        450,
        550,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    createEffectControls(data);

    data.systemPage = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD,
        10,
        50,
        450,
        550,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    label(
        data.systemPage,
        L"System",
        20,
        20,
        400,
        28
    );

    label(
        data.systemPage,
        L"System monitoring options.",
        20,
        60,
        400,
        40
    );

    data.textPage = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD,
        10,
        50,
        450,
        550,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    label(
        data.textPage,
        L"Text",
        20,
        20,
        400,
        28
    );

    label(
        data.textPage,
        L"Text and pixel-font functions are reserved for a future version.",
        20,
        60,
        420,
        50
    );

    createDevicePage(data, window);

    data.settingsPage = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD,
        10,
        50,
        450,
        550,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    label(
        data.settingsPage,
        L"Settings",
        20,
        20,
        400,
        28
    );

    label(
        data.settingsPage,
        L"Application settings are reserved for a future version.",
        20,
        60,
        420,
        50
    );

    selectTab(data, 0);

    const auto ports = data.app->midi().enumerate();

    for (const auto& port : ports) {
        const int index = static_cast<int>(
            SendMessageW(
                data.portCombo,
                CB_ADDSTRING,
                0,
                reinterpret_cast<LPARAM>(
                    port.name.c_str()
                )
            )
        );

        SendMessageW(
            data.portCombo,
            CB_SETITEMDATA,
            index,
            static_cast<LPARAM>(
                port.deviceIndex
            )
        );
    }

    if (!ports.empty()) {
        SendMessageW(
            data.portCombo,
            CB_SETCURSEL,
            0,
            0
        );
    }
}

void drawPreview(
    HDC dc,
    const RECT& client,
    const Application& app,
    Language language)
{
    const int panelX = 485;
    const int panelY = 70;
    const int panelW = client.right - panelX - 18;
    const int panelH = client.bottom - panelY - 18;

    HBRUSH bg = CreateSolidBrush(
        RGB(245, 245, 245)
    );

    RECT panel{
        panelX,
        panelY,
        panelX + panelW,
        panelY + panelH
    };

    FillRect(
        dc,
        &panel,
        bg
    );

    DeleteObject(bg);

    FrameRect(
        dc,
        &panel,
        static_cast<HBRUSH>(
            GetStockObject(GRAY_BRUSH)
        )
    );

    SetBkMode(
        dc,
        TRANSPARENT
    );

    const std::wstring title =
        text(language, "preview");

    TextOutW(
        dc,
        panelX + 18,
        panelY + 15,
        title.c_str(),
        static_cast<int>(
            title.size()
        )
    );

    /*
        Preview layout:

             T T T T T T T T
             ┌──────────────┐
             │  8 x 8 grid  │ R
             │              │ R
             │              │ R
             └──────────────┘ R

        The complete Launchpad area is calculated first so
        the right-side keys can never extend outside the panel.
    */

    const int margin = 28;
    const int topKeyHeight = 20;
    const int rightKeyWidth = 20;
    const int gap = 3;

    const int availableWidth =
        panelW
        - margin * 2
        - rightKeyWidth
        - gap;

    const int availableHeight =
        panelH
        - 85
        - margin;

    const int cellByWidth =
        (availableWidth - gap * 7) / 8;

    const int cellByHeight =
        (availableHeight - topKeyHeight - gap * 8) / 8;

    const int cell =
        std::max(
            12,
            std::min(
                cellByWidth,
                cellByHeight
            )
        );

    const int gridSize =
        cell * 8 + gap * 7;

    const int totalWidth =
        gridSize
        + gap
        + rightKeyWidth;

    const int totalHeight =
        topKeyHeight
        + gap
        + gridSize;

    const int baseX =
        panelX
        + (panelW - totalWidth) / 2;

    const int baseY =
        panelY
        + 58
        + (panelH - 58 - totalHeight) / 2;

    const int gridX = baseX;

    const int gridY =
        baseY
        + topKeyHeight
        + gap;

    const auto& frame =
        app.frame();

    // Main 8 x 8 grid.
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const Rgb& c =
                frame[y][x];

            HBRUSH brush =
                CreateSolidBrush(
                    RGB(
                        static_cast<int>(
                            c.r * 255.0
                        ),
                        static_cast<int>(
                            c.g * 255.0
                        ),
                        static_cast<int>(
                            c.b * 255.0
                        )
                    )
                );

            RECT r{
                gridX + x * (cell + gap),
                gridY + y * (cell + gap),
                gridX + x * (cell + gap) + cell,
                gridY + y * (cell + gap) + cell
            };

            FillRect(
                dc,
                &r,
                brush
            );

            DeleteObject(brush);

            FrameRect(
                dc,
                &r,
                static_cast<HBRUSH>(
                    GetStockObject(GRAY_BRUSH)
                )
            );
        }
    }

    const auto& keys =
        app.functionKeys();

    // Top 8 function keys.
    for (int i = 0; i < 8; ++i) {
        const Rgb& c =
            keys[8 + i];

        HBRUSH brush =
            CreateSolidBrush(
                RGB(
                    static_cast<int>(
                        c.r * 255.0
                    ),
                    static_cast<int>(
                        c.g * 255.0
                    ),
                    static_cast<int>(
                        c.b * 255.0
                    )
                )
            );

        const int x =
            gridX
            + i * (cell + gap);

        RECT r{
            x,
            baseY,
            x + cell,
            baseY + topKeyHeight
        };

        FillRect(
            dc,
            &r,
            brush
        );

        DeleteObject(brush);

        FrameRect(
            dc,
            &r,
            static_cast<HBRUSH>(
                GetStockObject(GRAY_BRUSH)
            )
        );
    }

    // Right 8 function keys.
    const int rightX =
        gridX
        + gridSize
        + gap;

    for (int i = 0; i < 8; ++i) {
        const Rgb& c =
            keys[i];

        HBRUSH brush =
            CreateSolidBrush(
                RGB(
                    static_cast<int>(
                        c.r * 255.0
                    ),
                    static_cast<int>(
                        c.g * 255.0
                    ),
                    static_cast<int>(
                        c.b * 255.0
                    )
                )
            );

        const int y =
            gridY
            + i * (cell + gap);

        RECT r{
            rightX,
            y,
            rightX + rightKeyWidth,
            y + cell
        };

        FillRect(
            dc,
            &r,
            brush
        );

        DeleteObject(brush);

        FrameRect(
            dc,
            &r,
            static_cast<HBRUSH>(
                GetStockObject(GRAY_BRUSH)
            )
        );
    }

    const std::wstring hint =
        text(language, "preview_hint");

    TextOutW(
        dc,
        panelX + 18,
        panel.bottom - 35,
        hint.c_str(),
        static_cast<int>(
            hint.size()
        )
    );
}

} // namespace

bool MainWindow::registerClass(
    HINSTANCE instance)
{
    WNDCLASSW wc{};

    wc.lpfnWndProc =
        procedure;

    wc.hInstance =
        instance;

    wc.lpszClassName =
        L"LaunchpadRGBAmbient";

    wc.hCursor =
        LoadCursorW(
            nullptr,
            MAKEINTRESOURCEW(32512)
        );

    wc.hbrBackground =
        static_cast<HBRUSH>(
            GetStockObject(WHITE_BRUSH)
        );

    return RegisterClassW(&wc) != 0;
}

HWND MainWindow::create(
    HINSTANCE instance,
    int showCommand,
    Application* app)
{
    HWND window =
        CreateWindowExW(
            0,
            L"LaunchpadRGBAmbient",
            L"Launchpad RGB Ambient",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            930,
            650,
            nullptr,
            nullptr,
            instance,
            app
        );

    if (!window) {
        return nullptr;
    }

    ShowWindow(
        window,
        showCommand
    );

    UpdateWindow(window);

    return window;
}

LRESULT MainWindow::procedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    WindowData* data =
        reinterpret_cast<WindowData*>(
            GetWindowLongPtrW(
                window,
                GWLP_USERDATA
            )
        );

    switch (message) {
    case WM_NCCREATE: {
        const auto* create =
            reinterpret_cast<
                const CREATESTRUCTW*
            >(lParam);

        auto* app =
            reinterpret_cast<Application*>(
                create->lpCreateParams
            );

        auto* windowData =
            new WindowData();

        windowData->app =
            app;

        SetWindowLongPtrW(
            window,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(
                windowData
            )
        );

        if (app) {
            app->initialize(window);
        }

        return TRUE;
    }

    case WM_CREATE:
        data =
            reinterpret_cast<WindowData*>(
                GetWindowLongPtrW(
                    window,
                    GWLP_USERDATA
                )
            );

        if (data) {
            createControls(
                *data,
                window
            );

            SetTimer(
                window,
                1,
                80,
                nullptr
            );

            SetTimer(
                window,
                2,
                500,
                nullptr
            );
        }

        return 0;

    case WM_COMMAND:
        if (!data || !data->app) {
            break;
        }

        switch (LOWORD(wParam)) {
        case ID_TAB_EFFECTS:
            selectTab(*data, 0);
            break;

        case ID_TAB_SYSTEM:
            selectTab(*data, 1);
            break;

        case ID_TAB_TEXT:
            selectTab(*data, 2);
            break;

        case ID_TAB_DEVICE:
            selectTab(*data, 3);
            break;

        case ID_TAB_SETTINGS:
            selectTab(*data, 4);
            break;

        case ID_REFRESH:
            data->app->refreshMidiPorts(
                data->portCombo
            );
            break;

        case ID_CONNECT:
            data->app->connectMidi(
                data->portCombo,
                data->status
            );
            break;

        case ID_RAINBOW:
            data->app->setEffect(
                Effect::Rainbow
            );
            break;

        case ID_BREATHE:
            data->app->setEffect(
                Effect::Breathe
            );
            break;

        case ID_WAVE:
            data->app->setEffect(
                Effect::Wave
            );
            break;

        case ID_STARS:
            data->app->setEffect(
                Effect::Stars
            );
            break;

        case ID_SOLID:
            data->app->setEffect(
                Effect::Solid
            );
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

        default:
            if (
                LOWORD(wParam) >= ID_PALETTE_BASE
                &&
                LOWORD(wParam)
                    < ID_PALETTE_BASE + 128
            ) {
                data->app->setPaletteColor(
                    LOWORD(wParam)
                    - ID_PALETTE_BASE
                );

                InvalidateRect(
                    window,
                    nullptr,
                    FALSE
                );
            }

            break;
        }

        return 0;

    case WM_HSCROLL:
        if (!data || !data->app) {
            break;
        }

        if (
            reinterpret_cast<HWND>(lParam)
            == data->brightness
        ) {
            data->app->state().brightness =
                static_cast<int>(
                    SendMessageW(
                        data->brightness,
                        TBM_GETPOS,
                        0,
                        0
                    )
                );
        }
        else if (
            reinterpret_cast<HWND>(lParam)
            == data->speed
        ) {
            data->app->state().speed =
                static_cast<int>(
                    SendMessageW(
                        data->speed,
                        TBM_GETPOS,
                        0,
                        0
                    )
                );
        }

        return 0;

    case WM_TIMER:
        if (!data || !data->app) {
            break;
        }

        if (wParam == 1) {
            data->app->render();
        }

        if (wParam == 2) {
            data->app->updateStatistics();
        }

        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps{};

        HDC dc =
            BeginPaint(
                window,
                &ps
            );

        RECT client{};

        GetClientRect(
            window,
            &client
        );

        if (data && data->app) {
            drawPreview(
                dc,
                client,
                *data->app,
                data->app->state().language
            );
        }

        EndPaint(
            window,
            &ps
        );

        return 0;
    }

    case WM_DESTROY:
        KillTimer(
            window,
            1
        );

        KillTimer(
            window,
            2
        );

        if (data) {
            if (data->app) {
                data->app->shutdown();
            }

            if (data->font) {
                DeleteObject(
                    data->font
                );
            }

            delete data;

            SetWindowLongPtrW(
                window,
                GWLP_USERDATA,
                0
            );
        }

        PostQuitMessage(0);

        return 0;
    }

    return DefWindowProcW(
        window,
        message,
        wParam,
        lParam
    );
}

} // namespace lra
