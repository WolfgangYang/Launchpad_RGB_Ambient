#include "Effect.h"
#include "../LED/LEDController.h"
#include <cmath>

namespace Effect {

// Note: In v0.1, these were implemented as standalone functions 
// or within the main file. We are now moving them to a factory pattern.

class RainbowEffect : public IEffect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
};

class BreatheEffect : public IEffect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
};

class WaveEffect : public IEffect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
};

class StarsEffect : public IEffect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
};

class SolidEffect : public IEffect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
};

IEffect* CreateEffect(Type type) {
    switch (type) {
        case Type::Rainbow: return new RainbowEffect();
        case Type::Breathe: return new BreatheEffect();
        case Type::Wave:   return new WaveEffect();
        case Type::Stars:  return new StarsEffect();
        case Type::Solid:  return new SolidEffect();
        default:           return nullptr;
    }
}

// --- Implementations (Moved from main.cpp) ---

void RainbowEffect::Render(double phase, double brightness, 
                          bool cpuIndicator, bool gpuIndicator, 
                          bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double hue = fmod(phase + (x + y) * 0.125, 1.0);
            double saturation = 0.8;
            double value = brightness / 100.0;
            
            double r, g, b;
            if (hue < 0.0) hue += 1.0;
            int sector = static_cast<int>(hue * 6) % 6;
            double fraction = hue * 6 - sector;
            
            switch (sector) {
                case 0: r = value; g = value * (1.0 - saturation); b = value * (1.0 - saturation * fraction); break;
                case 1: r = value * (1.0 - saturation * (1.0 - fraction)); g = value; b = value * (1.0 - saturation); break;
                case 2: r = value * (1.0 - saturation); g = value; b = value * fraction; break;
                case 3: r = value * (1.0 - saturation * fraction); g = value; b = value; break;
                case 4: r = value * (1.0 - saturation); g = value * fraction; b = value; break;
                case 5: r = value; g = value * saturation * fraction; b = value; break;
            }
            LEDController::SetLED(x, y, static_cast<BYTE>(r * 63), static_cast<BYTE>(g * 63), static_cast<BYTE>(b * 63));
        }
    }
}

void BreatheEffect::Render(double phase, double brightness, 
                          bool cpuIndicator, bool gpuIndicator, 
                          bool ramIndicator, bool temperatureIndicator) {
    double breath = (sin(phase * 0.5) + 1.0) / 2.0;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double intensity = breath * brightness / 100.0;
            LEDController::SetLED(x, y, static_cast<BYTE>(intensity * 20), static_cast<BYTE>(intensity * 40), static_cast<BYTE>(intensity * 80));
        }
    }
}

void WaveEffect::Render(double phase, double brightness, 
                       bool cpuIndicator, bool gpuIndicator, 
                       bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double wave = sin((x + phase) * 0.5) * cos((y + phase * 0.3) * 0.3);
            double intensity = (wave + 1.0) / 2.0 * brightness / 100.0;
            LEDController::SetLED(x, y, static_cast<BYTE>(intensity * 63), static_cast<BYTE>(intensity * 63), static_cast<BYTE>(intensity * 63));
        }
    }
}

void StarsEffect::Render(double phase, double brightness, 
                        bool cpuIndicator, bool gpuIndicator, 
                        bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int seed = (x * 7 + y * 3 + static_cast<int>(phase * 10)) % 5;
            bool star = (seed == 0);
            double value = star ? brightness : brightness * 0.02;
            if (star) {
                LEDController::SetLED(x, y, static_cast<BYTE>(value * 63), static_cast<BYTE>(value * 63), static_cast<BYTE>(value * 63));
            } else {
                LEDController::SetLED(x, y, 0, 0, 0);
            }
        }
    }
}

void SolidEffect::Render(double phase, double brightness, 
                        bool cpuIndicator, bool gpuIndicator, 
                        bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double intensity = brightness / 100.0;
            LEDController::SetLED(x, y, static_cast<BYTE>(intensity * 63), static_cast<BYTE>(intensity * 63), static_cast<BYTE>(intensity * 63));
        }
    }
}

} // namespace Effect
