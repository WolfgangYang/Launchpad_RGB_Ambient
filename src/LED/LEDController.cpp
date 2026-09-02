// LED Controller Implementation
#include "LEDController.h"
#include <cmath>

void LEDController::SetLED(int x, int y, BYTE red, BYTE green, BYTE blue) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return;
    
    // SysEx message for RGB mode
    BYTE message[12] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0x0B, 
        static_cast<BYTE>(PositionToIndex(x, y)),
        red, green, blue, 0xF7
    };
    
    MIDIDevice::SendSysEx(message, sizeof(message));
}

void LEDController::ClearAll() {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            SetLED(x, y, 0, 0, 0);
        }
    }
}

BYTE LEDController::PositionToIndex(int x, int y) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return 0xFF;
    return static_cast<BYTE>(y * 8 + x);
}

void LEDController::RenderEffect(double phase, double brightness) {
    // This should be implemented in Effect classes
}