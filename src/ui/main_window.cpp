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
    ID_TABS = 205,
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
    HWND tabControl = nullptr;
    int tab = 0;
};

HWND label(HWND parent, LPCWSTR value, int x, int y, int w, int h, int id = 0)
{
    return CreateWindowW(
        L"STATIC",
        value,
        WS_CHILD | WS_VISIBLE,
        x,
        y,
        w,
        h,
        parent,
        id ? reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)) : nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );
}

HWND button(
    HWND parent,
    int id,
    LPCWSTR value,
    int x,
    int y,
    int w,
    int h,
    DWORD style = BS_PUSHBUTTON)
{
    return CreateWindowW(
        L"BUTTON",
        value,
        WS_CHILD | WS_VISIBLE | style,
        x,
        y,
        w,
        h,
        parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr),
        nullptr
    );
}

/*
    Controls inside a STATIC page normally send their notifications
    to that page instead of the main window.

    Forward the relevant messages back to the main window so the
    existing WM_COMMAND / WM_HSCROLL / WM_DRAWITEM handling remains
    unchanged.
*/
LRESULT CALLBACK pageProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (
        message == WM_COMMAND ||
        message == WM_HSCROLL ||
        message == WM_DRAWITEM
    ) {
        HWND parent = GetParent(window);

        if (parent) {
            return SendMessageW(
                parent,
                message,
                wParam,
                lParam
            );
        }
    }

    return DefWindowProcW(
        window,
        message,
        wParam,
        lParam
    );
}

void enablePageMessageForwarding(HWND page)
{
    SetWindowLongPtrW(
        page,
        GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(pageProcedure)
    );
}

void applyFont(HWND window, HFONT font)
{
    EnumChildWindows(
        window,
        [](HWND child, LPARAM param) -> BOOL {
            SendMessageW(
                child,
                WM_SETFONT,
                param,
                TRUE
            );
            return TRUE;
        },
        reinterpret_cast<LPARAM>(font)
    );
}

void setPageVisibility(WindowData& data)
{
    const HWND pages[] = {
        data.effectPage,
        data.systemPage,
        data.textPage,
        data.devicePage,
        data.settingsPage
    };

    for (int i = 0; i < 5; ++i) {
        ShowWindow(
            pages[i],
            i == data.tab ? SW_SHOW : SW_HIDE
        );
    }
}

void selectTab(WindowData& data, int tab)
{
    data.tab = std::clamp(tab, 0, 4);

    if (data.tabControl) {
        TabCtrl_SetCurSel(data.tabControl, data.tab);
    }

    setPageVisibility(data);
}

bool isUsablePaletteColor(int index)
{
    const Rgb& c =
        launchpadPalette(
            static_cast<std::uint8_t>(index)
        ).rgb;

    // Remove only colors that are effectively too dark to be useful
    // on the Launchpad. Keep medium-dark colors that are still visible.
    const double luminance =
        0.2126 * c.r
        + 0.7152 * c.g
        + 0.0722 * c.b;

    return luminance >= 0.12;
}

