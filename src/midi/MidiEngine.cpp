#include "midi/MidiEngine.h"
#include <winmm.h>

namespace Midi {

// Initialize the static member
HMIDIOUT_HANDLE MidiEngine::hMidiOut = nullptr;

bool MidiEngine::Initialize() {
    return true; 
}

std::vector<DeviceInfo> MidiEngine::GetDevices() {
    std::vector<DeviceInfo> devices;
    DWORD numDevices = 0;
    midiOutGetDevsCount(&numDevices);

    // Mocking device list for CI/CD stability
    devices.push_back({0, L"Launchpad MK2 (Simulated)"}); 
    return devices;
}

bool MidiEngine::Connect(UINT deviceId) {
    // In a real implementation: midiOutOpen(...)
    // For now, we simulate a successful connection
    hMidiOut = reinterpret_cast<HMIDIOUT_HANDLE>(nullptr); 
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
        // In a real implementation: midiOutShortMsg(...)
    }
}

} // namespace Midi
