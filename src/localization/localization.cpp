#include "localization.h"

#include <cstring>

namespace lra {

namespace {
constexpr wchar_t kTitle[] = L"Launchpad RGB Ambient — v0.5";
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
    if (!std::strcmp(key, "tab_effects")) return zh ? L"光效" : L"Effects";
    if (!std::strcmp(key, "tab_system")) return zh ? L"系统" : L"System";
    if (!std::strcmp(key, "tab_text")) return zh ? L"文字" : L"Text";
    if (!std::strcmp(key, "tab_device")) return zh ? L"设备" : L"Device";
    if (!std::strcmp(key, "tab_settings")) return zh ? L"设置" : L"Settings";
    if (!std::strcmp(key, "preview")) return zh ? L"Launchpad 预览" : L"Launchpad Preview";
    if (!std::strcmp(key, "palette")) return zh ? L"Launchpad 标准颜色" : L"Launchpad Standard Colors";
    if (!std::strcmp(key, "palette_note")) return zh ? L"（不保证所有颜色可正常显示）" : L"(Not all colors are guaranteed to display correctly)";
    if (!std::strcmp(key, "placeholder_system")) return zh ? L"系统监控选项将在后续版本加入" : L"System monitoring options will be added in a later version";
    if (!std::strcmp(key, "placeholder_text")) return zh ? L"文字与图标功能将在后续版本加入" : L"Text and icon features will be added in a later version";
    if (!std::strcmp(key, "placeholder_settings")) return zh ? L"更多设置将在后续版本加入" : L"More settings will be added in a later version";
    if (!std::strcmp(key, "preview_hint")) return zh ? L"预览与实际Launchpad显示同步" : L"Preview is synchronized with the actual Launchpad display";

    if (!std::strcmp(key, "midi")) return L"MIDI";
    if (!std::strcmp(key, "refresh")) return zh ? L"刷新" : L"Refresh";
    if (!std::strcmp(key, "connect")) return zh ? L"连接" : L"Connect";
    if (!std::strcmp(key, "rainbow")) return zh ? L"彩虹" : L"Rainbow";
    if (!std::strcmp(key, "breathe")) return zh ? L"呼吸" : L"Breathe";
    if (!std::strcmp(key, "wave")) return zh ? L"波纹" : L"Wave";
    if (!std::strcmp(key, "stars")) return zh ? L"星空" : L"Stars";
    if (!std::strcmp(key, "solid")) return zh ? L"纯色" : L"Solid";
    if (!std::strcmp(key, "stop")) return zh ? L"停止" : L"Stop";
    if (!std::strcmp(key, "text_input")) return zh ? L"输入文字" : L"Text";
    if (!std::strcmp(key, "text_hint")) return zh ? L"仅支持英文字母 A-Z / a-z" : L"English letters only: A-Z / a-z";
    if (!std::strcmp(key, "text_start")) return zh ? L"开始显示" : L"Start";
    if (!std::strcmp(key, "text_invalid")) return zh ? L"文字只能包含英文字母 A-Z / a-z" : L"Text can contain English letters A-Z / a-z only";
    if (!std::strcmp(key, "tray_on_close")) return zh ? L"关闭窗口时隐藏到系统托盘" : L"Hide to system tray when closing";
    if (!std::strcmp(key, "settings_note")) return zh ? L"勾选后点击关闭按钮将隐藏窗口，而不是退出程序。" : L"When enabled, closing the window hides it instead of exiting.";
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
