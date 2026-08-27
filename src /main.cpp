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


    if
