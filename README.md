# Launchpad RGB Ambient v0.2

Novation Launchpad MK2 的 RGB 氛围灯控制软件。

## 📋 功能特性

- ✅ **四个页面**: 氛围灯效 / 状态指示 / 文字显示 / 设置
- ✅ **模块化架构**: 易于维护和扩展
- ✅ **现代UI**: Direct2D渲染 + Fluent Design风格
- ✅ **五种灯效**: 彩虹 / 呼吸 / 波纹 / 星空 / 纯色
- ✅ **系统监控**: CPU / RAM / GPU / 温度指示

## 🏗️ 架构设计

```
src/
├── main.cpp              # 程序入口 + 消息循环
├── MIDI/MIDIDevice.h     # MIDI通信层
├── LED/LEDController.h   # LED矩阵控制
├── Effects/Effect.h      # 效果基类和实现
├── UI/Page.h             # 页面基类和四个页面
├── Config/Config.h       # 配置管理
└── Renderer/Renderer.h    # Direct2D渲染引擎
```

## 🚀 构建方法

### 本地构建 (Windows)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### GitHub Actions
推送代码到GitHub后，Actions会自动编译并生成exe文件。

## 📦 依赖项

- Windows SDK
- DirectX 11+
- CMake 3.15+
- MSVC (Visual Studio 2019或更高版本)

## 📝 开发计划

### v0.2
- [x] 模块化架构重构
- [x] Direct2D渲染引擎
- [x] Fluent Design UI
- [ ] UI现代化完成
- [ ] 功能键同步发光
- [ ] GitHub编译配置完成

## 📄 许可证

MIT License