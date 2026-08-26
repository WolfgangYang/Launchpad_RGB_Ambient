#define UNICODE
#define _UNICODE

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <pdh.h>
#include <math.h>
#include <string.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "pdh.lib")

/* ============================================================
   Launchpad RGB Ambient
   Target: Novation Launchpad MK2

   Language:
   - Chinese Windows -> Simplified Chinese
   - Other Windows   -> English

   ============================================================ */

#define T_FRAME     1
#define T_STATS     2

#define C_PORT      100
#define C_REFRESH   101
#define C_CONNECT   102

#define C_RAINBOW   110
#define C_BREATHE   111
#define C_WAVE      112
#define C_STARS     113
#define C_SOLID     114

#define C_CPU       120
#define C_GPU       121
#define C_RAM       122
#define C_TEMP      123

#define C_BRIGHT    130
#define C_SPEED     131
#define C_STATUS    140


/* ============================================================
   Language
   ============================================================ */

enum
{
    LANG_ZH = 0,
    LANG_EN = 1
};

static int actualLang = LANG_EN;


/*
   Detect Windows UI language.

   This uses GetUserDefaultUILanguage(), rather than the
   computer's geographic region or currency settings.
*/
static void DetectLanguage(void)
{
    LANGID id = GetUserDefaultUILanguage();

    if (PRIMARYLANGID(id) == LANG_CHINESE)
        actualLang = LANG_ZH;
    else
        actualLang = LANG_EN;
}


/* Translation table */

static LPCWSTR TXT(const char *key)
{
    if (actualLang == LANG_ZH)
    {
        if (!strcmp(key, "title"))
            return L"Launchpad RGB Ambient";

        if (!strcmp(key, "midi"))
            return L"MIDI";

        if (!strcmp(key, "refresh"))
            return L"刷新";

        if (!strcmp(key, "connect"))
            return L"连接";

        if (!strcmp(key, "rainbow"))
            return L"彩虹";

        if (!strcmp(key, "breathe"))
            return L"呼吸";

        if (!strcmp(key, "wave"))
            return L"波纹";

        if (!strcmp(key, "stars"))
            return L"星空";

        if (!strcmp(key, "solid"))
            return L"纯色";

        if (!strcmp(key, "status"))
            return L"状态指示";

        if (!strcmp(key, "cpu"))
            return L"CPU";

        if (!strcmp(key, "gpu"))
            return L"GPU";

        if (!strcmp(key, "ram"))
            return L"内存";

        if (!strcmp(key, "temp"))
            return L"温度";

        if (!strcmp(key, "brightness"))
            return L"亮度";

        if (!strcmp(key, "speed"))
            return L"速度";

        if (!strcmp(key, "connected"))
            return L"已连接";

        if (!strcmp(key, "not_connected"))
            return L"未连接";

        if (!strcmp(key, "midi_failed"))
            return L"MIDI 打开失败";
    }
    else
    {
        if (!strcmp(key, "title"))
            return L"Launchpad RGB Ambient";

        if (!strcmp(key, "midi"))
            return L"MIDI";

        if (!strcmp(key, "refresh"))
            return L"Refresh";

        if (!strcmp(key, "connect"))
            return L"Connect";

        if (!strcmp(key, "rainbow"))
            return L"Rainbow";

        if (!strcmp(key, "breathe"))
            return L"Breathe";

        if (!strcmp(key, "wave"))
            return L"Wave";

        if (!strcmp(key, "stars"))
            return L"Stars";

        if (!strcmp(key, "solid"))
            return L"Solid";

        if (!strcmp(key, "status"))
            return L"Monitoring";

        if (!strcmp(key, "cpu"))
            return L"CPU";

        if (!strcmp(key, "gpu"))
            return L"GPU";

        if (!strcmp(key, "ram"))
            return L"RAM";

        if (!strcmp(key, "temp"))
            return L"Temp";

        if (!strcmp(key, "brightness"))
            return L"Brightness";

        if (!strcmp(key, "speed"))
            return L"Speed";

        if (!strcmp(key, "connected"))
            return L"Connected";

        if (!strcmp(key, "not_connected"))
            return L"Not connected";

        if (!strcmp(key, "midi_failed"))
            return L"MIDI open failed";
    }

    return L"";
}


/* ============================================================
   Global state
   ============================================================ */

static HMIDIOUT midi = NULL;

static HWND mainWindow;
static HWND ports;
static HWND statusText;
static HWND brightnessSlider;
static HWND speedSlider;

