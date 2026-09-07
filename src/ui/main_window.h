#pragma once

#include <windows.h>

namespace lra {

class Application;

class MainWindow {
public:
    static bool registerClass(HINSTANCE instance);
    static HWND create(HINSTANCE instance, int showCommand, Application* app);

private:
    static LRESULT CALLBACK procedure(HWND, UINT, WPARAM, LPARAM);
};

} // namespace lra
