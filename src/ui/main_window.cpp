#include "main_window.h"
#include "../app/application.h"
#include "../core/color.h"
#include "../localization/localization.h"

#include <commctrl.h>
#include <algorithm>
#include <string>
#include <vector>
#include <cwctype>

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
    ID_STOP = 140,
    ID_TEXT_INPUT,
    ID_TEXT_START,
    ID_TEXT_Y,
    ID_TEXT_SPEED,
    ID_TEXT_ANIMATION,
    ID_TEXT_RED,
    ID_TEXT_GREEN,
    ID_TEXT_BLUE,
    ID_TRAY_ON_CLOSE,
    ID_TAB_EFFECTS = 200,
    ID_TAB_SYSTEM,
    ID_TAB_TEXT,
    ID_TAB_SETTINGS,
    ID_TABS = 205,
    ID_PLACEHOLDER = 250,
    ID_PALETTE_BASE = 400,
    ID_TRAY_OPEN = 9001,
    ID_TRAY_EXIT = 9002,
    WM_TRAYICON = WM_APP + 10
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
    HWND settingsPage = nullptr;
    HFONT font = nullptr;
    HWND tabControl = nullptr;
    int tab = 0;
    HWND textInput = nullptr;
    HWND textHint = nullptr;
    HWND textY = nullptr;
    HWND textSpeed = nullptr;
    HWND textAnimation = nullptr;
    HWND textRed = nullptr;
    HWND textGreen = nullptr;
    HWND textBlue = nullptr;
    HWND trayCheck = nullptr;
    bool trayIconVisible = false;
    bool forceExit = false;
};

void updatePaletteVisibility(WindowData& data);

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
    // Each page owns the area where its controls are drawn.
    // Explicitly paint the page background so pixels left by a
    // previous layout/page cannot remain visible underneath the
    // current controls.
    if (message == WM_ERASEBKGND) {
        RECT client{};
        GetClientRect(window, &client);

        HBRUSH brush = static_cast<HBRUSH>(
            GetStockObject(WHITE_BRUSH)
        );

        FillRect(
            reinterpret_cast<HDC>(wParam),
            &client,
            brush
        );

        return 1;
    }

    if (message == WM_PAINT) {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(window, &ps);

        HBRUSH brush = static_cast<HBRUSH>(
            GetStockObject(WHITE_BRUSH)
        );

        FillRect(dc, &ps.rcPaint, brush);
        EndPaint(window, &ps);

        return 0;
    }

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
        data.settingsPage
    };

    for (int i = 0; i < 4; ++i) {
        ShowWindow(
            pages[i],
            i == data.tab ? SW_SHOW : SW_HIDE
        );
    }

    // The pages are white rectangles, so repaint the newly visible page
    // itself. This avoids relying on the parent window, which uses
    // WS_CLIPCHILDREN and therefore does not paint beneath child pages.
    HWND page = nullptr;

    switch (data.tab) {
    case 0: page = data.effectPage; break;
    case 1: page = data.systemPage; break;
    case 2: page = data.textPage; break;
    case 3: page = data.settingsPage; break;
    }

    if (page) {
        RedrawWindow(
            page,
            nullptr,
            nullptr,
            RDW_ERASE
                | RDW_INVALIDATE
                | RDW_ALLCHILDREN
                | RDW_UPDATENOW
        );
    }
}

void selectTab(WindowData& data, int tab)
{
    data.tab = std::clamp(tab, 0, 3);

    if (data.tabControl) {
        TabCtrl_SetCurSel(data.tabControl, data.tab);
    }

    setPageVisibility(data);
    if (data.effectPage) updatePaletteVisibility(data);
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

    return luminance >= 0.18;
}


