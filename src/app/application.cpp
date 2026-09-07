#include "application.h"
#include "../core/color.h"
#include "../localization/localization.h"

#include <cwchar>

namespace lra {

bool Application::initialize(HWND window)
{
    window_ = window;
    detectLanguage(state_);
    monitor_.initialize();
    return true;
}

void Application::shutdown()
{
    midi_.close();
    monitor_.shutdown();
    window_ = nullptr;
}

void Application::refreshMidiPorts(HWND combo)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);

    const auto ports = midi_.enumerate();
    int preferred = -1;

    for (const auto& port : ports) {
        const int index = static_cast<int>(
            SendMessageW(combo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(port.name.c_str())));
        SendMessageW(combo, CB_SETITEMDATA, index,
            static_cast<LPARAM>(port.deviceIndex));

        if (port.name.find(L"Launchpad MK2") != std::wstring::npos) {
            preferred = index;
        }
    }

    const LRESULT count = SendMessageW(combo, CB_GETCOUNT, 0, 0);
    if (count > 0) {
        SendMessageW(combo, CB_SETCURSEL,
            preferred >= 0 ? preferred : 0, 0);
    }
}

bool Application::connectMidi(HWND combo, HWND statusLabel)
{
    const int selected = static_cast<int>(
        SendMessageW(combo, CB_GETCURSEL, 0, 0));

    if (selected < 0) {
        SetWindowTextW(statusLabel, text(state_.language, "notconnected"));
        return false;
    }

    const UINT deviceIndex = static_cast<UINT>(
        SendMessageW(combo, CB_GETITEMDATA, selected, 0));

    if (!midi_.open(deviceIndex)) {
        SetWindowTextW(statusLabel, text(state_.language, "error"));
        return false;
    }

    SetWindowTextW(statusLabel, text(state_.language, "connected"));
    return true;
}

void Application::setEffect(Effect effect)
{
    state_.effect = effect;
    state_.effectRunning = true;
}

void Application::toggleCpu() { state_.cpuIndicator = !state_.cpuIndicator; }
void Application::toggleGpu() { state_.gpuIndicator = !state_.gpuIndicator; }
void Application::toggleRam() { state_.ramIndicator = !state_.ramIndicator; }
void Application::toggleTemperature()
{
    state_.temperatureIndicator = !state_.temperatureIndicator;
}

void Application::render()
{
    if (!midi_.isOpen() || !state_.effectRunning) {
        return;
    }

    effects_.render(state_, frame_, functionKeys_);

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const Rgb& rgb = frame_[y][x];
            midi_.setLed(x, y,
                toMidiValue(rgb.r * 63.0),
                toMidiValue(rgb.g * 63.0),
                toMidiValue(rgb.b * 63.0));
        }
    }

    // Right-side keys (top-to-bottom).
    for (int i = 0; i < 8; ++i) {
        const Rgb& rgb = functionKeys_[i];
        midi_.setFunctionKey(i,
            toMidiValue(rgb.r * 63.0),
            toMidiValue(rgb.g * 63.0),
            toMidiValue(rgb.b * 63.0));
    }

    // Top-side keys (left-to-right).
    for (int i = 0; i < 8; ++i) {
        const Rgb& rgb = functionKeys_[8 + i];
        midi_.setTopFunctionKey(i,
            toMidiValue(rgb.r * 63.0),
            toMidiValue(rgb.g * 63.0),
            toMidiValue(rgb.b * 63.0));
    }
}

void Application::updateStatistics()
{
    monitor_.update(state_.monitoring);
}

} // namespace lra
