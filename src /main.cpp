#define UNICODE
#define _UNICODE

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <pdh.h>

#include <cmath>
#include <cstring>
#include <cwchar>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "pdh.lib")


static constexpr wchar_t TITLE[] =
    L"Launchpad RGB Ambient v0.1";


/* ============================================================
   Control IDs
   ============================================================ */

enum
{
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


/* ============================================================
   Types
   ============================================================ */

enum class Language
{
    Chinese,
    English
};

enum class Effect
{
    Rainbow,
    Breathe,
    Wave,
    Stars,
    Solid
};


/* ============================================================
   Global state
   ============================================================ */

static Language language = Language::English;

static Effect currentEffect =
    Effect::Rainbow;

static HMIDIOUT midiDevice = nullptr;

static HWND mainWindow = nullptr;
static HWND portCombo = nullptr;
static HWND statusLabel = nullptr;
static HWND brightnessSlider = nullptr;
static HWND speedSlider = nullptr;

static bool effectRunning = false;

static bool cpuIndicator = false;
static bool gpuIndicator = false;
static bool ramIndicator = false;
static bool temperatureIndicator = false;

static double animationPhase = 0.0;
static double cpuUsage = 0.0;

static MEMORYSTATUSEX memoryStatus
{
    sizeof(MEMORYSTATUSEX)
};

static PDH_HQUERY cpuQuery = nullptr;
static PDH_HCOUNTER cpuCounter = nullptr;


/* ============================================================
   Language
   ============================================================ */

static void DetectLanguage()
{
    LANGID uiLanguage =
        GetUserDefaultUILanguage();

    if (PRIMARYLANGID(uiLanguage) == LANG_CHINESE)
    {
        language = Language::Chinese;
    }
    else
    {
        language = Language::English;
    }
}


static LPCWSTR Text(const char* key)
{
    const bool chinese =
        language == Language::Chinese;


    if (!strcmp(key, "title"))
        return TITLE;


    if (!strcmp(key, "midi"))
        return L"MIDI";


    if (!strcmp(key, "refresh"))
        return chinese ? L"刷新" : L"Refresh";


    if (!strcmp(key, "connect"))
        return chinese ? L"连接" : L"Connect";


    if (!strcmp(key, "rainbow"))
        return chinese ? L"彩虹" : L"Rainbow";


    if (!strcmp(key, "breathe"))
        return chinese ? L"呼吸" : L"Breathe";


    if (!strcmp(key, "wave"))
        return chinese ? L"波纹" : L"Wave";


    if (!strcmp(key, "stars"))
        return chinese ? L"星空" : L"Stars";


    if (!strcmp(key, "solid"))
        return chinese ? L"纯色" : L"Solid";


    if (!strcmp(key, "monitor"))
        return chinese ? L"状态指示" : L"Monitoring";


    if (!strcmp(key, "cpu"))
        return L"CPU";


    if (!strcmp(key, "gpu"))
        return L"GPU";


    if (!strcmp(key, "ram"))
        return chinese ? L"内存" : L"RAM";


    if (!strcmp(key, "temp"))
        return chinese ? L"温度" : L"Temp";


    if (!strcmp(key, "brightness"))
        return chinese ? L"亮度" : L"Brightness";


    if (!strcmp(key, "speed"))
        return chinese ? L"速度" : L"Speed";


    if (!strcmp(key, "connected"))
        return chinese ? L"已连接" : L"Connected";


    if (!strcmp(key, "notconnected"))
        return chinese ? L"未连接" : L"Not connected";


    if (!strcmp(key, "error"))
        return chinese
            ? L"MIDI 打开失败"
            : L"MIDI open failed";


    return L"";
}


/* ============================================================
   Launchpad helpers
   ============================================================ */

static BYTE ConvertColor(double value)
{
    if (value < 0.0)
        value = 0.0;

    if (value > 63.0)
        value = 63.0;

    return static_cast<BYTE>(
        value + 0.5);
}


static int LaunchpadLED(
    int x,
    int y)
{
    return 11 +
           (7 - y) * 10 +
           x;
}


/* ============================================================
   MIDI SysEx
   ============================================================ */

static bool SendSysEx(
    const BYTE* data,
    DWORD length)
{
    if (!midiDevice)
        return false;


    MIDIHDR header{};

    header.lpData =
        reinterpret_cast<LPSTR>(
            const_cast<BYTE*>(data));

    header.dwBufferLength =
        length;


    if (midiOutPrepareHeader(
            midiDevice,
            &header,
            sizeof(header))
        != MMSYSERR_NOERROR)
    {
        return false;
    }


    MMRESULT result =
        midiOutLongMsg(
            midiDevice,
            &header,
            sizeof(header));


    DWORD startTime =
        GetTickCount();


    while (!(header.dwFlags & MHDR_DONE))
    {
        if (GetTickCount() -
            startTime > 500)
        {
            break;
        }

        Sleep(1);
    }


    midiOutUnprepareHeader(
        midiDevice,
        &header,
        sizeof(header));


    return result ==
           MMSYSERR_NOERROR;
}


/* ============================================================
   Set one Launchpad RGB LED
   ============================================================ */

static void SetLED(
    int x,
    int y,
    BYTE red,
    BYTE green,
    BYTE blue)
{
    BYTE message[12] =
    {
        0xF0,
        0x00,
        0x20,
        0x29,
        0x02,
        0x18,
        0x0B,
        0x00,
        0x00,
        0x00,
        0x00,
        0xF7
    };


    message[7] =
        static_cast<BYTE>(
            LaunchpadLED(x, y));

    message[8] = red;
    message[9] = green;
    message[10] = blue;


    SendSysEx(
        message,
        sizeof(message));
}


/* ============================================================
   Clear Launchpad
   ============================================================ */

static void ClearLaunchpad()
{
    if (!midiDevice)
        return;


    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            SetLED(
                x,
                y,
                0,
                0,
                0);
        }
    }
}


