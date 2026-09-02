#pragma once
#include <windows.h>
#include "../Renderer/Renderer.h"

class Page {
public:
    virtual ~Page() {}
    virtual void Initialize(HWND parent) = 0;
    virtual void Render() = 0;
    virtual const wchar_t* GetTitle() = 0;
};

// --- 氛围灯效页面 ---
class EffectPage : public Page {
public:
    void Initialize(HWND parent) override {}
    void Render() override; // 实现见下文
    const wchar_t* GetTitle() override { return L"氛围灯效"; }
};

// --- 其他三个占位页面 ---
class StatusPage : public Page {
public:
    void Initialize(HWND parent) override {}
    void Render() override { Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f}); } // 简单的深色背景
    const wchar_t* GetTitle() override { return L"状态指示"; }
};

class TextPage : public Page {
public:
    void Initialize(HWND parent) override {}
    void Render() override { Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f}); }
    const wchar_t* GetTitle() override { return L"文字显示"; }
};

class SettingsPage : public Page {
public:
    void Initialize(HWND parent) override {}
    void Render() override { Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f}); }
    const wchar_t* GetTitle() override { return L"设置"; }
};
