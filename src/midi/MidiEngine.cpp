#include "midi/MidiEngine.h"
#include <winmm.h>

namespace Midi {

// Correcting the type for MIDI output handle
HMIDIIMSMETHOD MidiEngine::hMidiOut = nullptr;

bool MidiEngine::Initialize() {
    return true; 
}

std::vector<DeviceInfo> MidiEngine::GetDevices() {
    std::vector<DeviceInfo> devices;
    // In a real implementation, we would use midiOutGetDevsCount and MIDI_OUT_GET_INFO
    // For this prototype, we provide a mock device to allow compilation.
    devices.push_back({0, L"Launchpad MK2 (Simulated)"}); 
    return devices;
}

bool MidiEngine::Connect(UINT deviceId) {
    // In real implementation: midiOutOpen(...)
    hMidiOut = reinterpret_cast<HMIDIIMSMETHOD>(nullptr); // Placeholder for actual handle
    return true;
}

void MidiEngine::Disconnect() {
    if (hMidiOut != nullptr) {
        // midiOutClose(hMidiOut);
        hMidiOut = nullptr;
    }
}

void MidiEngine::SendSysEx(const std::vector<unsigned char>& message) {
    if (hMidiOut != nullptr && !message.empty()) {
        // In real implementation: midiOutShortMsg(...)
    }
}

} // namespace Midi
