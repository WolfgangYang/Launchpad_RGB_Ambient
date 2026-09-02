#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>

class MIDIDevice {
public:
    struct DeviceInfo {
        std::wstring name;
        UINT id;
    };
    static bool Connect(UINT deviceId);
    static void Disconnect();
    static const std::vector<DeviceInfo>& GetDevices();
    static void RefreshDevices();
    static bool SendSysEx(const BYTE* data, size_t length);
private:
    static HMIDIOUT midiDevice;
    static std::vector<DeviceInfo> devices;
};
