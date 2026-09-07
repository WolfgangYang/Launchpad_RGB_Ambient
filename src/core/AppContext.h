#pragma once

#include <windows.h>
#include "../Config/Config.h"
#include "../Effects/Effect.h"

namespace Core {

// AppContext 封装了程序运行时的所有动态状态。
// 通过将此对象传递给各个模块，我们实现了从“全局变量”到“显式依赖注入”的转变。
class AppContext {
public:
    AppContext() = default;

    // --- 配置与语言 (Configuration) ---
    Config::Language language = Config::English;

    // --- 动画与灯效状态 (Animation & Effect State) ---
    double animationPhase = 0.0;      // 用于驱动动画的连续时间值
    int brightness = 100;             // 当前全局亮度 (0-100)
    Effect::Type currentEffectType = Effect::Type::Rainbow;

    // --- 系统监控数据 (System Monitoring Data) ---
    // 这些数值由 SystemMonitor 模块定期更新，并供 UI 和灯效使用
    double cpuUsage = 0.0;
    double gpuUsage = 0.0;
    double ramUsage = 0.0;
    double temperature = 0.0;

    // --- 设备与窗口状态 (Device & Window State) ---
    bool isDeviceConnected = false;
    HWND mainWindow = nullptr;
    HWND statusLabel = nullptr; // 用于更新 UI 上的连接状态文本

    // --- 程序生命周期控制 ---
    bool shouldExit = false;

    // 更新动画相位（由主循环调用）
    void UpdateAnimation(double deltaTime) {
        animationPhase += deltaTime;
    }
};

} // namespace Core
