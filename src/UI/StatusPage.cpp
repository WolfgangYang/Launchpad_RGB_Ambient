#include "StatusPage.h"
#include "../core/AppContext.h"

namespace UI {

void StatusPage::Initialize(HWND parent, Core::AppContext& context) {}

void StatusPage::Update(Core::AppContext& context) {
    // 状态页现在可以根据 context 中的 CPU/RAM 数据进行逻辑处理
}

void StatusPage::OnResize(int width, int height) {}

const wchar_t* StatusPage::GetTitle() const { return L"状态指示"; }

void StatusPage::Render() {}

} // namespace UI
