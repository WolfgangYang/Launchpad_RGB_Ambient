#pragma once

#include <windows.h>

namespace Core { class AppContext; } // Forward declaration

namespace UI {

class TextPage : public IPage {
public:
    TextPage() = default;
    ~TextPage() override = default;

    void Initialize(HWND parent, Core::AppContext& context) override;
    void Update(Core::AppContext& context) override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() const override { return L"文字显示"; }
    void Render() override;
};

} // namespace UI