static int running = 0;
static int effectMode = 0;

static BOOL cpuEnabled = FALSE;
static BOOL gpuEnabled = FALSE;
static BOOL ramEnabled = FALSE;
static BOOL tempEnabled = FALSE;

static double phase = 0.0;


/* CPU */

static PDH_HQUERY cpuQuery = NULL;
static PDH_HCOUNTER cpuCounter = NULL;
static double cpuUsage = 0.0;


/* RAM */

static MEMORYSTATUSEX memoryStatus =
{
    sizeof(MEMORYSTATUSEX)
};


/* ============================================================
   Utility
   ============================================================ */

static BYTE Clamp63(double value)
{
    if (value < 0.0)
        value = 0.0;

    if (value > 63.0)
        value = 63.0;

    return (BYTE)(value + 0.5);
}


/* ============================================================
   Launchpad MK2 MIDI / SysEx
   ============================================================ */


/*
   Launchpad MK2 LED numbering.

   The central 8x8 grid uses:
       11 ... 18
       21 ... 28
       ...
       71 ... 78
*/
static UINT LaunchpadLED(int x, int y)
{
    return 11 + (7 - y) * 10 + x;
}


/*
   Send a SysEx packet through Windows MIDI.
*/
static BOOL SendSysEx(BYTE *data, DWORD length)
{
    if (!midi)
        return FALSE;

    MIDIHDR header;
    ZeroMemory(&header, sizeof(header));

    header.lpData = (LPSTR)data;
    header.dwBufferLength = length;

    if (midiOutPrepareHeader(
            midi,
            &header,
            sizeof(header)) != MMSYSERR_NOERROR)
    {
        return FALSE;
    }

    MMRESULT result =
        midiOutLongMsg(
            midi,
            &header,
            sizeof(header));

    DWORD start = GetTickCount();

    while (!(header.dwFlags & MHDR_DONE))
    {
        if (GetTickCount() - start > 500)
            break;

        Sleep(1);
    }

    midiOutUnprepareHeader(
        midi,
        &header,
        sizeof(header));

    return result == MMSYSERR_NOERROR;
}


/*
   Set one RGB pad.

   Launchpad MK2 RGB:
       R = 0..63
       G = 0..63
       B = 0..63
*/
static void SetRGB(
    int x,
    int y,
    BYTE r,
    BYTE g,
    BYTE b)
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

    message[7] = (BYTE)LaunchpadLED(x, y);
    message[8] = r;
    message[9] = g;
    message[10] = b;

    SendSysEx(message, sizeof(message));
}


/*
   Turn every central pad off.
*/
static void ClearLaunchpad(void)
{
    if (!midi)
        return;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            SetRGB(x, y, 0, 0, 0);
        }
    }
}


/* ============================================================
   MIDI device handling
   ============================================================ */

static void CloseMIDI(void)
{
    if (!midi)
        return;

    ClearLaunchpad();

    midiOutReset(midi);
    midiOutClose(midi);

    midi = NULL;
}


/*
   Enumerate MIDI output devices.
*/
static void RefreshPorts(void)
{
    SendMessageW(
        ports,
        CB_RESETCONTENT,
        0,
        0);

    UINT deviceCount =
        midiOutGetNumDevs();

    int launchpadIndex = -1;

    for (UINT i = 0; i < deviceCount; i++)
    {
        MIDIOUTCAPSW caps;

        if (midiOutGetDevCapsW(
                i,
                &caps,
                sizeof(caps)) != MMSYSERR_NOERROR)
        {
            continue;
        }

        int index =
            (int)SendMessageW(
                ports,
                CB_ADDSTRING,
                0,
                (LPARAM)caps.szPname);

        SendMessageW(
            ports,
            CB_SETITEMDATA,
            index,
            i);

        /*
           Automatically prefer Launchpad MK2.
        */
        if (wcsstr(
                caps.szPname,
                L"Launchpad MK2") != NULL)
        {
            launchpadIndex = index;
        }
    }

    int count =
        (int)SendMessageW(
            ports,
            CB_GETCOUNT,
            0,
            0);

    if (count > 0)
    {
        SendMessageW(
            ports,
            CB_SETCURSEL,
            launchpadIndex >= 0
                ? launchpadIndex
                : 0,
            0);
    }
}


