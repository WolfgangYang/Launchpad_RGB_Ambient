#include "EffectPage.h"
#include "../Effects/Effect.h"
#include "../LED/LEDController.h"
#include "../core/AppContext.h"

namespace UI {

void EffectPage::Initialize(HWND parent, Core::AppContext& context) {
    // Placeholder for Direct2D initialization if needed in future
}

void EffectPage::Update(Core::AppContext& context) {
    // 从 Context 中获取实时数据并驱动灯效
    Effect::IEffect* effect = Effect::CreateEffect(context.currentEffectType);
    
    if (effect) {
        // 传入 context 中的所有参数，实现真正的解耦驱动
        effect->Render(context.animationPhase, 
                       static_cast<double>(context.brightness), 
                       false, false, false, false); // Indicators placeholder
        delete effect;
    }
}

void EffectPage::OnResize(int width, int height) {}

void EffectPage::Render() {}

const wchar_t* EffectPage::GetTitle() const { return L"氛围灯效"; }

} // namespace UI
