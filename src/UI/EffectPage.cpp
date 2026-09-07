#include "EffectPage.h"
#include "../Effects/Effect.h"
#include "../LED/LEDController.h"
#include "../core/AppContext.h"

namespace UI {

void EffectPage::Initialize(HWND parent, Core::AppContext& context) {}

void EffectPage::Update(Core::AppContext& context) {
    // 使用传入的 context，不再依赖全局变量
    Effect::IEffect* effect = Effect::CreateEffect(context.currentEffectType);
    
    if (effect) {
        effect->Render(context.animationPhase, 
                       static_cast<double>(context.brightness), 
                       false, false, false, false);
        delete effect;
    }
}

void EffectPage::OnResize(int width, int height) {}

void EffectPage::Render() {}

const wchar_t* EffectPage::GetTitle() const { return L"氛围灯效"; }

} // namespace UI