/*
   Connect selected MIDI output.
*/
static BOOL ConnectMIDI(void)
{
    CloseMIDI();

    int index =
        (int)SendMessageW(
            ports,
            CB_GETCURSEL,
            0,
            0);

    if (index < 0)
    {
        SetWindowTextW(
            statusText,
            TXT("not_connected"));

        return FALSE;
    }

    UINT deviceID =
        (UINT)SendMessageW(
            ports,
            CB_GETITEMDATA,
            index,
            0);

    MMRESULT result =
        midiOutOpen(
            &midi,
            deviceID,
            0,
            0,
            CALLBACK_NULL);

    if (result != MMSYSERR_NOERROR)
    {
        midi = NULL;

        SetWindowTextW(
            statusText,
            TXT("midi_failed"));

        return FALSE;
    }

    SetWindowTextW(
        statusText,
        TXT("connected"));

    return TRUE;
}


/* ============================================================
   HSV -> RGB
   ============================================================ */

static void HSVtoRGB(
    double h,
    double s,
    double v,
    double *r,
    double *g,
    double *b)
{
    h = fmod(h, 1.0);

    if (h < 0)
        h += 1.0;

    double sector = floor(h * 6.0);
    double fraction = h * 6.0 - sector;

    double p = v * (1.0 - s);
    double q = v * (1.0 - fraction * s);
    double t = v * (1.0 - (1.0 - fraction) * s);

    switch ((int)sector % 6)
    {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;

        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;

        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;

        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;

        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;

        default:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}


/* ============================================================
   Effects
   ============================================================ */

static void RenderFrame(void)
{
    if (!midi || !running)
        return;

    double brightness =
        SendMessageW(
            brightnessSlider,
            TBM_GETPOS,
            0,
            0) / 100.0;

    double speed =
        SendMessageW(
            speedSlider,
            TBM_GETPOS,
            0,
            0) / 20.0;

    phase +=
        0.035 *
        (0.2 + speed);

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;


            /*
               0 = Rainbow
            */
            if (effectMode == 0)
            {
                HSVtoRGB(
                    x / 8.0 +
                    y / 16.0 +
                    phase * 0.08,
                    0.9,
                    brightness,
                    &r,
                    &g,
                    &b);
            }


            /*
               1 = Breathing
            */
            else if (effectMode == 1)
            {
                double value =
                    (0.5 +
                     0.5 *
                     sin(phase * 2.0))
                    * brightness;

                HSVtoRGB(
                    phase * 0.02,
                    0.75,
                    value,
                    &r,
                    &g,
                    &b);
            }


            /*
               2 = Wave
            */
            else if (effectMode == 2)
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
                         phase * 5.0))
                    * brightness;

                r = value;
                g = value * 0.45;
                b = value * 0.10;
            }


            /*
               3 = Stars
            */
            else if (effectMode == 3)
            {
                double z =
                    sin(
                        x * 12.9898 +
                        y * 78.233 +
                        floor(phase * 1.5) *
                        37.7);

                double value =
                    ((z - floor(z)) *
                     0.85 +
                     0.15) *
                    brightness;

                BOOL star =
                    ((x * 17 +
                      y * 31 +
                      (int)phase) % 19)
                    == 0;

                r = star ? value : value * 0.02;
                g = star ? value * 0.25 : value * 0.02;
                b = star ? value : value * 0.02;
            }


            /*
               4 = Solid
            */
            else
            {
                r = 0.50 * brightness;
                g = 0.05 * brightness;
                b = 0.12 * brightness;
            }


            /*
               CPU monitor
               Top row becomes a CPU usage meter.
            */
            if (cpuEnabled && y == 0)
            {
                double value =
                    cpuUsage / 100.0;

                if (x < (int)(value * 8.0))
                {
                    r = 0.08;
                    g = 1.0 - value;
                    b = 0.05;
                }
                else
                {
                    r *= 0.15;
                    g *= 0.15;
                    b *= 0.15;
                }
            }


            /*
               RAM monitor
               Bottom row becomes RAM usage meter.
            */
            if (ramEnabled && y == 7)
            {
                double value =
                    1.0 -
                    (double)memoryStatus.ullAvailPhys /
                    (double)memoryStatus.ullTotalPhys;

                if (x < (int)(value * 8.0))
                {
                    r = 0.10;
                    g = 1.0 - value;
                    b = 0.05;
                }
            }


            /*
               Temperature indicator placeholder.

               Real GPU/CPU temperature support will be added
               separately.
            */
            if (tempEnabled &&
                x == 7 &&
                y == 7)
            {
                r = 1.0;
                g = 0.10;
                b = 0.0;
            }


            SetRGB(
                x,
                y,
                Clamp63(r * 63.0),
                Clamp63(g * 63.0),
                Clamp63(b * 63.0));
        }
    }
}


