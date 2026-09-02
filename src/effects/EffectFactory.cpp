#include "effects/EffectFactory.h"
#include "core/LEDController.h"
#include <cmath>

namespace Core {

// --- Rainbow Effect Implementation ---
class RainbowEffect : public EffectBase {
public:
    void Update(float deltaTime) override {
        phase += deltaTime * speed;
        if (phase > 6.28318f) phase -= 6.28318f;
    }

    void Render(LEDController& controller, const EffectParams& params, float dynamicFactor) override {
        // Apply the dynamic factor to the base speed for visual feedback
        float effectiveSpeed = params.speed * (1.0f + dynamicFactor);
        
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                // Use the modulated phase for color calculation
                float hue = fmod(phase * effectiveSpeed + (x + y) * 0.125f, 1.0f);
                if (hue < 0) hue += 1.0f;

                float r_val, g_val, b_val;
                int sector = static_cast<int>(hue * 6) % 6;
                float fraction = hue * 6 - sector;
                float base_v = params.brightness; // Use the brightness parameter

                switch (sector) {
                    case 0: r_val = base_v; g_val = base_v * (1.0f - 0.8f); b_val = base_v * (1.0f - 0.8f * fraction); break;
                    case 1: r_val = base_v * (1.0f - 0.8f * (1.0f - fraction)); g_val = base_v; b_val = base_v * (1.0f - 0.8f); break;
                    case 2: r_val = base_v * (1.0f - 0.8f); g_val = base_v; b_val = base_v * fraction; break;
                    case 3: r_val = base_v * (1.0f - 0.8f * fraction); g_val = base_v; b_val = base_v * (1.0f - 0.8f); break;
                    case 4: r_val = base_v * (1.0f - 0.8f); g_val = base_v * fraction; b_val = base_v; break;
                    default: r_val = base_v; g_val = base_v * 0.2f; b_val = base_v; break;
                }

                controller.SetLED(x, y, static_cast<uint8_t>(r_val * 63), 
                                    static_cast<uint8_t>(g_val * 63), 
                                    static_cast<uint8_t>(b_val * 63));
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
        default: return nullptr;
    }
}

} // namespace Core
