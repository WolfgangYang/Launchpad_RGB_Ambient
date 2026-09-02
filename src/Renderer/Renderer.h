#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>

class Renderer {
public:
    struct Rect { float x, y, width, height; };
    struct Color { float r, g, b, a; };

    static void Initialize(HWND hwnd);
    static void BeginDraw();
    static void EndDraw();
    static void Clear(const Color& color);
    
    // 绘制基础形状
    static void FillRect(const Rect& rect, const Color& color);
    static void DrawText(const wchar_t* text, float x, float y, float size, const Color& color);

private:
    static ID2D1Factory* factory;
    static ID2D1HwndRenderTarget* renderTarget;
    static IDWriteFactory* dwriteFactory;
    static ID2D1SolidColorBrush* brush;
};
