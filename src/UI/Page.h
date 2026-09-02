// Page Base Class
#pragma once

#include <windows.h>
#include <vector>

class Page {
public:
    virtual void Initialize(HWND parent) = 0;
    virtual void Render() = 0;
    virtual void Update() = 0;
    virtual void OnResize(int width, int height) = 0;
    virtual const wchar_t* GetTitle() = 0;
    virtual ~Page() {}
};

class EffectPage : public Page {
public:
    void Initialize(HWND parent) override;
    void Render() override;
    void Update() override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() override { return L"氛围灯效"; }
};

class StatusPage : public Page {
public:
    void Initialize(HWND parent) override;
    void Render() override;
    void Update() override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() override { return L"状态指示"; }
};

class TextPage : public Page {
public:
    void Initialize(HWND parent) override;
    void Render() override;
    void Update() override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() override { return L"文字显示"; }
};

class SettingsPage : public Page {
public:
    void Initialize(HWND parent) override;
    void Render() override;
    void Update() override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() override { return L"设置"; }
};