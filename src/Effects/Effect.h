#pragma once
#include "../LED/LEDController.h"
#include <windows.h>

class Effect {
public:
    enum class Type { Rainbow, Breathe, Wave, Stars, Solid };
    virtual ~Effect() {}
    // phase: 动画相位 (0.0 - 1.0), brightness: 亮度 (0-100)
    virtual void Render(double phase, int brightness) = 0;
    virtual const wchar_t* GetName() = 0;

    static Effect* Create(Type type);
};

class RainbowEffect : public Effect {
public:
    void Render(double phase, int brightness) override;
    const wchar_t* GetName() override { return L"Rainbow"; }
};

class BreatheEffect : public Effect {
public:
    void Render(double phase, int brightness) override;
    const wchar_t* GetName() override { return L"Breathe"; }
};

class WaveEffect : public Effect {
public:
    void Render(double phase, int brightness) override;
    const wchar_t* GetName() override { return L"Wave"; }
};

class StarsEffect : public Effect {
public:
    void Render(double phase, int brightness) override;
    const wchar_t* GetName() override { return L"Stars"; }
};

class SolidEffect : public Effect {
public:
    void Render(double phase, int brightness) override;
    constwchar_t* GetName() override { return L"Solid"; }
};