/* ============================================================
   Close MIDI device
   ============================================================ */

static void CloseMidi()
{
    if (!midiDevice)
        return;


    ClearLaunchpad();


    midiOutReset(
        midiDevice);


    midiOutClose(
        midiDevice);


    midiDevice = nullptr;
}


/* ============================================================
   Refresh MIDI device list
   ============================================================ */

static void RefreshMidiPorts()
{
    SendMessageW(
        portCombo,
        CB_RESETCONTENT,
        0,
        0);


    UINT deviceCount =
        midiOutGetNumDevs();


    int preferredIndex = -1;


    for (UINT i = 0;
         i < deviceCount;
         ++i)
    {
        MIDIOUTCAPSW capabilities{};


        if (midiOutGetDevCapsW(
                i,
                &capabilities,
                sizeof(capabilities))
            != MMSYSERR_NOERROR)
        {
            continue;
        }


        int comboIndex =
            static_cast<int>(
                SendMessageW(
                    portCombo,
                    CB_ADDSTRING,
                    0,
                    reinterpret_cast<LPARAM>(
                        capabilities.szPname)));


        SendMessageW(
            portCombo,
            CB_SETITEMDATA,
            comboIndex,
            static_cast<LPARAM>(i));


        if (wcsstr(
                capabilities.szPname,
                L"Launchpad MK2"))
        {
            preferredIndex =
                comboIndex;
        }
    }


    LRESULT count =
        SendMessageW(
            portCombo,
            CB_GETCOUNT,
            0,
            0);


    if (count > 0)
    {
        SendMessageW(
            portCombo,
            CB_SETCURSEL,
            preferredIndex >= 0
                ? preferredIndex
                : 0,
            0);
    }
}


/* ============================================================
   Connect MIDI
   ============================================================ */

static bool ConnectMidi()
{
    CloseMidi();


    int selectedIndex =
        static_cast<int>(
            SendMessageW(
                portCombo,
                CB_GETCURSEL,
                0,
                0));


    if (selectedIndex < 0)
    {
        SetWindowTextW(
            statusLabel,
            Text("notconnected"));

        return false;
    }


    UINT deviceIndex =
        static_cast<UINT>(
            SendMessageW(
                portCombo,
                CB_GETITEMDATA,
                selectedIndex,
                0));


    if (midiOutOpen(
            &midiDevice,
            deviceIndex,
            0,
            0,
            CALLBACK_NULL)
        != MMSYSERR_NOERROR)
    {
        midiDevice = nullptr;


        SetWindowTextW(
            statusLabel,
            Text("error"));


        return false;
    }


    SetWindowTextW(
        statusLabel,
        Text("connected"));


    return true;
}


