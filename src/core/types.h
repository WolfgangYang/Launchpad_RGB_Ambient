#pragma once

#include <cstdint>

namespace lra {

enum class Language {
    English,
    Chinese
};

enum class Effect {
    Rainbow,
    Breathe,
    Wave,
    Stars,
    Solid
};

struct Rgb {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
};

struct MonitoringState {
    double cpuUsage = 0.0;
    double ramUsage = 0.0;
    double gpuUsage = 0.0;
    double temperature = 0.0;
};

struct AppState {
    Language language = Language::English;
    Effect effect = Effect::Rainbow;
    bool effectRunning = false;

    bool cpuIndicator = false;
    bool gpuIndicator = false;
    bool ramIndicator = false;
    bool temperatureIndicator = false;

    int brightness = 70;
    int speed = 20;
    double animationPhase = 0.0;

    MonitoringState monitoring;
};

} // namespace lra
