#include "Effect.h"
#include <cmath>

// 辅助函数：HSV 转 RGB
static void HSVtoRGB(double h, double s, double v, BYTE& r, BYTE& g, BYTE& b) {
    int i = static_cast<int>(h * 6);
    double f = h * 6 - i;
    double p = (1.0 - s) * v;
    double q = (1.0 - f * s) * v;
    double t = f * s * v + (1.0 - f) * v;

    switch (i % 6) {
        case 0: r = v*255; g = t*255; b = p*255; break;
        case 1: r = q*255; g = v*255; b = p*255; break;
        case 2: r = p*255; g = v*255; b = t*255; break;
        case 3: r = p*255; g = q*255; b = v*255; break;
        case 4: r = t*255; g = p*255; b = v*255; break;
        case 5: r = v*255; g = p*255; b = q*255; break;
    }
}

Effect* Effect::Create(Type type) {
    switch (type) {
        case Type::Rainbow: return new RainbowEffect();
        case Type::Breathe: return new BreatheEffect();
        case Type::Wave:    return new WaveEffect();
        case Type::Stars:   return new StarsEffect();
        case Type::Solid:   return new SolidEffect();
        default:            return nullptr;
    }
}

void RainbowEffect::Render(double phase, int brightness) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double h = fmod(phase + (x * 0.125 + y * 0.0625), 1.0);
            BYTE r, g, b;
            HSVtoRGB(h, 0.8, brightness / 100.0, r, g, b);
            LEDController::SetLED(x, y, r, g, b);
        }
    }
}

void BreatheEffect::Render(double phase, int brightness) {
    double pulse = (sin(phase * M_PI) + 1.0) / 2.0; // 0 to 1
    int currentBrightness = static_cast<int>(brightness * pulse);
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            LEDController::SetLED(x, y, 0, currentBrightness, currentBrightness/2);
        }
    }
}

void WaveEffect::Render(double phase, int brightness) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double wave = sin(phase + (x * 0.5) + (y * 0.3));
            BYTE val = static_cast<BYTE>((wave + 1.0) / 2.0 * brightness);
            LEDController::SetLED(x, y, val, val, val);
        }
    }
}

void StarsEffect::Render(double phase, int brightness) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            if ((x * y + static_cast<int>(phase * 10)) % 7 == 0)
                LEDController::SetLED(x, y, brightness, brightness, brightness);
            else
                LEDController::SetLED(x, y, 0, 0, 0);
        }
    }
}

void SolidEffect::Render(double phase, int brightness) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            LEDController::SetLED(x, y, brightness, brightness, brightness);
        }
    }
}
