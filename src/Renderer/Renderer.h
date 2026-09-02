// Renderer Module
#pragma once

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

class Renderer {
public:
    struct Rect { float x, y, width, height; };
    struct Color { float r, g, b, a; };
    
    static void Initialize(HWND hwnd);
    static void Render();
    static void Cleanup();
    static void FillRect(const Rect& rect, const Color& color);
    static void DrawText(const wchar_t* text, float x, float y, float size, const Color& color);
    static void Clear();
    
private:
    static ID2D1HwndRenderTarget* renderTarget;
    static IDWriteFactory* dwriteFactory;
};