/* ============================================================
   System statistics
   ============================================================ */

static void UpdateStatistics(void)
{
    if (cpuQuery && cpuCounter)
    {
        PDH_FMT_COUNTERVALUE value;

        if (PdhCollectQueryData(cpuQuery)
                == ERROR_SUCCESS)
        {
            if (PdhGetFormattedCounterValue(
                    cpuCounter,
                    PDH_FMT_DOUBLE,
                    NULL,
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
   UI helper
   ============================================================ */

static void AddButton(
    HWND parent,
    int id,
    int x,
    int y,
    int width,
    int height,
    LPCWSTR text)
{
    CreateWindowW(
        L"BUTTON",
        text,
        WS_CHILD | WS_VISIBLE,
        x,
        y,
        width,
        height,
        parent,
        (HMENU)(INT_PTR)id,
        GetModuleHandle(NULL),
        NULL);
}


/* ============================================================
   Window procedure
   ============================================================ */

static LRESULT CALLBACK WindowProc(
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

            /*
               Detect language before creating UI.
            */
            DetectLanguage();

            SetWindowTextW(
                window,
                TXT("title"));


            /*
               Title
            */
            CreateWindowW(
                L"STATIC",
                TXT("title"),
                WS_CHILD | WS_VISIBLE,
                18,
                12,
                300,
                28,
                window,
                NULL,
                GetModuleHandle(NULL),
                NULL);


            /*
               MIDI
            */
            CreateWindowW(
                L"STATIC",
                TXT("midi"),
                WS_CHILD | WS_VISIBLE,
                18,
                48,
                40,
                22,
                window,
                NULL,
                GetModuleHandle(NULL),
                NULL);


            ports =
                CreateWindowW(
                    L"COMBOBOX",
                    NULL,
                    WS_CHILD |
                    WS_VISIBLE |
                    CBS_DROPDOWNLIST,
                    60,
                    45,
                    340,
                    200,
                    window,
                    (HMENU)C_PORT,
                    GetModuleHandle(NULL),
                    NULL);


            AddButton(
                window,
                C_REFRESH,
                18,
                78,
                95,
                28,
                TXT("refresh"));

            AddButton(
                window,
                C_CONNECT,
                120,
                78,
                95,
                28,
                TXT("connect"));


            /*
               Effects
            */

            AddButton(
                window,
                C_RAINBOW,
                18,
                118,
                72,
                28,
                TXT("rainbow"));

            AddButton(
                window,
                C_BREATHE,
                96,
                118,
                72,
                28,
                TXT("breathe"));

            AddButton(
                window,
                C_WAVE,
                174,
                118,
                72,
                28,
                TXT("wave"));

            AddButton(
                window,
                C_STARS,
                252,
                118,
                72,
                28,
                TXT("stars"));

            AddButton(
                window,
                C_SOLID,
                330,
                118,
                72,
                28,
                TXT("solid"));


            /*
               Monitoring
            */

            CreateWindowW(
                L"STATIC",
                TXT("status"),
                WS_CHILD | WS_VISIBLE,
                18,
                158,
                70,
                22,
                window,
                NULL,
                GetModuleHandle(NULL),
                NULL);

            AddButton(
                window,
                C_CPU,
                92,
                153,
                60,
                28,
                TXT("cpu"));

            AddButton(
                window,
                C_GPU,
                158,
                153,
                60,
                28,
                TXT("gpu"));

            AddButton(
                window,
                C_RAM,
                224,
                153,
                60,
                28,
                TXT("ram"));

            AddButton(
                window,
                C_TEMP,
                290,
                153,
                60,
                28,
                TXT("temp"));


            /*
               Brightness
            */

            CreateWindowW(
                L"STATIC",
                TXT("brightness"),
                WS_CHILD | WS_VISIBLE,
                18,
                198,
                45,
                22,
                window,
                NULL,
                GetModuleHandle(NULL),
                NULL);

            brightnessSlider =
                CreateWindowW(
                    TRACKBAR_CLASSW,
                    NULL,
                    WS_CHILD | WS_VISIBLE,
                    60,
                    193,
                    340,
                    30,
                    window,
                    (HMENU)C_BRIGHT,
                    GetModuleHandle(NULL),
                    NULL);

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


            /*
               Speed
            */

            CreateWindowW(
                L"STATIC",
                TXT("speed"),
                WS_CHILD | WS_VISIBLE,
                18,
                238,
                45,
                22,
                window,
                NULL,
                GetModuleHandle(NULL),
                NULL);

            speedSlider =
                CreateWindowW(
                    TRACKBAR_CLASSW,
                    NULL,
                    WS_CHILD | WS_VISIBLE,
                    60,
                    233,
                    340,
                    30,
                    window,
                    (HMENU)C_SPEED,
                    GetModuleHandle(NULL),
                    NULL);

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


            /*
               Status
            */

            statusText =
                CreateWindowW(
                    L"STATIC",
                    TXT("not_connected"),
                    WS_CHILD | WS_VISIBLE,
                    18,
                    280,
                    380,
                    25,
                    window,
                    (HMENU)C_STATUS,
                    GetModuleHandle(NULL),
                    NULL);


            /*
               MIDI devices
            */
            RefreshPorts();


            /*
               CPU performance counter
            */
            PdhOpenQueryW(
                NULL,
                0,
                &cpuQuery);

            PdhAddEnglishCounterW(
                cpuQuery,
                L"\\Processor(_Total)\\% Processor Time",
                0,
                &cpuCounter);

            PdhCollectQueryData(
                cpuQuery);


            SetTimer(
                window,
                T_STATS,
                1000,
                NULL);

            SetTimer(
                window,
                T_FRAME,
                80,
                NULL);

            break;
        }


        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case C_REFRESH:
                    RefreshPorts();
                    break;


                case C_CONNECT:
                    ConnectMIDI();
                    break;


                case C_RAINBOW:
                    effectMode = 0;
                    running = 1;
                    break;


                case C_BREATHE:
                    effectMode = 1;
                    running = 1;
                    break;


                case C_WAVE:
                    effectMode = 2;
                    running = 1;
                    break;


                case C_STARS:
                    effectMode = 3;
                    running = 1;
                    break;


                case C_SOLID:
                    effectMode = 4;
                    running = 1;
                    break;


                case C_CPU:
                    cpuEnabled = !cpuEnabled;
                    break;


                case C_GPU:
                    gpuEnabled = !gpuEnabled;
                    break;


                case C_RAM:
                    ramEnabled = !ramEnabled;
                    break;


                case C_TEMP:
                    tempEnabled = !tempEnabled;
                    break;
            }

            break;
        }


        case WM_TIMER:
        {
            if (wParam == T_FRAME)
            {
                RenderFrame();
            }
            else if (wParam == T_STATS)
            {
                UpdateStatistics();
            }

            break;
        }


        case WM_DESTROY:
        {
            KillTimer(
                window,
                T_FRAME);

            KillTimer(
                window,
                T_STATS);

            CloseMIDI();

            if (cpuQuery)
            {
                PdhCloseQuery(
                    cpuQuery);

                cpuQuery = NULL;
            }

            PostQuitMessage(0);

            break;
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
    HINSTANCE previousInstance,
    LPWSTR commandLine,
    int showCommand)
{
    (void)previousInstance;
    (void)commandLine;


    /*
       Trackbar controls
    */
    INITCOMMONCONTROLSEX controls;

    controls.dwSize =
        sizeof(controls);

    controls.dwICC =
        ICC_BAR_CLASSES;

    InitCommonControlsEx(
        &controls);


    /*
       Register window class
    */
    WNDCLASSW windowClass;

    ZeroMemory(
        &windowClass,
        sizeof(windowClass));

    windowClass.lpfnWndProc =
        WindowProc;

    windowClass.hInstance =
        instance;

    windowClass.lpszClassName =
        L"LaunchpadRGBAmbient";

    windowClass.hCursor =
        LoadCursor(
            NULL,
            IDC_ARROW);

    windowClass.hbrBackground =
        (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(
        &windowClass);


    /*
       Create main window
    */
    HWND window =
        CreateWindowW(
            L"LaunchpadRGBAmbient",
            L"Launchpad RGB Ambient",
            WS_OVERLAPPED |
            WS_CAPTION |
            WS_SYSMENU |
            WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            440,
            350,
            NULL,
            NULL,
            instance,
            NULL);


    if (!window)
        return 1;


    ShowWindow(
        window,
        showCommand);

    UpdateWindow(
        window);


    /*
       Message loop
    */
    MSG message;

    while (GetMessageW(
        &message,
        NULL,
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
