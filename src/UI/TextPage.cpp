#include "TextPage.h"
#include "../core/AppContext.h"

namespace UI {

void TextPage::Initialize(HWND parent, Core::AppContext& context) {}

void TextPage::Update(Core::AppContext& context) {}

void TextPage::OnResize(int width, int height) {}

const wchar_t* TextPage::GetTitle() const { return L"文字显示"; }

void TextPage::Render() {}

} // namespace UI