/* ============================================================
   HSV color conversion
   ============================================================ */

static void HSVtoRGB(
    double h,
    double s,
    double v,
    double& r,
    double& g,
    double& b)
{
    h = fmod(h, 1.0);


    if (h < 0.0)
        h += 1.0;


    double sector =
        floor(h * 6.0);


    double fraction =
        h * 6.0 - sector;


    double p =
        v * (1.0 - s);


    double q =
        v * (1.0 -
             fraction * s);


    double t =
        v * (1.0 -
             (1.0 - fraction) * s);


    switch (
        static_cast<int>(sector) % 6)
    {
        case 0:
            r = v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = v;
            b = p;
            break;

        case 2:
            r = p;
            g = v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = v;
            break;

        case 4:
            r = t;
            g = p;
            b = v;
            break;

        default:
            r = v;
            g = p;
            b = q;
            break;
    }
}


/* ============================================================
   Render effects
   ============================================================ */

static void RenderEffect()
{
    if (!midiDevice ||
        !effectRunning)
    {
        return;
    }


    int brightnessValue =
        static_cast<int>(
            SendMessageW(
                brightnessSlider,
                TBM_GETPOS,
                0,
                0));


    int speedValue =
        static_cast<int>(
            SendMessageW(
                speedSlider,
                TBM_GETPOS,
                0,
                0));


    double brightness =
        brightnessValue / 100.0;


    double effectSpeed =
        speedValue / 20.0;


    animationPhase +=
        0.035 *
        (0.2 + effectSpeed);


    for (int y = 0;
         y < 8;
         ++y)
    {
        for (int x = 0;
             x < 8;
             ++x)
        {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;


            /* ----------------------------------------
               Rainbow
               ---------------------------------------- */

            if (currentEffect ==
                Effect::Rainbow)
            {
                HSVtoRGB(
                    x / 8.0 +
                    y / 16.0 +
                    animationPhase * 0.08,
                    0.9,
                    brightness,
                    red,
                    green,
                    blue);
            }


            /* ----------------------------------------
               Breathe
               ---------------------------------------- */

            else if (
                currentEffect ==
                Effect::Breathe)
            {
                double value =
                    (0.5 +
                     0.5 *
                     sin(
                         animationPhase * 2.0))
                    * brightness;


                HSVtoRGB(
                    animationPhase * 0.02,
                    0.75,
                    value,
                    red,
                    green,
                    blue);
            }


            /* ----------------------------------------
               Wave
               ---------------------------------------- */

            else if (
                currentEffect ==
                Effect::Wave)
            {
                double distance =
                    sqrt(
                        (x - 3.5) *
                        (x - 3.5) +
                        (y - 3.5) *
                        (y - 3.5));


                double value =
                    (0.5 +
                     0.5 *
                     sin(
                         distance * 2.0 -
                         animationPhase * 5.0))
                    * brightness;


                red =
                    value;

                green =
                    value * 0.45;

                blue =
                    value * 0.10;
            }


            /* ----------------------------------------
               Stars
               ---------------------------------------- */

            else if (
                currentEffect ==
                Effect::Stars)
            {
                double randomValue =
                    sin(
                        x * 12.9898 +
                        y * 78.233 +
                        floor(
                            animationPhase * 1.5) *
                        37.7);


                double value =
                    (randomValue -
                     floor(randomValue))
                    * 0.85 *
                    brightness;


                bool star =
                    ((x * 17 +
                      y * 31 +
                      static_cast<int>(
                          animationPhase))
                     % 19) == 0;


                if (star)
                {
                    red = value;
                    green = value * 0.25;
                    blue = value;
                }
                else
                {
                    red = value * 0.02;
                    green = value * 0.02;
                    blue = value * 0.02;
                }
            }


            /* ----------------------------------------
               Solid
               ---------------------------------------- */

            else
            {
                red =
                    0.5 *
                    brightness;

                green =
                    0.05 *
                    brightness;

                blue =
                    0.12 *
                    brightness;
            }


            /* ----------------------------------------
               CPU indicator
               Top row
               ---------------------------------------- */

            if (cpuIndicator &&
                y == 0)
            {
                double value =
                    cpuUsage / 100.0;


                if (x <
                    static_cast<int>(
                        value * 8.0))
                {
                    red = 0.08;
                    green = 1.0 - value;
                    blue = 0.05;
                }
                else
                {
                    red *= 0.15;
                    green *= 0.15;
                    blue *= 0.15;
                }
            }


            /* ----------------------------------------
               RAM indicator
               Bottom row
               ---------------------------------------- */

            if (ramIndicator &&
                y == 7)
            {
                double value =
                    1.0 -
                    static_cast<double>(
                        memoryStatus.ullAvailPhys) /
                    static_cast<double>(
                        memoryStatus.ullTotalPhys);


                if (x <
                    static_cast<int>(
                        value * 8.0))
                {
                    red = 0.1;
                    green = 1.0 - value;
                    blue = 0.05;
                }
            }


            /* ----------------------------------------
               Temperature indicator
               ---------------------------------------- */

            if (temperatureIndicator &&
                x == 7 &&
                y == 7)
            {
                red = 1.0;
                green = 0.1;
                blue = 0.0;
            }


            SetLED(
                x,
                y,
                ConvertColor(red * 63.0),
                ConvertColor(green * 63.0),
                ConvertColor(blue * 63.0));
        }
    }
}


