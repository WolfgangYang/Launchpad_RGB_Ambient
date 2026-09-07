#pragma once

#include "../core/types.h"
#include <windows.h>

namespace lra {

void detectLanguage(AppState& state);
LPCWSTR text(Language language, const char* key);
LPCWSTR windowTitle();

} // namespace lra
