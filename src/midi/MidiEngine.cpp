#include "midi/MidiEngine.h"
#include <winmm.h>

namespace Midi {

MIDIinHandle MidiEngine::hMidiIn = nullptr;
UINT MidiEngine::selectedDeviceId = 0;

bool MidiEngine::Initialize() {
    return true; // WinMM initialization is often implicit, but can be added if needed
}

void MidiEngine::Shutdown() {
    if (hMidiIn) {
        midiOutClose(hMidiIn);
        hMidiIn = nullptr;
    }
}

std::vector<DeviceInfo> MidiEngine::GetDevices() {
    std::vector<DeviceInfo> devices;
    DWORD numDevices = 0;
    midiOutGetDevsCount(&numDevices);

    for (DWORD i = 0; i < numDevices; ++i) {
        MIDIPROCESS_INFO info; // Note: This is a simplification, real Win32 uses midiOutOpen/MIDI_OUT_GET_INFO
        // For brevity in this prototype, we'll assume standard device enumeration logic
    }
    
    // Placeholder for actual MIDI enumeration logic to keep the buildable structure
    devices.push_back({0, L"Launchpad MK2 (Simulated)"}); 
    return devices;
}

bool MidiEngine::Connect(UINT deviceId) {
    if (midiOutOpen(reinterpret_cast<HMIDIIMSMETHOD(&MidiEngine::Shutdown)(MIDIinHandle)>(&MidiEngine::Shutdown), &hMidiIn, MIDI_APPLY_DEVICE_TYPE | MIDI_OPENCM_TIME_REVERSED, 0) != MMSYSERR_NOERROR) {
        return false;
    }
    selectedDeviceId = deviceId;
    return true;
}

void MidiEngine::Disconnect() {
    Shutdown();
}

void MidiEngine::SendSysEx(const std::vector<BYTE>& message) {
    if (hMidiIn) {
        midiOutShortMsg(hMidiIn, 0); // Placeholder for actual SysEx implementation
    }
}

} // namespace Midi