/* ============================================================
   Update system statistics
   ============================================================ */

static void UpdateStatistics()
{
    if (cpuQuery &&
        cpuCounter)
    {
        PDH_FMT_COUNTERVALUE value{};


        if (PdhCollectQueryData(
                cpuQuery)
            == ERROR_SUCCESS)
        {
            if (PdhGetFormattedCounterValue(
                    cpuCounter,
                    PDH_FMT_DOUBLE,
                    nullptr,
                    &value)
                == ERROR_SUCCESS)
            {
                cpuUsage =
                    value.doubleValue;
            }
        }
    }


    GlobalMemoryStatusEx(
        &memoryStatus);
}


/* ============================================================
   UI helpers
   ============================================================ */

static HFONT CreateUIFont()
{
    return CreateFontW(
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
        L"Microsoft YaHei UI");
}


static HWND CreateLabel(
    HWND parent,
    LPCWSTR text,
    int x,
    int y,
    int width,
    int height)
{
    return CreateWindowW(
        L"STATIC",
        text,
        WS_CHILD | WS_VISIBLE,
        x,
        y,
        width,
        height,
        parent,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);
}


static HWND CreateButton(
    HWND parent,
    int id,
    LPCWSTR text,
    int x,
    int y,
    int width,
    int height)
{
    return CreateWindowW(
        L"BUTTON",
        text,
        WS_CHILD |
        WS_VISIBLE |
        BS_PUSHBUTTON,
        x,
        y,
        width,
        height,
        parent,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr),
        nullptr);
}


/* ============================================================
   Window procedure
   ============================================================ */

