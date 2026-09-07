#pragma once

#include <windows.h>

namespace Core { class AppContext; } // Forward declaration

namespace UI {

class SettingsPage : public IPage {
public:
    SettingsPage() = default;
    ~SettingsPage() override = default;

    void Initialize(HWND parent, Core::AppContext& context) override;
    void Update(Core::AppContext& context) override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() const override { return L"设置"; }
    void Render() override;
};

} // namespace UI
