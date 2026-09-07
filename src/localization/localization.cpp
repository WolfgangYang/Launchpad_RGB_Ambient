#include "localization.h"

#include <cstring>

namespace lra {

namespace {
constexpr wchar_t kTitle[] = L"Launchpad RGB Ambient v0.3";
}

void detectLanguage(AppState& state)
{
    const LANGID lang = GetUserDefaultUILanguage();
    state.language = PRIMARYLANGID(lang) == LANG_CHINESE
        ? Language::Chinese
        : Language::English;
}

LPCWSTR text(Language language, const char* key)
{
    const bool zh = language == Language::Chinese;

    if (!std::strcmp(key, "title")) return kTitle;
    if (!std::strcmp(key, "midi")) return L"MIDI";
    if (!std::strcmp(key, "refresh")) return zh ? L"刷新" : L"Refresh";
    if (!std::strcmp(key, "connect")) return zh ? L"连接" : L"Connect";
    if (!std::strcmp(key, "rainbow")) return zh ? L"彩虹" : L"Rainbow";
    if (!std::strcmp(key, "breathe")) return zh ? L"呼吸" : L"Breathe";
    if (!std::strcmp(key, "wave")) return zh ? L"波纹" : L"Wave";
    if (!std::strcmp(key, "stars")) return zh ? L"星空" : L"Stars";
    if (!std::strcmp(key, "solid")) return zh ? L"纯色" : L"Solid";
    if (!std::strcmp(key, "monitor")) return zh ? L"状态指示" : L"Monitoring";
    if (!std::strcmp(key, "cpu")) return L"CPU";
    if (!std::strcmp(key, "gpu")) return L"GPU";
    if (!std::strcmp(key, "ram")) return zh ? L"内存" : L"RAM";
    if (!std::strcmp(key, "temp")) return zh ? L"温度" : L"Temp";
    if (!std::strcmp(key, "brightness")) return zh ? L"亮度" : L"Brightness";
    if (!std::strcmp(key, "speed")) return zh ? L"速度" : L"Speed";
    if (!std::strcmp(key, "connected")) return zh ? L"已连接" : L"Connected";
    if (!std::strcmp(key, "notconnected")) return zh ? L"未连接" : L"Not connected";
    if (!std::strcmp(key, "error")) return zh ? L"MIDI 打开失败" : L"MIDI open failed";
    return L"";
}

LPCWSTR windowTitle()
{
    return kTitle;
}

} // namespace lra
