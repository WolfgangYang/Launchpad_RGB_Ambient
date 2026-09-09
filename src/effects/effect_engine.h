#pragma once

#include "../core/types.h"
#include <array>

namespace lra {

using LedFrame = std::array<std::array<Rgb, 8>, 8>;
using FunctionKeyFrame = std::array<Rgb, 16>;
// [0..7] = right-side keys, top-to-bottom; [8..15] = top-side keys, left-to-right.

class EffectEngine {
public:
    void render(AppState& state, LedFrame& frame, FunctionKeyFrame& functionKeys) const;

private:
    void renderBase(const AppState& state, LedFrame& frame) const;
    void renderIndicators(const AppState& state, LedFrame& frame) const;
    void renderFunctionKeys(const AppState& state, const LedFrame& frame, FunctionKeyFrame& functionKeys) const;
};

} // namespace lra
