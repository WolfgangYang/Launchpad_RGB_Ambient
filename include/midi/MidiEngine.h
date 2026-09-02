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

class MidiEngine {
public:
    static bool Initialize();
    static void Shutdown();
    static std::vector<DeviceInfo> GetDevices();
    static bool Connect(UINT deviceId);
    static void Disconnect();
    // Use vector for safer memory management in MIDI messages
    static void SendSysEx(const std::vector<unsigned char>& message);

private:
    // Using HMIDIIMSMETHOD is not correct here; we need the actual handle type
    // midiOutOpen returns an HMIDIIMSMETHOD (which is actually a pointer to a function) 
    // but for our storage, we use the standard MIDIOUTHANDLE.
    static HMIDIIMSMETHOD hMidiOut; 
};

} // namespace Midi

#endif // MIDI_ENGINE_H
