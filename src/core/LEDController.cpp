#include "core/LEDController.h"
#include "midi/MidiEngine.h"

namespace Core {

LEDController::~LEDController() {}

uint8_t LEDController::PositionToIndex(int x, int y) const {
    // Launchpad MK2 8x8 Grid Mapping: index = y * 8 + x
    if (x < 0 || x > 7 || y < 0 || y > 7) return 0xFF; 
    return static_cast<uint8_t>(y * 8 + x);
}

void LEDController::SetLED(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    // Boundary Check: Prevent out-of-bounds access
    if (x < 0 || x > 7 || y < 0 || y > 7) return;

    uint8_t index = PositionToIndex(x, y);
    
    // Launchpad MK2 SysEx Protocol for RGB Control:
    // F0 40 13 4B 01 [index] [R] [G] [B] F7
    std::vector<unsigned char> msg = {
        0xF0, 0x40, 0x13, 0x4B, 0x01, index, r, g, b, 0xF7
    };

    MidiEngine::SendSysEx(msg);
}

void LEDController::ClearAll() {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            SetLED(x, y, 0, 0, 0);
        }
    }
}

} // namespace Core
