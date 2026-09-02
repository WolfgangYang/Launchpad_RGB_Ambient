#ifndef MIDI_ENGINE_H
#define MIDI_ENGINE_H

#include <windows.h>
#include <string>
#include <vector>

namespace Midi {

struct DeviceInfo {
    UINT id;
    std::wstring name;
};

// Correctly define the handle type for MIDI output
typedef HMIDIIMSMETHOD HMIDIOUT_HANDLE;

class MidiEngine {
public:
    static bool Initialize();
    static void Shutdown();
    static std::vector<DeviceInfo> GetDevices();
    static bool Connect(UINT deviceId);
    static void Disconnect();
    static void SendSysEx(const std::vector<unsigned char>& message);

private:
    // Use the correct handle type for MIDI output
    static HMIDIOUT_HANDLE hMidiOut; 
};

} // namespace Midi

#endif // MIDI_ENGINE_H
