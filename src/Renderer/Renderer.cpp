#include "Renderer.h"
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

ID2D1Factory* Renderer::factory = nullptr;
ID2D1HwndRenderTarget* Renderer::renderTarget = nullptr;
IDWriteFactory* Renderer::dwriteFactory = nullptr;
ID2D1SolidColorBrush* Renderer::brush = nullptr;

void Renderer::Initialize(HWND hwnd) {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwriteFactory);

    RECT rc;
    GetClientRect(hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    factory->CreateHwndRenderTarget(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::HwndRenderTargetProperties(),
        size, &renderTarget
    );

    renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);
}

void Renderer::BeginDraw() { renderTarget->BeginDraw(); }
void Renderer::EndDraw() { renderTarget->EndDraw(); }
void Renderer::Clear(const Color& color) { 
    renderTarget->Clear(D2D1::ColorF(color.r, color.g, color.b, color.a)); 
}

void Renderer::FillRect(const Rect& rect, const Color& color) {
    brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    renderTarget->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height), brush);
}

void Renderer::DrawText(const wchar_t* text, float x, float y, float size, const Color& color) {
    IDWriteTextFormat* textFormat = nullptr;
    dwriteFactory->CreateTextFormat(L"Segoe UI", NULL, D2D1_FONT_WEIGHT_NORMAL, 
        D2D1_FONT_STYLE_NORMAL, D2D1_FONT_STRETCH_NORMAL, size, L"zh-cn", &textFormat);

    brush->SetColor(D2D1::ColorF(color.r, color.g, color.b, color.a));
    renderTarget->DrawText(text, (UINT32)wcslen(text), textFormat, 
        D2D1::RectF(x, y, x + 500, y + size * 2), brush);

    textFormat->Release();
}

void Renderer::Cleanup() {
    if (brush) brush->Release();
    if (renderTarget) renderTarget->Release();
    if (dwriteFactory) dwriteFactory->Release();
    if (factory) factory->Release();
}
