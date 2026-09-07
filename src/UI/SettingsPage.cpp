#include "SettingsPage.h"
#include "../core/AppContext.h"

namespace UI {

void SettingsPage::Initialize(HWND parent, Core::AppContext& context) {}

void SettingsPage::Update(Core::AppContext& context) {}

void SettingsPage::OnResize(int width, int height) {}

const wchar_t* SettingsPage::GetTitle() const { return L"设置"; }

void SettingsPage::Render() {}

} // namespace UI