void updatePaletteVisibility(WindowData& data)
{
    const bool visible = data.app->state().effect != Effect::Rainbow &&
                         data.app->state().effect != Effect::Text;

    if (HWND h = GetDlgItem(data.effectPage, ID_PALETTE_BASE - 1)) {
        ShowWindow(h, visible ? SW_SHOW : SW_HIDE);
    }
    if (HWND h = GetDlgItem(data.effectPage, ID_PALETTE_BASE - 2)) {
        ShowWindow(h, visible ? SW_SHOW : SW_HIDE);
    }

    EnumChildWindows(data.effectPage, [](HWND child, LPARAM param) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(param);
        const int id = static_cast<int>(GetDlgCtrlID(child));
        if (id >= ID_PALETTE_BASE && id < ID_PALETTE_BASE + 128) {
            ShowWindow(child, data->app->state().effect == Effect::Rainbow ||
                               data->app->state().effect == Effect::Text
                           ? SW_HIDE : SW_SHOW);
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&data));
}

void createEffectPage(HWND window, WindowData& data)
{
    const auto& state = data.app->state();

    data.effectPage = CreateWindowW(
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        15,
        105,
        450,
        490,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    enablePageMessageForwarding(
        data.effectPage
    );

    button(
        data.effectPage,
        ID_RAINBOW,
        text(state.language, "rainbow"),
        5,
        8,
        62,
        30
    );

    button(
        data.effectPage,
        ID_BREATHE,
        text(state.language, "breathe"),
        72,
        8,
        62,
        30
    );

    button(
        data.effectPage,
        ID_WAVE,
        text(state.language, "wave"),
        139,
        8,
        62,
        30
    );

    button(
        data.effectPage,
        ID_STARS,
        text(state.language, "stars"),
        206,
        8,
        62,
        30
    );

    button(
        data.effectPage,
        ID_SOLID,
        text(state.language, "solid"),
        273,
        8,
        62,
        30
    );

    button(
        data.effectPage,
        ID_STOP,
        text(state.language, "stop"),
        340,
        8,
        62,
        30
    );

    label(
        data.effectPage,
        text(state.language, "brightness"),
        5,
        88,
        70,
        22
    );

    data.brightness = CreateWindowW(
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE,
        78,
        83,
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
        128,
        70,
        22
    );

    data.speed = CreateWindowW(
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE,
        78,
        123,
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
        168,
        180,
        22,
        ID_PALETTE_BASE - 1
    );

    label(
        data.effectPage,
        text(state.language, "palette_note"),
        190,
        168,
        255,
        22,
        ID_PALETTE_BASE - 2
    );

    const int startX = 5;
    const int startY = 196;
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
        target = &data.settingsPage;
    }

    *target = CreateWindowW(
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        15,
        105,
        450,
        490,
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

void createSystemPage(HWND window, WindowData& data)
{
    const auto& state = data.app->state();

    data.systemPage = CreateWindowW(
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        15,
        105,
        450,
        490,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    enablePageMessageForwarding(data.systemPage);

    label(
        data.systemPage,
        text(state.language, "monitor"),
        5,
        8,
        75,
        22
    );

    button(data.systemPage, ID_CPU, text(state.language, "cpu"), 82, 4, 58, 30);
    button(data.systemPage, ID_GPU, text(state.language, "gpu"), 145, 4, 58, 30);
    button(data.systemPage, ID_RAM, text(state.language, "ram"), 208, 4, 58, 30);
    button(data.systemPage, ID_TEMP, text(state.language, "temp"), 271, 4, 58, 30);
    button(data.systemPage, ID_STOP, text(state.language, "stop"), 335, 4, 62, 30);
}


void createTextPage(HWND window, WindowData& data)
{
    const auto& state = data.app->state();
    data.textPage = CreateWindowW(L"STATIC", nullptr,
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        15, 105, 450, 490, window, nullptr, GetModuleHandleW(nullptr), nullptr);
    enablePageMessageForwarding(data.textPage);

    label(data.textPage, text(state.language, "text_input"), 5, 12, 100, 22);
    data.textInput = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        5, 42, 300, 28, data.textPage,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_INPUT)),
        GetModuleHandleW(nullptr), nullptr);
    data.textHint = label(data.textPage, text(state.language, "text_hint"), 5, 75, 320, 22);
    button(data.textPage, ID_TEXT_START, text(state.language, "text_start"), 315, 41, 60, 30);
    button(data.textPage, ID_STOP, text(state.language, "stop"), 382, 41, 60, 30);

    label(data.textPage, text(state.language, "text_position"), 5, 112, 90, 22);
    label(data.textPage, L"Y", 100, 112, 18, 22);
    data.textY = CreateWindowW(TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE, 118, 107, 300, 30, data.textPage, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_Y)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.textY, TBM_SETRANGE, TRUE, MAKELONG(-7, 7));
    SendMessageW(data.textY, TBM_SETPOS, TRUE, state.textY);

    label(data.textPage, text(state.language, "text_animation"), 155, 153, 75, 22);
    data.textAnimation = CreateWindowW(L"COMBOBOX", nullptr, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 230, 150, 212, 140, data.textPage, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_ANIMATION)), GetModuleHandleW(nullptr), nullptr);
    const LPCWSTR animations[] = { text(state.language, "text_static"), text(state.language, "text_scroll"), text(state.language, "text_fade_in"), text(state.language, "text_fade_out"), text(state.language, "text_fade_in_out"), text(state.language, "text_blink") };
    for (auto value : animations) SendMessageW(data.textAnimation, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(value));
    SendMessageW(data.textAnimation, CB_SETCURSEL, state.textAnimation, 0);

    label(data.textPage, text(state.language, "text_speed"), 5, 195, 90, 22);
    data.textSpeed = CreateWindowW(TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE, 100, 190, 300, 30, data.textPage, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_SPEED)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.textSpeed, TBM_SETRANGE, TRUE, MAKELONG(1, 50));
    SendMessageW(data.textSpeed, TBM_SETPOS, TRUE, state.textSpeed);

    label(data.textPage, text(state.language, "text_color"), 5, 236, 90, 22);
    label(data.textPage, text(state.language, "text_red"), 100, 236, 18, 22);
    data.textRed = CreateWindowW(TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE, 120, 231, 300, 30, data.textPage, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_RED)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.textRed, TBM_SETRANGE, TRUE, MAKELONG(0, 63)); SendMessageW(data.textRed, TBM_SETPOS, TRUE, state.textRed);
    label(data.textPage, text(state.language, "text_green"), 100, 271, 18, 22);
    data.textGreen = CreateWindowW(TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE, 120, 266, 300, 30, data.textPage, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_GREEN)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.textGreen, TBM_SETRANGE, TRUE, MAKELONG(0, 63)); SendMessageW(data.textGreen, TBM_SETPOS, TRUE, state.textGreen);
    label(data.textPage, text(state.language, "text_blue"), 100, 306, 18, 22);
    data.textBlue = CreateWindowW(TRACKBAR_CLASSW, nullptr, WS_CHILD | WS_VISIBLE, 120, 301, 300, 30, data.textPage, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TEXT_BLUE)), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.textBlue, TBM_SETRANGE, TRUE, MAKELONG(0, 63)); SendMessageW(data.textBlue, TBM_SETPOS, TRUE, state.textBlue);
}

