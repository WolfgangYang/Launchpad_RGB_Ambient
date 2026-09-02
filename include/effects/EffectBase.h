#ifndef EFFECT_BASE_H
#define EFFECT_BASE_H

#include <cstdint>

namespace Core {

// Represents a value that can be either static or dynamically mapped from system stats
struct DynamicValue {
    float baseValue = 1.0f;      // The default/static value (e.g., brightness)
    float dynamicMultiplier = 1.0f; // How much the dynamic data affects this parameter
};

struct EffectParams {
    DynamicValue brightness{1.0f, 1.0f};
    DynamicValue speed{1.0f, 1.0f};
    DynamicValue intensity{1.0f, 1.0f}; // For effects like "Pulse" or "Wave height"
};

class EffectBase {
public:
    virtual ~EffectBase() = default;

    // Update the internal animation phase (time-based)
    virtual void Update(float deltaTime) = 0;

    // Render the effect onto the 8x8 grid using provided parameters and system stats
    virtual void Render(class LEDController& controller, const EffectParams& params, float dynamicDataFactor) = 0;

    virtual const char* GetName() const = 0;
};

} // namespace Core

#endif // EFFECT_BASE_H
