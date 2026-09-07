#pragma once

#include "types.h"

namespace lra {

Rgb hsvToRgb(double h, double s, double v);
std::uint8_t toMidiValue(double value);

} // namespace lra
