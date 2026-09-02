#ifndef EFFECT_FACTORY_H
#define EFFECT_FACTORY_H

#include "effects/EffectBase.h"
#include <memory>
#include <string>

namespace Core {

enum class EffectType {
    Rainbow,
    Breathe,
    Wave,
    Stars,
    Solid
};

class EffectFactory {
public:
    static std::unique_ptr<EffectBase> Create(EffectType type);
};

} // namespace Core

#endif // EFFECT_FACTORY_H