void createEffectPage(HWND window, WindowData& data)
{
    const auto& state = data.app->state();

    data.effectPage = CreateWindowW(
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        15,
        70,
        450,
        525,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    enablePageMessageForwarding(
        data.effectPage
    );

    label(
        data.effectPage,
        text(state.language, "midi"),
        5,
        8,
        45,
        22
    );

    data.portCombo = CreateWindowW(
        L"COMBOBOX",
        nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        55,
        5,
        330,
        180,
        data.effectPage,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(ID_PORT)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    button(
        data.effectPage,
        ID_REFRESH,
        text(state.language, "refresh"),
        5,
        38,
        92,
        30
    );

    button(
        data.effectPage,
        ID_CONNECT,
        text(state.language, "connect"),
        103,
        38,
        92,
        30
    );

    data.status = label(
        data.effectPage,
        text(state.language, "notconnected"),
        205,
        43,
        180,
        22,
        ID_STATUS
    );

    button(
        data.effectPage,
        ID_RAINBOW,
        text(state.language, "rainbow"),
        5,
        78,
        62,
        30
    );

    button(
        data.effectPage,
        ID_BREATHE,
        text(state.language, "breathe"),
        72,
        78,
        62,
        30
    );

    button(
        data.effectPage,
        ID_WAVE,
        text(state.language, "wave"),
        139,
        78,
        62,
        30
    );

    button(
        data.effectPage,
        ID_STARS,
        text(state.language, "stars"),
        206,
        78,
        62,
        30
    );

    button(
        data.effectPage,
        ID_SOLID,
        text(state.language, "solid"),
        273,
        78,
        62,
        30
    );

    label(
        data.effectPage,
        text(state.language, "monitor"),
        5,
        118,
        75,
        22
    );

    button(
        data.effectPage,
        ID_CPU,
        text(state.language, "cpu"),
        82,
        114,
        58,
        30
    );

    button(
        data.effectPage,
        ID_GPU,
        text(state.language, "gpu"),
        145,
        114,
        58,
        30
    );

    button(
        data.effectPage,
        ID_RAM,
        text(state.language, "ram"),
        208,
        114,
        58,
        30
    );

    button(
        data.effectPage,
        ID_TEMP,
        text(state.language, "temp"),
        271,
        114,
        58,
        30
    );

    label(
        data.effectPage,
        text(state.language, "brightness"),
        5,
        158,
        70,
        22
    );

    data.brightness = CreateWindowW(
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE,
        78,
        153,
        305,
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
        MAKELONG(5, 100)
    );

    SendMessageW(
        data.brightness,
        TBM_SETPOS,
        TRUE,
        state.brightness
    );

    label(
        data.effectPage,
        text(state.language, "speed"),
        5,
        198,
        70,
        22
    );

    data.speed = CreateWindowW(
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE,
        78,
        193,
        305,
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
        MAKELONG(1, 50)
    );

    SendMessageW(
        data.speed,
        TBM_SETPOS,
        TRUE,
        state.speed
    );

    label(
        data.effectPage,
        text(state.language, "palette"),
        5,
        238,
        230,
        22
    );

    const int startX = 5;
    const int startY = 266;
    const int cell = 22;
    const int gap = 2;

    int displayIndex = 0;

    for (int i = 0; i < 128; ++i) {
        if (!isUsablePaletteColor(i)) {
            continue;
        }

        const int col = displayIndex % 16;
        const int row = displayIndex / 16;
        ++displayIndex;

        HWND swatch = CreateWindowW(
            L"BUTTON",
            nullptr,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            startX + col * (cell + gap),
            startY + row * (cell + gap),
            cell,
            cell,
            data.effectPage,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(
                    ID_PALETTE_BASE + i
                )
            ),
            GetModuleHandleW(nullptr),
            nullptr
        );

        SendMessageW(
            swatch,
            WM_SETFONT,
            reinterpret_cast<WPARAM>(data.font),
            TRUE
        );
    }
}

void createPlaceholderPage(
    HWND window,
    WindowData& data,
    int index,
    const char* key)
{
    HWND* target = nullptr;

    if (index == 1) {
        target = &data.systemPage;
    }

    if (index == 2) {
        target = &data.textPage;
    }

    if (index == 3) {
        target = &data.devicePage;
    }

    if (index == 4) {
        target = &data.settingsPage;
    }

    *target = CreateWindowW(
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE,
        15,
        70,
        450,
        525,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    enablePageMessageForwarding(*target);

    if (index != 3) {
        label(
            *target,
            text(data.app->state().language, key),
            20,
            35,
            400,
            80
        );
    }
}

void createDevicePage(WindowData& data)
{
    const auto& state = data.app->state();

    label(
        data.devicePage,
        text(state.language, "midi"),
        20,
        35,
        70,
        22
    );

    label(
        data.devicePage,
        text(state.language, "preview_hint"),
        20,
        75,
        400,
        60
    );
}

void createControls(HWND window, WindowData& data)
{
    const auto& state = data.app->state();

    data.tabControl = CreateWindowW(
        WC_TABCONTROLW,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TABS,
        15,
        15,
        426,
        32,
        window,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(ID_TABS)
        ),
        GetModuleHandleW(nullptr),
        nullptr
    );

    const LPCWSTR tabLabels[] = {
        text(state.language, "tab_effects"),
        text(state.language, "tab_system"),
        text(state.language, "tab_text"),
        text(state.language, "tab_device"),
        text(state.language, "tab_settings")
    };

    for (int i = 0; i < 5; ++i) {
        TCITEMW item{};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPWSTR>(tabLabels[i]);
        TabCtrl_InsertItem(data.tabControl, i, &item);
    }

    createEffectPage(window, data);

    createPlaceholderPage(
        window,
        data,
        1,
        "placeholder_system"
    );

    createPlaceholderPage(
        window,
        data,
        2,
        "placeholder_text"
    );

    createPlaceholderPage(
        window,
        data,
        3,
        "placeholder_text"
    );

    createPlaceholderPage(
        window,
        data,
        4,
        "placeholder_settings"
    );

    createDevicePage(data);

    selectTab(data, 0);

    data.app->refreshMidiPorts(
        data.portCombo
    );
}

void drawPreview(
    HDC dc,
    const RECT& client,
    const Application& app,
    Language language,
    HFONT font)
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

    HFONT oldFont = nullptr;

    if (font) {
        oldFont = static_cast<HFONT>(
            SelectObject(dc, font)
        );
    }

    if (!app.state().effectRunning) {
        if (oldFont) {
            SelectObject(dc, oldFont);
        }
        return;
    }

    const std::wstring title =
        text(language, "preview");

    TextOutW(
        dc,
        panelX + 18,
        panelY + 15,
        title.c_str(),
        static_cast<int>(title.size())
    );

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
        static_cast<int>(hint.size())
    );

    if (oldFont) {
        SelectObject(dc, oldFont);
    }
}

} // namespace

