// Page Base Module
// Defines the interface for all UI pages in Launchpad RGB Ambient

#pragma once

#include <windows.h>

namespace Core { class AppContext; } // Forward declaration to avoid circular dependency

namespace UI {

class IPage {
public:
    virtual ~IPage() = default;

    // Initialize page resources and sub-controls
    virtual void Initialize(HWND parent, Core::AppContext& context) = 0;

    // Update logic (animations, data polling) - Now accepts AppContext for decoupling
    virtual void Update(Core::AppContext& context) = 0;

    // Handle resize events
    virtual void OnResize(int width, int height) = 0;

    // Get the title of the page for UI display
    virtual const wchar_t* GetTitle() const = 0;

    // Render logic (if using Direct2D or custom drawing)
    virtual void Render() = 0;
};

} // namespace UI
