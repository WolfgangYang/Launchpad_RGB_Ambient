#include "effect_engine.h"
#include "../core/color.h"

#include <cmath>
#include <algorithm>
#include <cctype>

namespace lra {

void EffectEngine::render(AppState& state, LedFrame& frame, FunctionKeyFrame& functionKeys) const
{
    if (!state.effectRunning) {
        return;
    }

    renderBase(state, frame);
    renderIndicators(state, frame);

    // MK2 has a fixed 128-color hardware palette. Keep the preview and the
    // physical Launchpad in the same practical gamut. Text is intentionally
    // kept crisp and is therefore quantized too.
    for (auto& row : frame) {
        for (auto& pixel : row) {
            pixel = snapToPalette(pixel);
        }
    }

    renderFunctionKeys(frame, functionKeys);

    const double speed = state.speed / 20.0;
    state.animationPhase += 0.035 * (0.2 + speed);

    if (state.effect == Effect::Text) {
        state.textOffset += 0.12 * (0.2 + speed);
        if (state.textOffset > static_cast<double>(state.textContent.size() * 6 + 8)) {
            state.textOffset = -8.0;
        }
    }
}

namespace {

const std::uint8_t* glyph(char ch)
{
    // 5x7 bitmap font. Only English letters are supported by the text UI.
    static const std::uint8_t blank[5] = {0, 0, 0, 0, 0};
    static const std::uint8_t letters[26][5] = {
        {0x1E,0x05,0x05,0x1E,0x00}, // A
        {0x1F,0x15,0x15,0x0A,0x00}, // B
        {0x0E,0x11,0x11,0x11,0x00}, // C
        {0x1F,0x11,0x11,0x0E,0x00}, // D
        {0x1F,0x15,0x15,0x11,0x00}, // E
        {0x1F,0x05,0x05,0x01,0x00}, // F
        {0x0E,0x11,0x15,0x1D,0x00}, // G
        {0x1F,0x04,0x04,0x1F,0x00}, // H
        {0x11,0x1F,0x11,0x00,0x00}, // I
        {0x08,0x10,0x10,0x0F,0x00}, // J
        {0x1F,0x04,0x0A,0x11,0x00}, // K
        {0x1F,0x10,0x10,0x10,0x00}, // L
        {0x1F,0x02,0x04,0x02,0x1F}, // M
        {0x1F,0x02,0x04,0x1F,0x00}, // N
        {0x0E,0x11,0x11,0x0E,0x00}, // O
        {0x1F,0x05,0x05,0x02,0x00}, // P
        {0x0E,0x11,0x19,0x1E,0x00}, // Q
        {0x1F,0x05,0x0D,0x12,0x00}, // R
        {0x12,0x15,0x15,0x09,0x00}, // S
        {0x01,0x1F,0x01,0x00,0x00}, // T
        {0x0F,0x10,0x10,0x0F,0x00}, // U
        {0x07,0x08,0x10,0x08,0x07}, // V
        {0x1F,0x08,0x04,0x08,0x1F}, // W
        {0x11,0x0A,0x04,0x0A,0x11}, // X
        {0x03,0x04,0x18,0x04,0x03}, // Y
        {0x19,0x15,0x13,0x00,0x00}  // Z
    };

    if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
    if (ch < 'A' || ch > 'Z') return blank;
    return letters[ch - 'A'];
}

} // namespace

void EffectEngine::renderBase(const AppState& state, LedFrame& frame) const
{
    const double brightness = state.brightness / 100.0;

    if (state.effect == Effect::Text) {
        for (auto& row : frame) for (auto& pixel : row) pixel = {};

        const Rgb base = selectedPaletteRgb(static_cast<std::uint8_t>(state.paletteIndex));
        const int offset = static_cast<int>(std::floor(state.textOffset));
        const int totalWidth = static_cast<int>(state.textContent.size()) * 6;

        for (int x = 0; x < 8; ++x) {
            const int sourceX = x + offset;
            if (sourceX < 0 || sourceX >= totalWidth) continue;

            const int charIndex = sourceX / 6;
            const int glyphX = sourceX % 6;
            if (glyphX >= 5) continue;

            const std::uint8_t column = glyph(state.textContent[charIndex])[glyphX];
            for (int y = 0; y < 7; ++y) {
                if (column & (1u << y)) {
                    frame[y][x] = {base.r * brightness, base.g * brightness, base.b * brightness};
                }
            }
        }
        return;
    }

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Rgb rgb{};

            switch (state.effect) {
            case Effect::Rainbow: {
                // Rainbow has its own hue cycle; the palette selector does not
                // influence it because there is no meaningful single base color.
                rgb = hsvToRgb(
                    x / 8.0 + y / 16.0 + state.animationPhase * 0.11,
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

            case Effect::Text:
                break;
            }

            frame[y][x] = rgb;
        }
    }
}

void EffectEngine::renderIndicators(const AppState& state, LedFrame& frame) const
{
    const auto barLength = [](double usage) {
        const double value = std::clamp(usage / 100.0, 0.0, 1.0);
        return static_cast<int>(std::round(value * 8.0));
    };

    // Indicators are a deliberate overlay. CPU and RAM occupy their own rows;
    // GPU and temperature reserve the two corner cells, avoiding accidental
    // overwrites when several indicators are enabled together.
    if (state.cpuIndicator) {
        const int length = barLength(state.monitoring.cpuUsage);
        for (int x = 0; x < 8; ++x) {
            if (state.gpuIndicator && x == 7) continue;
            if (x < length) {
                const double value = std::clamp(state.monitoring.cpuUsage / 100.0, 0.0, 1.0);
                frame[0][x] = {0.08, 1.0 - value, 0.05};
            } else {
                frame[0][x] = {frame[0][x].r * 0.15, frame[0][x].g * 0.15, frame[0][x].b * 0.15};
            }
        }
    }

    if (state.ramIndicator) {
        const int length = barLength(state.monitoring.ramUsage);
        for (int x = 0; x < 8; ++x) {
            if (state.temperatureIndicator && x == 7) continue;
            if (x < length) {
                const double value = std::clamp(state.monitoring.ramUsage / 100.0, 0.0, 1.0);
                frame[7][x] = {0.10, 1.0 - value, 0.05};
            } else {
                frame[7][x] = {frame[7][x].r * 0.15, frame[7][x].g * 0.15, frame[7][x].b * 0.15};
            }
        }
    }

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
