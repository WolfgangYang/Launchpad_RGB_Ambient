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
    static void SendSysEx(const std::vector<BYTE>& message);

private:
    static MIDIinHandle hMidiIn;
    static UINT selectedDeviceId;
};

} // namespace Midi

#endif // MIDI_ENGINE_H
