#include "Page.h"
#include "../Effects/Effect.h"
#include <vector>

// 这里我们用一个简单的全局变量来模拟当前选中的效果，实际开发中应由 App 管理
extern Effect* g_CurrentEffect; 

void EffectPage::Render() {
    Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f}); // 背景深灰色
    
    // 在这里调用效果渲染逻辑 (简化版)
    if (g_CurrentEffect) {
        g_CurrentEffect->Render(0.5, 80.0); // 这里传入固定值用于测试，实际应随时间变化
    }

    // 绘制一个简单的标题栏作为 UI 示意
    Renderer::FillRect({10, 10, 360, 40}, {0.2f, 0.2f, 0.2f, 1.0f});
    Renderer::DrawText(L"Effect Preview", 20, 15, 24, {1.0f, 1.0f, 1.0f, 1.0f});
}
