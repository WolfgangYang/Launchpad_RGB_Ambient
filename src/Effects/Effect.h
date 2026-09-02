// Effect Base Class
#pragma once

#include "../LED/LEDController.h"
#include <windows.h>

class Effect {
public:
    enum class Type { Rainbow, Breathe, Wave, Stars, Solid };
    
    virtual void Render(double phase, double brightness, 
                       bool cpuIndicator, bool gpuIndicator, 
                       bool ramIndicator, bool temperatureIndicator) = 0;
    virtual const wchar_t* GetName() = 0;
    static Effect* Create(Type type);
    virtual ~Effect() {}
};

// Effect implementations
class RainbowEffect : public Effect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
    const wchar_t* GetName() override { return L"Rainbow"; }
};

class BreatheEffect : public Effect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
    const wchar_t* GetName() override { return L"Breathe"; }
};

class WaveEffect : public Effect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
    const wchar_t* GetName() override { return L"Wave"; }
};

class StarsEffect : public Effect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
    const wchar_t* GetName() override { return L"Stars"; }
};

class SolidEffect : public Effect {
public:
    void Render(double phase, double brightness, 
                bool cpuIndicator, bool gpuIndicator, 
                bool ramIndicator, bool temperatureIndicator) override;
    const wchar_t* GetName() override { return L"Solid"; }
};