#include "application.h"
#include "../core/color.h"
#include "../localization/localization.h"

#include <algorithm>
#include <array>
#include <cwchar>

namespace lra {

bool Application::initialize(HWND window)
{
    window_ = window;
    detectLanguage(state_);
    state_.effectRunning = false;
    state_.textContent.clear();
    state_.textOffset = -8.0;
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
            SendMessageW(
                combo,
                CB_ADDSTRING,
                0,
                reinterpret_cast<LPARAM>(port.name.c_str())
            )
        );

        SendMessageW(
            combo,
            CB_SETITEMDATA,
            index,
            static_cast<LPARAM>(port.deviceIndex)
        );

        if (port.name.find(L"Launchpad MK2") != std::wstring::npos) {
            preferred = index;
        }
    }

    const LRESULT count =
        SendMessageW(combo, CB_GETCOUNT, 0, 0);

    if (count > 0) {
        SendMessageW(
            combo,
            CB_SETCURSEL,
            preferred >= 0 ? preferred : 0,
            0
        );
    }
}
void Application::syncMidiPorts(
    HWND combo,
    HWND statusLabel)
{
    // The render loop already detects an unplugged device and
    // invalidates the MIDI handle. This function only keeps the
    // device list synchronized with the actual Windows MIDI ports.
    if (midi_.isOpen()) {
        return;
    }

    const auto ports = midi_.enumerate();

    const LRESULT currentCount =
        SendMessageW(
            combo,
            CB_GETCOUNT,
            0,
            0
        );

    bool changed =
        currentCount != static_cast<LRESULT>(ports.size());

    if (!changed) {
        for (int i = 0; i < currentCount; ++i) {
            const UINT currentDeviceIndex =
                static_cast<UINT>(
                    SendMessageW(
                        combo,
                        CB_GETITEMDATA,
                        i,
                        0
                    )
                );

            wchar_t currentName[256]{};

            SendMessageW(
                combo,
                CB_GETLBTEXT,
                i,
                reinterpret_cast<LPARAM>(currentName)
            );

            if (
                currentDeviceIndex != ports[i].deviceIndex
                || std::wstring(currentName) != ports[i].name
            ) {
                changed = true;
                break;
            }
        }
    }

    if (!changed) {
        return;
    }

    refreshMidiPorts(combo);

    SetWindowTextW(
        statusLabel,
        text(
            state_.language,
            "notconnected"
        )
    );
}
bool Application::connectMidi(HWND combo, HWND statusLabel)
{
    const int selected = static_cast<int>(
        SendMessageW(combo, CB_GETCURSEL, 0, 0)
    );

    if (selected < 0) {
        SetWindowTextW(
            statusLabel,
            text(state_.language, "notconnected")
        );
        return false;
    }

    const UINT deviceIndex = static_cast<UINT>(
        SendMessageW(combo, CB_GETITEMDATA, selected, 0)
    );

    if (!midi_.open(deviceIndex)) {
        SetWindowTextW(
            statusLabel,
            text(state_.language, "error")
        );
        return false;
    }

    SetWindowTextW(
        statusLabel,
        text(state_.language, "connected")
    );

    return true;
}

void Application::setEffect(Effect effect)
{
    state_.effect = effect;
    state_.effectRunning = true;
    state_.textOffset = -8.0;
}

void Application::startText(const std::wstring& content)
{
    if (content.empty()) {
        return;
    }

    state_.textContent = content;
    state_.effect = Effect::Text;
    state_.textOffset = -8.0;
    state_.effectRunning = true;
}

void Application::stopAll()
{
    state_.effectRunning = false;
    state_.cpuIndicator = false;
    state_.gpuIndicator = false;
    state_.ramIndicator = false;
    state_.temperatureIndicator = false;
    state_.textOffset = -8.0;

    for (auto& row : frame_) {
        for (auto& pixel : row) pixel = {};
    }
    for (auto& key : functionKeys_) key = {};

    if (window_) {
        InvalidateRect(window_, nullptr, FALSE);
    }

    if (midi_.isOpen()) {
        midi_.clearGrid();
    }
}

void Application::setPaletteColor(int index)
{
    if (index < 0) {
        index = 0;
    }

    if (index > 127) {
        index = 127;
    }

    state_.paletteIndex = index;
}

void Application::toggleCpu()
{
    state_.cpuIndicator = !state_.cpuIndicator;
    if (state_.cpuIndicator) state_.effectRunning = true;
}

void Application::toggleGpu()
{
    state_.gpuIndicator = !state_.gpuIndicator;
    if (state_.gpuIndicator) state_.effectRunning = true;
}

void Application::toggleRam()
{
    state_.ramIndicator = !state_.ramIndicator;
    if (state_.ramIndicator) state_.effectRunning = true;
}

void Application::toggleTemperature()
{
    state_.temperatureIndicator = !state_.temperatureIndicator;
    if (state_.temperatureIndicator) state_.effectRunning = true;
}

void Application::render()
{
    effects_.render(state_, frame_, functionKeys_);

    if (!state_.effectRunning) {
        for (auto& row : frame_) {
            for (auto& pixel : row) {
                pixel = {};
            }
        }

        for (auto& key : functionKeys_) {
            key = {};
        }
    }

    if (window_) {
        InvalidateRect(window_, nullptr, FALSE);
    }

    if (!midi_.isOpen()) {
        return;
    }

    std::array<std::array<BYTE, 3>, 80> colors{};
    auto toMidi = [](const Rgb& rgb) {
        return std::array<BYTE, 3>{
            static_cast<BYTE>(std::clamp(rgb.r, 0.0, 1.0) * 63.0),
            static_cast<BYTE>(std::clamp(rgb.g, 0.0, 1.0) * 63.0),
            static_cast<BYTE>(std::clamp(rgb.b, 0.0, 1.0) * 63.0)
        };
    };

    int index = 0;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            colors[index++] = toMidi(frame_[y][x]);
        }
    }
    for (int i = 0; i < 8; ++i) {
        colors[index++] = toMidi(functionKeys_[i]);
    }
    for (int i = 0; i < 8; ++i) {
        colors[index++] = toMidi(functionKeys_[8 + i]);
    }

    if (!midi_.sendFrame(colors)) {
        // The device disappeared while rendering.
        stopAll();
    }

}

void Application::updateStatistics()
{
    monitor_.update(state_.monitoring);
}

} // namespace lra
