#include "EffectPage.h"
#include "../Effects/Effect.h"
#include "../LED/LEDController.h"
#include "../core/AppContext.h"

namespace UI {

class EffectPage : public IPage {
public:
    EffectPage() = default;
    ~EffectPage() override = default;

    void Initialize(HWND parent, Core::AppContext& context) override;
    void Update(Core::AppContext& context) override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() const override { return L"氛围灯效"; }
    void Render() override;
};

} // namespace UI
