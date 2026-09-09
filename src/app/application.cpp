#include "application.h"
#include "../core/color.h"
#include "../localization/localization.h"

#include <algorithm>
#include <cwchar>

namespace lra {

bool Application::initialize(HWND window)
{
    window_ = window;
    detectLanguage(state_);
    state_.effectRunning = false;
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
}

void Application::toggleGpu()
{
    state_.gpuIndicator = !state_.gpuIndicator;
}

void Application::toggleRam()
{
    state_.ramIndicator = !state_.ramIndicator;
}

void Application::toggleTemperature()
{
    state_.temperatureIndicator =
        !state_.temperatureIndicator;
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

    // Keep the RGB SysEx path used by the verified v0.3 version.
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const Rgb& rgb = frame_[y][x];

            const BYTE red = static_cast<BYTE>(
                std::clamp(rgb.r, 0.0, 1.0) * 63.0
            );

            const BYTE green = static_cast<BYTE>(
                std::clamp(rgb.g, 0.0, 1.0) * 63.0
            );

            const BYTE blue = static_cast<BYTE>(
                std::clamp(rgb.b, 0.0, 1.0) * 63.0
            );

            if (!midi_.setLed(
                    x,
                    y,
                    red,
                    green,
                    blue)) {

                // The device disappeared while rendering.
                // Restore the software to its initial state.
                state_.effectRunning = false;

                for (auto& row : frame_) {
                    for (auto& pixel : row) {
                        pixel = {};
                    }
                }

                for (auto& key : functionKeys_) {
                    key = {};
                }

                if (window_) {
                    InvalidateRect(window_, nullptr, FALSE);
                }

                return;
            }
        }
    }

    // Right-side function keys, top to bottom.
    for (int i = 0; i < 8; ++i) {
        const Rgb& rgb = functionKeys_[i];

        const BYTE red = static_cast<BYTE>(
            std::clamp(rgb.r, 0.0, 1.0) * 63.0
        );

        const BYTE green = static_cast<BYTE>(
            std::clamp(rgb.g, 0.0, 1.0) * 63.0
        );

        const BYTE blue = static_cast<BYTE>(
            std::clamp(rgb.b, 0.0, 1.0) * 63.0
        );

        if (!midi_.setFunctionKey(
                i,
                red,
                green,
                blue)) {

            state_.effectRunning = false;

            for (auto& row : frame_) {
                for (auto& pixel : row) {
                    pixel = {};
                }
            }

            for (auto& key : functionKeys_) {
                key = {};
            }

            if (window_) {
                InvalidateRect(window_, nullptr, FALSE);
            }

            return;
        }
    }

    // Top-side function keys, left to right.
    for (int i = 0; i < 8; ++i) {
        const Rgb& rgb = functionKeys_[8 + i];

        const BYTE red = static_cast<BYTE>(
            std::clamp(rgb.r, 0.0, 1.0) * 63.0
        );

        const BYTE green = static_cast<BYTE>(
            std::clamp(rgb.g, 0.0, 1.0) * 63.0
        );

        const BYTE blue = static_cast<BYTE>(
            std::clamp(rgb.b, 0.0, 1.0) * 63.0
        );

        if (!midi_.setTopFunctionKey(
                i,
                red,
                green,
                blue)) {

            state_.effectRunning = false;

            for (auto& row : frame_) {
                for (auto& pixel : row) {
                    pixel = {};
                }
            }

            for (auto& key : functionKeys_) {
                key = {};
            }

            if (window_) {
                InvalidateRect(window_, nullptr, FALSE);
            }

            return;
        }
    }
}

void Application::updateStatistics()
{
    monitor_.update(state_.monitoring);
}

} // namespace lra
