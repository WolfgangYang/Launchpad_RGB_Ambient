#pragma once

#include <windows.h>

namespace Core { class AppContext; } // Forward declaration

namespace UI {

class StatusPage : public IPage {
public:
    ~StatusPage() override = default;
    void Initialize(HWND parent, Core::AppContext& context) override;
    void Update(Core::AppContext& context) override;
    void OnResize(int width, int height) override;
    const wchar_t* GetTitle() const override;
    void Render() override;
};

} // namespace UI
