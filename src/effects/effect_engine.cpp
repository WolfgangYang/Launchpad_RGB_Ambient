#include "effect_engine.h"
#include "../core/color.h"

#include <cmath>

namespace lra {

void EffectEngine::render(AppState& state, LedFrame& frame, FunctionKeyFrame& functionKeys) const
{
    if (!state.effectRunning) {
        return;
    }

    renderBase(state, frame);
    renderIndicators(state, frame);

    // MK2 has a fixed 128-color hardware palette. Quantize the final frame so
    // the preview and the physical Launchpad show the same practical gamut.
    if (state.effect != Effect::Breathe) {
    for (auto& row : frame) {
        for (auto& pixel : row) {
            pixel = snapToPalette(pixel);
        }
    }
}

    renderFunctionKeys(frame, functionKeys);

    const double speed = state.speed / 20.0;
    state.animationPhase += 0.035 * (0.2 + speed);
}

void EffectEngine::renderBase(const AppState& state, LedFrame& frame) const
{
    const double brightness = state.brightness / 100.0;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Rgb rgb{};

            switch (state.effect) {
            case Effect::Rainbow: {
                const Rgb base = selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));
                const double hueOffset = std::atan2(base.g - base.b, base.r + 1e-6) / (2.0 * 3.141592653589793);
                rgb = hsvToRgb(
                    x / 8.0 + y / 16.0 + state.animationPhase * 0.11 + hueOffset * 0.15,
                    0.9, brightness);
                break;
            }

            case Effect::Breathe: {
                const double value =
                    (0.5 + 0.5 * std::sin(state.animationPhase * 2.0)) * brightness;
                const Rgb base = selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));
                rgb = {base.r * value, base.g * value, base.b * value};
                break;
            }

            case Effect::Wave: {
                const double dx = x - 3.5;
                const double dy = y - 3.5;
                const double distance = std::sqrt(dx * dx + dy * dy);
                const double value =
                    (0.5 + 0.5 * std::sin(distance * 2.0 - state.animationPhase * 5.0))
                    * brightness;
                const Rgb base = selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));
                rgb = {base.r * value, base.g * value, base.b * value};
                break;
            }

            case Effect::Stars: {
                const double n = std::sin(
                    x * 12.9898 + y * 78.233 +
                    std::floor(state.animationPhase * 1.5) * 37.7);
                const double value = (n - std::floor(n)) * 0.85 * brightness;
                const bool star =
                    ((x * 17 + y * 31 + static_cast<int>(state.animationPhase)) % 19) == 0;
                const Rgb base = selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));
                if (star) {
                    rgb = {base.r * value, base.g * value, base.b * value};
                } else {
                    rgb = {base.r * value * 0.02, base.g * value * 0.02, base.b * value * 0.02};
                }
                break;
            }

            case Effect::Solid: {
                const Rgb base = selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));
                rgb = {base.r * brightness, base.g * brightness, base.b * brightness};
                break;
            }
            }

            frame[y][x] = rgb;
        }
    }
}

void EffectEngine::renderIndicators(const AppState& state, LedFrame& frame) const
{
    if (state.cpuIndicator) {
        const double value = state.monitoring.cpuUsage / 100.0;
        for (int x = 0; x < 8; ++x) {
            frame[0][x] = x < static_cast<int>(value * 8.0)
                ? Rgb{0.08, 1.0 - value, 0.05}
                : Rgb{frame[0][x].r * 0.15, frame[0][x].g * 0.15, frame[0][x].b * 0.15};
        }
    }

    if (state.ramIndicator) {
        const double value = state.monitoring.ramUsage / 100.0;
        for (int x = 0; x < 8; ++x) {
            if (x < static_cast<int>(value * 8.0)) {
                frame[7][x] = {0.1, 1.0 - value, 0.05};
            }
        }
    }

    // GPU and temperature are intentionally kept as separate indicators.
    // Their data sources are implemented in monitor/system_monitor.cpp.
    if (state.gpuIndicator) {
        frame[0][7] = {0.15, 0.25, 1.0};
    }

    if (state.temperatureIndicator) {
        frame[7][7] = {1.0, 0.1, 0.0};
    }
}


void EffectEngine::renderFunctionKeys(const LedFrame& frame, FunctionKeyFrame& functionKeys) const
{
    // Right-side keys: top-to-bottom, each mirrors the rightmost RGB pad
    // in the corresponding row.
    for (int i = 0; i < 8; ++i) {
        functionKeys[i] = frame[i][7];
    }

    // Top-side keys: left-to-right, each mirrors the top RGB pad
    // in the corresponding column.
    for (int i = 0; i < 8; ++i) {
        functionKeys[8 + i] = frame[0][i];
    }
}

} // namespace lra