static LRESULT CALLBACK WindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            mainWindow = window;


            DetectLanguage();


            SetWindowTextW(
                window,
                Text("title"));


            /* ----------------------------------------
               Title
               ---------------------------------------- */

            CreateLabel(
                window,
                Text("title"),
                18,
                12,
                380,
                30);


            /* ----------------------------------------
               MIDI
               ---------------------------------------- */

            CreateLabel(
                window,
                Text("midi"),
                18,
                50,
                40,
                22);


            portCombo =
                CreateWindowW(
                    L"COMBOBOX",
                    nullptr,
                    WS_CHILD |
                    WS_VISIBLE |
                    CBS_DROPDOWNLIST,
                    60,
                    47,
                    340,
                    180,
                    window,
                    reinterpret_cast<HMENU>(
                        static_cast<INT_PTR>(
                            ID_PORT)),
                    GetModuleHandleW(nullptr),
                    nullptr);


            CreateButton(
                window,
                ID_REFRESH,
                Text("refresh"),
                18,
                78,
                95,
                30);


            CreateButton(
                window,
                ID_CONNECT,
                Text("connect"),
                120,
                78,
                95,
                30);


            /* ----------------------------------------
               Effects
               ---------------------------------------- */

            CreateButton(
                window,
                ID_RAINBOW,
                Text("rainbow"),
                18,
                120,
                72,
                30);


            CreateButton(
                window,
                ID_BREATHE,
                Text("breathe"),
                96,
                120,
                72,
                30);


            CreateButton(
                window,
                ID_WAVE,
                Text("wave"),
                174,
                120,
                72,
                30);


            CreateButton(
                window,
                ID_STARS,
                Text("stars"),
                252,
                120,
                72,
                30);


            CreateButton(
                window,
                ID_SOLID,
                Text("solid"),
                330,
                120,
                72,
                30);


            /* ----------------------------------------
               Monitoring
               ---------------------------------------- */

            CreateLabel(
                window,
                Text("monitor"),
                18,
                160,
                80,
                22);


            CreateButton(
                window,
                ID_CPU,
                Text("cpu"),
                92,
                155,
                60,
                30);


            CreateButton(
                window,
                ID_GPU,
                Text("gpu"),
                158,
                155,
                60,
                30);


            CreateButton(
                window,
                ID_RAM,
                Text("ram"),
                224,
                155,
                60,
                30);


            CreateButton(
                window,
                ID_TEMP,
                Text("temp"),
                290,
                155,
                60,
                30);


            /* ----------------------------------------
               Brightness
               ---------------------------------------- */

            CreateLabel(
                window,
                Text("brightness"),
                18,
                198,
                70,
                22);


            brightnessSlider =
                CreateWindowW(
                    TRACKBAR_CLASSW,
                    nullptr,
                    WS_CHILD |
                    WS_VISIBLE,
                    80,
                    193,
                    320,
                    30,
                    window,
                    reinterpret_cast<HMENU>(
                        static_cast<INT_PTR>(
                            ID_BRIGHTNESS)),
                    GetModuleHandleW(nullptr),
                    nullptr);


            SendMessageW(
                brightnessSlider,
                TBM_SETRANGE,
                TRUE,
                MAKELONG(5, 100));


            SendMessageW(
                brightnessSlider,
                TBM_SETPOS,
                TRUE,
                70);


            /* ----------------------------------------
               Speed
               ---------------------------------------- */

            CreateLabel(
                window,
                Text("speed"),
                18,
                238,
                70,
                22);


            speedSlider =
                CreateWindowW(
                    TRACKBAR_CLASSW,
                    nullptr,
                    WS_CHILD |
                    WS_VISIBLE,
                    80,
                    233,
                    320,
                    30,
                    window,
                    reinterpret_cast<HMENU>(
                        static_cast<INT_PTR>(
                            ID_SPEED)),
                    GetModuleHandleW(nullptr),
                    nullptr);


            SendMessageW(
                speedSlider,
                TBM_SETRANGE,
                TRUE,
                MAKELONG(1, 50));


            SendMessageW(
                speedSlider,
                TBM_SETPOS,
                TRUE,
                20);


            /* ----------------------------------------
               Status
               ---------------------------------------- */

            statusLabel =
                CreateLabel(
                    window,
                    Text("notconnected"),
                    18,
                    280,
                    380,
                    24);


            /* ----------------------------------------
               MIDI initialization
               ---------------------------------------- */

            RefreshMidiPorts();


            /* ----------------------------------------
               CPU monitor
               ---------------------------------------- */

            PdhOpenQueryW(
                nullptr,
                0,
                &cpuQuery);


            if (cpuQuery)
            {
                PdhAddEnglishCounterW(
                    cpuQuery,
                    L"\\Processor(_Total)\\% Processor Time",
                    0,
                    &cpuCounter);


                PdhCollectQueryData(
                    cpuQuery);
            }


            /* ----------------------------------------
               Font
               ---------------------------------------- */

            HFONT font =
                CreateUIFont();


            EnumChildWindows(
                window,
                [](HWND child,
                   LPARAM parameter)
                -> BOOL
                {
                    SendMessageW(
                        child,
                        WM_SETFONT,
                        parameter,
                        TRUE);

                    return TRUE;
                },
                reinterpret_cast<LPARAM>(
                    font));


            SendMessageW(
                window,
                WM_SETFONT,
                reinterpret_cast<WPARAM>(
                    font),
                TRUE);


            /* ----------------------------------------
               Timers
               ---------------------------------------- */

            SetTimer(
                window,
                1,
                80,
                nullptr);


            SetTimer(
                window,
                2,
                1000,
                nullptr);


            return 0;
        }


        case WM_COMMAND:
        {
            switch (
                LOWORD(wParam))
            {
                case ID_REFRESH:
                {
                    RefreshMidiPorts();
                    break;
                }


                case ID_CONNECT:
                {
                    ConnectMidi();
                    break;
                }


                case ID_RAINBOW:
                {
                    currentEffect =
                        Effect::Rainbow;

                    effectRunning = true;
                    break;
                }


                case ID_BREATHE:
                {
                    currentEffect =
                        Effect::Breathe;

                    effectRunning = true;
                    break;
                }


                case ID_WAVE:
                {
                    currentEffect =
                        Effect::Wave;

                    effectRunning = true;
                    break;
                }


                case ID_STARS:
                {
                    currentEffect =
                        Effect::Stars;

                    effectRunning = true;
                    break;
                }


                case ID_SOLID:
                {
                    currentEffect =
                        Effect::Solid;

                    effectRunning = true;
                    break;
                }


                case ID_CPU:
                {
                    cpuIndicator =
                        !cpuIndicator;

                    break;
                }


                case ID_GPU:
                {
                    gpuIndicator =
                        !gpuIndicator;

                    break;
                }


                case ID_RAM:
                {
                    ramIndicator =
                        !ramIndicator;

                    break;
                }


                case ID_TEMP:
                {
                    temperatureIndicator =
                        !temperatureIndicator;

                    break;
                }
            }


            return 0;
        }


        case WM_TIMER:
        {
            if (wParam == 1)
            {
                RenderEffect();
            }
            else if (wParam == 2)
            {
                UpdateStatistics();
            }


            return 0;
        }


        case WM_DESTROY:
        {
            KillTimer(
                window,
                1);

            KillTimer(
                window,
                2);


            CloseMidi();


            if (cpuQuery)
            {
                PdhCloseQuery(
                    cpuQuery);

                cpuQuery = nullptr;
            }


            PostQuitMessage(
                0);


            return 0;
        }
    }


    return DefWindowProcW(
        window,
        message,
        wParam,
        lParam);
}


