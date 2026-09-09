#pragma once

#include "../core/types.h"
#include "../effects/effect_engine.h"
#include "../hardware/midi_output.h"
#include "../monitor/system_monitor.h"
#include <windows.h>

namespace lra {

class Application {
public:
    bool initialize(HWND window);
    void shutdown();

    void refreshMidiPorts(HWND combo);
    bool connectMidi(HWND combo, HWND statusLabel);
    void syncMidiPorts(HWND combo, HWND statusLabel);

    void setEffect(Effect effect);
    void setPaletteColor(int index);
    void startText(const std::wstring& content);
    void stopAll();
    void toggleCpu();
    void toggleGpu();
    void toggleRam();
    void toggleTemperature();

    void render();
    void updateStatistics();

    AppState& state() { return state_; }
    const AppState& state() const { return state_; }
    const LedFrame& frame() const { return frame_; }
    const FunctionKeyFrame& functionKeys() const { return functionKeys_; }

private:
    HWND window_ = nullptr;
    AppState state_{};
    MidiOutput midi_;
    EffectEngine effects_;
    SystemMonitor monitor_;
    LedFrame frame_{};
    FunctionKeyFrame functionKeys_{};
};

} // namespace lra
