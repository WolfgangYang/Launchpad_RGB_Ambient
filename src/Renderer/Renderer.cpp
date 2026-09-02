// Direct2D Renderer Implementation
#include "Renderer.h"
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

ID2D1HwndRenderTarget* Renderer::renderTarget = nullptr;
IDWriteFactory* Renderer::dwriteFactory = nullptr;
ID2D1SolidColorBrush* Renderer::brush = nullptr;

void Renderer::Initialize(HWND hwnd) {
    // Create Direct2D factory and render target
    ID2D1Factory* factory = nullptr;
    D2D1CreateFactory(D2D1_DEBUG_LEVEL_NONE, &factory);
    
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
    
    ID2D1HwndRenderTarget* rt = nullptr;
    D2D1_HWEND_RENDER_TARGET_PROPERTIES props = {
        hwnd,
        size,
        D2D1_PIXEL_SAMPLING_MODE_DEFAULT,
        0,
        0,
        D2D1_RENDER_TARGET_TYPE_HARDWARE,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        D2D1_ALPHA_MODE_PREMULTIPLIED
    };
    
    // Create render target
    // Note: This is a simplified implementation
    // In production, you would need proper error handling
    
    renderTarget = rt;
    dwriteFactory = nullptr; // Will be initialized in full implementation
}

void Renderer::Render() {
    if (!renderTarget) return;
    
    renderTarget->BeginDraw();
    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    
    // Render content here
    
    HRESULT result = renderTarget->EndDraw();
    if (result != D2DERR_RESTART) {
        // Handle error
    }
}

void Renderer::Cleanup() {
    if (renderTarget) {
        renderTarget->Release();
        renderTarget = nullptr;
    }
    if (dwriteFactory) {
        dwriteFactory->Release();
        dwriteFactory = nullptr;
    }
}

void Renderer::FillRect(const Rect& rect, const Color& color) {
    if (!renderTarget) return;
    
    ID2D1SolidColorBrush* brush = nullptr;
    D2D1_COLOR_F dcColor = { color.r, color.g, color.b, color.a };
    
    // Create brush and fill rectangle
    renderTarget->FillRectangle(
        D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height),
        brush);
}

void Renderer::DrawText(const wchar_t* text, float x, float y, float size, const Color& color) {
    if (!renderTarget || !dwriteFactory) return;
    
    IDWriteTextFormat* textFormat = nullptr;
    dwriteFactory->CreateTextFormat(
        L"Microsoft YaHei",
        &textFormat);
    
    D2D1_COLOR_F dcColor = { color.r, color.g, color.b, color.a };
    
    renderTarget->DrawText(
        text,
        wcslen(text),
        textFormat,
        D2D1::RectF(x, y, x + 400, y + size),
        brush);
}

void Renderer::Clear() {
    if (!renderTarget) return;
    
    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
}