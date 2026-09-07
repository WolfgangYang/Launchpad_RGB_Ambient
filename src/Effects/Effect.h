// Effect Base Module
// Defines the interface for all Launchpad light effects

#pragma once

namespace Effect {

enum class Type {
    Rainbow,
    Breathe,
    Wave,
    Stars,
    Solid
};

class IEffect {
public:
    virtual ~IEffect() = default;
    
    // Render the effect to the Launchpad
    // phase: time-based parameter for animation (0.0 to 1.0+)
    // brightness: 0-100
    // indicators: flags for system status visualization
    virtual void Render(double phase, double brightness, 
                        bool cpuIndicator, bool gpuIndicator, 
                        bool ramIndicator, bool temperatureIndicator) = 0;
};

// Factory function to create effect instances
IEffect* CreateEffect(Type type);

} // namespace Effect