/* ============================================================
   Windows entry point
   ============================================================ */

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPWSTR,
    int showCommand)
{
    INITCOMMONCONTROLSEX controls
    {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_BAR_CLASSES
    };


    InitCommonControlsEx(
        &controls);


    WNDCLASSW windowClass{};


    windowClass.lpfnWndProc =
        WindowProcedure;


    windowClass.hInstance =
        instance;


    windowClass.lpszClassName =
        L"LaunchpadRGBAmbientV01";


    windowClass.hCursor =
        LoadCursorW(
            nullptr,
            IDC_ARROW);


    windowClass.hbrBackground =
        reinterpret_cast<HBRUSH>(
            COLOR_WINDOW + 1);


    RegisterClassW(
        &windowClass);


    HWND window =
        CreateWindowW(
            windowClass.lpszClassName,
            TITLE,
            WS_OVERLAPPED |
            WS_CAPTION |
            WS_SYSMENU |
            WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            440,
            350,
            nullptr,
            nullptr,
            instance,
            nullptr);


    if (!window)
        return 1;


    ShowWindow(
        window,
        showCommand);


    UpdateWindow(
        window);


    MSG message{};


    while (
        GetMessageW(
            &message,
            nullptr,
            0,
            0) > 0)
    {
        TranslateMessage(
            &message);

        DispatchMessageW(
            &message);
    }


    return 0;
}