void createSettingsPage(HWND window, WindowData& data)
{
    const auto& state = data.app->state();
    data.settingsPage = CreateWindowW(L"STATIC", nullptr,
        WS_CHILD | WS_VISIBLE | SS_WHITERECT,
        15, 105, 450, 490, window, nullptr, GetModuleHandleW(nullptr), nullptr);
    enablePageMessageForwarding(data.settingsPage);

    data.trayCheck = CreateWindowW(
        L"BUTTON", text(state.language, "tray_on_close"),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        5, 15, 410, 26, data.settingsPage,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TRAY_ON_CLOSE)),
        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(data.trayCheck, BM_SETCHECK,
        state.trayOnClose ? BST_CHECKED : BST_UNCHECKED, 0);
    label(data.settingsPage, text(state.language, "settings_note"),
        25, 48, 400, 45);
}

void createControls(HWND window, WindowData& data)
{
    const auto& state = data.app->state();

    // Device selection is part of the main window rather than a tab,
    // so it remains visible while switching between pages.
    label(
        window,
        text(state.language, "midi"),
        15,
        12,
        45,
        22
    );

    data.portCombo = CreateWindowW(
        L"COMBOBOX",
        nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        60,
        9,
        300,
        180,
        window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_PORT)),
        GetModuleHandleW(nullptr),
        nullptr
    );

    button(
        window,
        ID_REFRESH,
        text(state.language, "refresh"),
        365,
        7,
        78,
        28
    );

    button(
        window,
        ID_CONNECT,
        text(state.language, "connect"),
        448,
        7,
        78,
        28
    );

    data.status = label(
        window,
        text(state.language, "notconnected"),
        532,
        11,
        190,
        22,
        ID_STATUS
    );

    data.tabControl = CreateWindowW(
        WC_TABCONTROLW,
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TABS,
        15,
        48,
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
        text(state.language, "tab_settings")
    };

    for (int i = 0; i < 4; ++i) {
        TCITEMW item{};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<LPWSTR>(tabLabels[i]);
        TabCtrl_InsertItem(data.tabControl, i, &item);
    }

    createEffectPage(window, data);

    createSystemPage(window, data);

    createTextPage(window, data);
    createSettingsPage(window, data);

    selectTab(data, 0);
    updatePaletteVisibility(data);

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
    const int panelY = 105;
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


