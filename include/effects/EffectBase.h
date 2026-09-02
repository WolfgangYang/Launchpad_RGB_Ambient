#ifndef EFFECT_BASE_H
#define EFFECT_BASE_H

#include <cstdint>

namespace Core {

// Forward declaration to break circular dependency
class LEDController;

struct EffectParams {
    float brightness = 1.0f;      // Base brightness (0.0 to 1.0)
    float speed = 1.0f;           // Base speed multiplier
    float intensity = 1.0f;        // Base intensity for pulse/wave effects
};

class EffectBase {
public:
    virtual ~EffectBase() = default;

    // Update the internal animation phase (time-based)
    virtual void Update(float deltaTime) = 0;

    // Render the effect onto the 8x8 grid using provided parameters and system stats
    virtual void Render(LEDController& controller, const EffectParams& params, float dynamicFactor) = 0;

    virtual const char* GetName() const = 0;
};

} // namespace Core

#endif // EFFECT_BASE_H