bool MainWindow::registerClass(HINSTANCE instance)
{
    WNDCLASSW wc{};

    wc.lpfnWndProc =
        procedure;

    wc.hInstance =
        instance;

    wc.lpszClassName =
        L"LaunchpadRGBAmbientV04";

    wc.hCursor =
        LoadCursorW(
            nullptr,
            MAKEINTRESOURCEW(32512)
        );

    wc.hbrBackground =
        reinterpret_cast<HBRUSH>(
            COLOR_WINDOW + 1
        );

    return RegisterClassW(&wc) != 0;
}

HWND MainWindow::create(
    HINSTANCE instance,
    int showCommand,
    Application* app)
{
    HWND window = CreateWindowW(
        L"LaunchpadRGBAmbientV04",
        windowTitle(),
        WS_OVERLAPPED
            | WS_CAPTION
            | WS_SYSMENU
            | WS_MINIMIZEBOX
            | WS_CLIPCHILDREN,
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

LRESULT CALLBACK MainWindow::procedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    auto* data =
        reinterpret_cast<WindowData*>(
            GetWindowLongPtrW(
                window,
                GWLP_USERDATA
            )
        );

    switch (message) {

    case WM_NCCREATE: {
        auto* cs =
            reinterpret_cast<CREATESTRUCTW*>(
                lParam
            );

        auto* newData =
            new WindowData{};

        newData->app =
            reinterpret_cast<Application*>(
                cs->lpCreateParams
            );

        SetWindowLongPtrW(
            window,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(
                newData
            )
        );

        return TRUE;
    }

    case WM_CREATE:
        data->app->initialize(window);

        data->font =
            CreateFontW(
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
                L"Microsoft YaHei"
            );

        createControls(
            window,
            *data
        );

        applyFont(
            window,
            data->font
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
            1000,
            nullptr
        );

        return 0;

    case WM_DRAWITEM: {
        auto* dis =
            reinterpret_cast<DRAWITEMSTRUCT*>(
                lParam
            );

        if (
            dis
            && dis->CtlID >= ID_PALETTE_BASE
            && dis->CtlID <
                ID_PALETTE_BASE + 128
        ) {
            const int index =
                dis->CtlID
                - ID_PALETTE_BASE;

            const Rgb& c =
                launchpadPalette(
                    static_cast<std::uint8_t>(
                        index
                    )
                ).rgb;

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

            FillRect(
                dis->hDC,
                &dis->rcItem,
                brush
            );

            DeleteObject(brush);

            const bool selected =
                data->app->state().paletteIndex
                == index;

            HBRUSH border =
                CreateSolidBrush(
                    selected
                        ? RGB(0, 0, 0)
                        : RGB(160, 160, 160)
                );

            FrameRect(
                dis->hDC,
                &dis->rcItem,
                border
            );

            DeleteObject(border);

            if (selected) {
                RECT inner =
                    dis->rcItem;

                InflateRect(
                    &inner,
                    -3,
                    -3
                );

                FrameRect(
                    dis->hDC,
                    &inner,
                    static_cast<HBRUSH>(
                        GetStockObject(
                            WHITE_BRUSH
                        )
                    )
                );
            }

            return TRUE;
        }

        break;
    }

    case WM_NOTIFY: {
        auto* header =
            reinterpret_cast<NMHDR*>(lParam);

        if (
            header
            && header->idFrom == ID_TABS
            && header->code == TCN_SELCHANGE
        ) {
            const int tab =
                TabCtrl_GetCurSel(data->tabControl);

            selectTab(*data, tab);
            return 0;
        }

        break;
    }

    case WM_COMMAND:
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
                && LOWORD(wParam) <
                    ID_PALETTE_BASE + 128
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

        drawPreview(
            dc,
            client,
            *data->app,
            data->app->state().language,
            data->font
        );

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

        data->app->shutdown();

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
