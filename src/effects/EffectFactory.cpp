#include "effects/EffectFactory.h"
#include <cmath>

namespace Core {

// --- Rainbow Effect Implementation ---
class RainbowEffect : public EffectBase {
public:
    void Update(float deltaTime) override {
        phase += deltaTime * speed;
        if (phase > 6.28318f) phase -= 6.28318f;
    }

    void Render(LEDController& controller, const EffectParams& params) override {
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                float hue = phase + (x + y) * 0.125f;
                // Simplified RGB calculation for demonstration
                uint8_t r = static_cast<uint8_t>(sin(hue) * 0.5 + 0.5 * params.brightness * 255);
                uint8_t g = static_cast<uint8_t>(sin(hue + 2.0f) * 0.5 + 0.5 * params.brightness * 255);
                uint8_t b = static_cast<uint8_t>(sin(hue + 4.0f) * 0.5 + 0.5 * params.brightness * 255);
                controller.SetLED(x, y, r, g, b);
            }
        }
    }

    const char* GetName() const override { return "Rainbow"; }

private:
    float phase = 0.0f;
    float speed = 1.0f;
};

// --- Factory Implementation ---
std::unique_ptr<EffectBase> EffectFactory::Create(EffectType type) {
    switch (type) {
        case EffectType::Rainbow: return std::make_unique<RainbowEffect>();
        // Other effects will be implemented in subsequent steps
        default: return nullptr;
    }
}

} // namespace Core
