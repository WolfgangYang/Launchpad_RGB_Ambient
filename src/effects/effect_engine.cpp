#include "effect_engine.h"
#include "../core/color.h"

#include <cmath>

namespace lra {

void EffectEngine::render(AppState& state, LedFrame& frame) const
{
    if (!state.effectRunning) {
        return;
    }

    renderBase(state, frame);
    renderIndicators(state, frame);

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
            case Effect::Rainbow:
                rgb = hsvToRgb(
                    x / 8.0 + y / 16.0 + state.animationPhase * 0.08,
                    0.9, brightness);
                break;

            case Effect::Breathe: {
                const double value =
                    (0.5 + 0.5 * std::sin(state.animationPhase * 2.0)) * brightness;
                rgb = hsvToRgb(state.animationPhase * 0.02, 0.75, value);
                break;
            }

            case Effect::Wave: {
                const double dx = x - 3.5;
                const double dy = y - 3.5;
                const double distance = std::sqrt(dx * dx + dy * dy);
                const double value =
                    (0.5 + 0.5 * std::sin(distance * 2.0 - state.animationPhase * 5.0))
                    * brightness;
                rgb = {value, value * 0.45, value * 0.10};
                break;
            }

            case Effect::Stars: {
                const double n = std::sin(
                    x * 12.9898 + y * 78.233 +
                    std::floor(state.animationPhase * 1.5) * 37.7);
                const double value = (n - std::floor(n)) * 0.85 * brightness;
                const bool star =
                    ((x * 17 + y * 31 + static_cast<int>(state.animationPhase)) % 19) == 0;
                if (star) {
                    rgb = {value, value * 0.25, value};
                } else {
                    rgb = {value * 0.02, value * 0.02, value * 0.02};
                }
                break;
            }

            case Effect::Solid:
                rgb = {0.5 * brightness, 0.05 * brightness, 0.12 * brightness};
                break;
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

} // namespace lra