void addTrayIcon(HWND window, WindowData& data)
{
    if (data.trayIconVisible) return;
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = window;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    lstrcpynW(nid.szTip, L"Launchpad RGB Ambient", ARRAYSIZE(nid.szTip));
    if (Shell_NotifyIconW(NIM_ADD, &nid)) data.trayIconVisible = true;
}

void removeTrayIcon(HWND window, WindowData& data)
{
    if (!data.trayIconVisible) return;
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = window;
    nid.uID = 1;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    data.trayIconVisible = false;
}

void showTrayMenu(HWND window)
{
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_TRAY_OPEN, L"打开");
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT, L"退出");
    POINT point{};
    GetCursorPos(&point);
    SetForegroundWindow(window);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, point.x, point.y, 0, window, nullptr);
    DestroyMenu(menu);
}

bool MainWindow::registerClass(HINSTANCE instance)
{
    WNDCLASSW wc{};

    wc.lpfnWndProc =
        procedure;

    wc.hInstance =
        instance;

    wc.lpszClassName =
        L"LaunchpadRGBAmbientV05";

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
        L"LaunchpadRGBAmbientV05",
        L"Launchpad RGB Ambient - v0.5",
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

    // Set the caption explicitly as well as passing it to CreateWindowW.
    // This keeps the title stable even if another initialization path
    // changes the window text later.
    SetWindowTextW(
        window,
        L"Launchpad RGB Ambient - v0.5"
    );

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
        SetTimer(
            window,
            3,
            100,
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

        case ID_TAB_SETTINGS:
            selectTab(*data, 3);
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
            updatePaletteVisibility(*data);
            break;

        case ID_BREATHE:
            data->app->setEffect(
                Effect::Breathe
            );
            updatePaletteVisibility(*data);
            break;

        case ID_WAVE:
            data->app->setEffect(
                Effect::Wave
            );
            updatePaletteVisibility(*data);
            break;

        case ID_STARS:
            data->app->setEffect(
                Effect::Stars
            );
            updatePaletteVisibility(*data);
            break;

        case ID_SOLID:
            data->app->setEffect(
                Effect::Solid
            );
            updatePaletteVisibility(*data);
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

        case ID_STOP:
            data->app->stopAll();
            break;

        case ID_TEXT_START: {
            wchar_t buffer[256]{};
            GetWindowTextW(data->textInput, buffer, ARRAYSIZE(buffer));
            std::wstring value(buffer);
            bool valid = !value.empty();
            for (wchar_t ch : value) {
                if (!((ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z'))) {
                    valid = false;
                    break;
                }
            }
            if (!valid) {
                SetWindowTextW(data->textHint, text(data->app->state().language, "text_invalid"));
                break;
            }
            SetWindowTextW(data->textHint, text(data->app->state().language, "text_hint"));
            data->app->state().animationPhase = 0.0;
            data->app->startText(value);
            break;
        }

        case ID_TEXT_ANIMATION:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                data->app->state().textAnimation = static_cast<int>(SendMessageW(data->textAnimation, CB_GETCURSEL, 0, 0));
                data->app->state().animationPhase = 0.0;
                data->app->state().textOffset = -9.0;
            }
            break;

        case ID_TRAY_ON_CLOSE:
            data->app->state().trayOnClose =
                SendMessageW(data->trayCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;

        case ID_TRAY_OPEN:
            removeTrayIcon(window, *data);
            ShowWindow(window, SW_SHOW);
            ShowWindow(window, SW_RESTORE);
            SetForegroundWindow(window);
            break;

        case ID_TRAY_EXIT:
            data->forceExit = true;
            removeTrayIcon(window, *data);
            DestroyWindow(window);
            break;

        default:
            if (
                LOWORD(wParam) >= ID_PALETTE_BASE
                && LOWORD(wParam) <
                    ID_PALETTE_BASE + 128
            ) {
                const int oldPaletteIndex =
                    data->app->state().paletteIndex;
                const int newPaletteIndex =
                    LOWORD(wParam) - ID_PALETTE_BASE;

                data->app->setPaletteColor(
                    newPaletteIndex
                );

                // Only redraw the old and new swatches. Invalidating the
                // parent is not enough because WS_CLIPCHILDREN prevents
                // the parent from repainting these child controls.
                HWND oldSwatch =
                    GetDlgItem(
                        data->effectPage,
                        ID_PALETTE_BASE + oldPaletteIndex
                    );
                HWND newSwatch =
                    GetDlgItem(
                        data->effectPage,
                        ID_PALETTE_BASE + newPaletteIndex
                    );

                if (oldSwatch) {
                    InvalidateRect(oldSwatch, nullptr, FALSE);
                    UpdateWindow(oldSwatch);
                }

                if (newSwatch && newSwatch != oldSwatch) {
                    InvalidateRect(newSwatch, nullptr, FALSE);
                    UpdateWindow(newSwatch);
                }

                // Remove keyboard focus from the palette buttons without
                // using the focus state as a visual selection indicator.
                SetFocus(data->effectPage);
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
        else if (reinterpret_cast<HWND>(lParam) == data->speed) {
            data->app->state().speed = static_cast<int>(SendMessageW(data->speed, TBM_GETPOS, 0, 0));
        }
        else if (reinterpret_cast<HWND>(lParam) == data->textY) {
            data->app->state().textY = static_cast<int>(SendMessageW(data->textY, TBM_GETPOS, 0, 0));
        }
        else if (reinterpret_cast<HWND>(lParam) == data->textSpeed) {
            data->app->state().textSpeed = static_cast<int>(SendMessageW(data->textSpeed, TBM_GETPOS, 0, 0));
        }
        else if (reinterpret_cast<HWND>(lParam) == data->textRed) {
            data->app->state().textRed = static_cast<int>(SendMessageW(data->textRed, TBM_GETPOS, 0, 0));
        }
        else if (reinterpret_cast<HWND>(lParam) == data->textGreen) {
            data->app->state().textGreen = static_cast<int>(SendMessageW(data->textGreen, TBM_GETPOS, 0, 0));
        }
        else if (reinterpret_cast<HWND>(lParam) == data->textBlue) {
            data->app->state().textBlue = static_cast<int>(SendMessageW(data->textBlue, TBM_GETPOS, 0, 0));
        }

        return 0;

    case WM_TIMER:
    if (wParam == 1) {
        data->app->render();
    }

    if (wParam == 2) {
        data->app->updateStatistics();
    }

    if (wParam == 3) {
        data->app->syncMidiPorts(
            data->portCombo,
            data->status
        );
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

    case WM_CLOSE:
        if (data && data->app->state().trayOnClose && !data->forceExit) {
            addTrayIcon(window, *data);
            ShowWindow(window, SW_HIDE);
            return 0;
        }
        DestroyWindow(window);
        return 0;

    case WM_TRAYICON:
        if (data && (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP)) {
            removeTrayIcon(window, *data);
            ShowWindow(window, SW_SHOW);
            ShowWindow(window, SW_RESTORE);
            SetForegroundWindow(window);
        } else if (data && (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)) {
            showTrayMenu(window);
        }
        return 0;

    case WM_DESTROY:
        if (data) removeTrayIcon(window, *data);
        KillTimer(
    window,
    1
);

KillTimer(
    window,
    2
);

KillTimer(
    window,
    3
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
