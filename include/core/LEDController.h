#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <cstdint>
#include <vector>

namespace Core {

class LEDController {
public:
    LEDController() = default;
    ~LEDController();

    // Set a single LED color (0-7, 0-7)
    void SetLED(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    
    // Clear the entire 8x8 matrix
    void ClearAll();

private:
    uint8_t PositionToIndex(int x, int y) const;
};

} // namespace Core

#endif // LED_CONTROLLER_H
