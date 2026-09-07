#pragma once

#include <cstdint>

#include "types.h"

namespace lra {

Rgb hsvToRgb(double h, double s, double v);
std::uint8_t toMidiValue(double value);

struct PaletteColor {
    std::uint8_t index = 0;
    Rgb rgb{};
};

const PaletteColor& launchpadPalette(std::uint8_t index);
const Rgb& selectedPaletteRgb(std::uint8_t index);
std::uint8_t nearestPaletteIndex(const Rgb& rgb);
Rgb snapToPalette(const Rgb& rgb);

} // namespace lra
