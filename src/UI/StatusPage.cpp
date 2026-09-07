#include "StatusPage.h"
#include "../core/AppContext.h"

namespace UI {

void StatusPage::Initialize(HWND parent, Core::AppContext& context) {}

void StatusPage::Update(Core::AppContext& context) {
    // 逻辑已解耦，通过 context 获取数据
}

void StatusPage::OnResize(int width, int height) {}

const wchar_t* StatusPage::GetTitle() const { return L"状态指示"; }

void StatusPage::Render() {}

} // namespace UI
