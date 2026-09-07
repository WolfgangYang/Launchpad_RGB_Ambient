#pragma once

#include "../core/types.h"
#include <array>

namespace lra {

using LedFrame = std::array<std::array<Rgb, 8>, 8>;

class EffectEngine {
public:
    void render(AppState& state, LedFrame& frame) const;

private:
    void renderBase(const AppState& state, LedFrame& frame) const;
    void renderIndicators(const AppState& state, LedFrame& frame) const;
};

} // namespace lra
