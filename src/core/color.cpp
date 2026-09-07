#include "color.h"

#include <cmath>

namespace lra {

Rgb hsvToRgb(double h, double s, double v)
{
    h = std::fmod(h, 1.0);
    if (h < 0.0) {
        h += 1.0;
    }

    const double sector = std::floor(h * 6.0);
    const double fraction = h * 6.0 - sector;
    const double p = v * (1.0 - s);
    const double q = v * (1.0 - fraction * s);
    const double t = v * (1.0 - (1.0 - fraction) * s);

    switch (static_cast<int>(sector) % 6) {
    case 0: return {v, t, p};
    case 1: return {q, v, p};
    case 2: return {p, v, t};
    case 3: return {p, q, v};
    case 4: return {t, p, v};
    default: return {v, p, q};
    }
}

std::uint8_t toMidiValue(double value)
{
    if (value < 0.0) value = 0.0;
    if (value > 63.0) value = 63.0;
    return static_cast<std::uint8_t>(value + 0.5);
}

} // namespace lra
