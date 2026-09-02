#include "LEDController.h"

void LEDController::SetLED(int x, int y, BYTE r, BYTE g, BYTE b) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return;
    BYTE msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0x0B, static_cast<BYTE>(PositionToIndex(x, y)), r, g, b, 0xF7 };
    MIDIDevice::SendSysEx(msg, sizeof(msg));
}

void LEDController::ClearAll() {
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) SetLED(x, y, 0, 0, 0);
}

BYTE LEDController::PositionToIndex(int x, int y) {
    return static_cast<BYTE>(y * 8 + x);
}
