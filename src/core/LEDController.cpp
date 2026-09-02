#include "core/LEDController.h"
#include "midi/MidiEngine.h"
#include <vector>

namespace Core {

LEDController::~LEDController() {}

uint8_t LEDController::PositionToIndex(int x, int y) const {
    if (x < 0 || x > 7 || y < 0 || y > 7) return 0xFF;
    // Launchpad MK2 mapping: index = y * 8 + x
    return static_cast<uint8_t>(y * 8 + x);
}

void LEDController::SetLED(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return;

    uint8_t index = PositionToIndex(x, y);
    
    // Launchpad MK2 SysEx format for RGB:
    // F0 40 13 4B 01 [index] [R] [G] [B] F7
    std::vector<uint8_t> msg = {
        0xF0, 0x40, 0x13, 0x4B, 0x01, index, r, g, b, 0xF7
    };

    Midi::MidiEngine::SendSysEx(msg);
}

void LEDController::ClearAll() {
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            SetLED(x, y, 0, 0, 0);
        }
    }
}

} // namespace Core
