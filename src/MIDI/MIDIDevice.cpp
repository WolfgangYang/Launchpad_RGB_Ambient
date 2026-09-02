// MIDI Device Implementation
#include "MIDIDevice.h"
#include <vector>

HMIDIOUT MIDIDevice::midiDevice = nullptr;
std::vector<MIDIDevice::DeviceInfo> MIDIDevice::devices;

bool MIDIDevice::Connect(UINT deviceId) {
    MMRESULT result = midiOutOpen(&midiDevice, deviceId, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        return false;
    }
    
    // Clear the LED matrix after connecting
    return true;
}

void MIDIDevice::Disconnect() {
    if (midiDevice) {
        midiOutReset(midiDevice);
        midiOutClose(midiDevice);
        midiDevice = nullptr;
    }
}

void MIDIDevice::Close() {
    Disconnect();
}

const std::vector<MIDIDevice::DeviceInfo>& MIDIDevice::GetDevices() {
    return devices;
}

void MIDIDevice::RefreshDevices() {
    devices.clear();
    
    UINT deviceCount = midiOutGetNumDevs();
    for (UINT i = 0; i < deviceCount; ++i) {
        MIDIOUTCAPSW caps{};
        if (midiOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            DeviceInfo info;
            info.id = i;
            info.name = std::wstring(caps.szPname);
            devices.push_back(info);
        }
    }
}

bool MIDIDevice::SendSysEx(const BYTE* data, size_t length) {
    if (!midiDevice || !data) return false;
    
    // Create MIDI header
    MidiHdr header{};
    header.dwFlags = 0;
    header.lpMidiMsg = const_cast<BYTE*>(data);
    header.wLength = static_cast<UINT>(length);
    
    MMRESULT result = midiOutPrepareHeader(midiDevice, &header, sizeof(header));
    if (result != MMSYSERR_NOERROR) {
        return false;
    }
    
    result = midiOutLongMsg(midiDevice, &header, sizeof(header));
    
    // Wait for completion
    DWORD startTime = GetTickCount();
    while (!(header.dwFlags & MHDR_DONE)) {
        if (GetTickCount() - startTime > 500) break;
        Sleep(1);
    }
    
    midiOutUnprepareHeader(midiDevice, &header, sizeof(header));
    return result == MMSYSYSERR_NOERROR;
}