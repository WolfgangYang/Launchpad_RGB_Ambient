// Effect Implementations
#include "Effect.h"
#include <cmath>

Effect* Effect::Create(Type type) {
    switch (type) {
        case Type::Rainbow: return new RainbowEffect();
        case Type::Breathe: return new BreatheEffect();
        case Type::Wave: return new WaveEffect();
        case Type::Stars: return new StarsEffect();
        case Type::Solid: return new SolidEffect();
        default: return new RainbowEffect();
    }
}

// Rainbow effect implementation
void RainbowEffect::Render(double phase, double brightness, 
                          bool cpuIndicator, bool gpuIndicator, 
                          bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double hue = fmod(phase + (x + y) * 0.125, 1.0);
            
            // HSV to RGB
            double r, g, b;
            if (hue < 0.0) hue += 1.0;
            int sector = static_cast<int>(hue * 6) % 6;
            double fraction = hue * 6 - sector;
            double sat = 0.8;
            double val = brightness / 100.0;
            
            switch (sector) {
                case 0: r = val; g = val * (1.0 - sat); b = val * (1.0 - sat * fraction); break;
                case 1: r = val * (1.0 - sat * (1.0 - fraction)); g = val; b = val * (1.0 - sat); break;
                case 2: r = val * (1.0 - sat); g = val; b = val * fraction; break;
                case 3: r = val * (1.0 - sat * fraction); g = val * (1.0 - sat); b = val; break;
                case 4: r = val * (1.0 - sat); g = val * fraction; b = val; break;
                case 5: r = val; g = val * sat * fraction; b = val; break;
            }
            
            LEDController::SetLED(x, y, 
                static_cast<BYTE>(r * 63), 
                static_cast<BYTE>(g * 63), 
                static_cast<BYTE>(b * 63));
        }
    }
}

// Breathe effect implementation
void BreatheEffect::Render(double phase, double brightness, 
                          bool cpuIndicator, bool gpuIndicator, 
                          bool ramIndicator, bool temperatureIndicator) {
    double breath = (sin(phase * 0.5) + 1.0) / 2.0;
    
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double intensity = breath * brightness / 100.0;
            LEDController::SetLED(x, y, 
                static_cast<BYTE>(intensity * 20),
                static_cast<BYTE>(intensity * 40),
                static_cast<BYTE>(intensity * 80));
        }
    }
}

// Wave effect implementation
void WaveEffect::Render(double phase, double brightness, 
                       bool cpuIndicator, bool gpuIndicator, 
                       bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double wave = sin((x + phase) * 0.5) * cos((y + phase * 0.3) * 0.3);
            double intensity = (wave + 1.0) / 2.0 * brightness / 100.0;
            
            LEDController::SetLED(x, y,
                static_cast<BYTE>(intensity * 63),
                static_cast<BYTE>(intensity * 63),
                static_cast<BYTE>(intensity * 63));
        }
    }
}

// Stars effect implementation
void StarsEffect::Render(double phase, double brightness, 
                        bool cpuIndicator, bool gpuIndicator, 
                        bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            int seed = (x * 7 + y * 3 + static_cast<int>(phase * 10)) % 5;
            bool star = (seed == 0);
            
            double value = star ? brightness : brightness * 0.02;
            
            if (star) {
                LEDController::SetLED(x, y, 
                    static_cast<BYTE>(value * 63),
                    static_cast<BYTE>(value * 63),
                    static_cast<BYTE>(value * 63));
            } else {
                LEDController::SetLED(x, y, 0, 0, 0);
            }
        }
    }
}

// Solid effect implementation
void SolidEffect::Render(double phase, double brightness, 
                        bool cpuIndicator, bool gpuIndicator, 
                        bool ramIndicator, bool temperatureIndicator) {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            double intensity = brightness / 100.0;
            LEDController::SetLED(x, y,
                static_cast<BYTE>(intensity * 63),
                static_cast<BYTE>(intensity * 63),
                static_cast<BYTE>(intensity * 63));
        }
    }
}