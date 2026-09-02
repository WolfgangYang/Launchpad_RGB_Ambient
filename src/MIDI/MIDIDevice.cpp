#include "MIDIDevice.h"

HMIDIOUT MIDIDevice::midiDevice = nullptr;
std::vector<MIDIDevice::DeviceInfo> MIDIDevice::devices;

bool MIDIDevice::Connect(UINT deviceId) {
    if (midiOutOpen(&midiDevice, deviceId, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) return false;
    return true;
}

void MIDIDevice::Disconnect() {
    if (midiDevice) { midiOutReset(midiDevice); midiOutClose(midiDevice); midiDevice = nullptr; }
}

const std::vector<MIDIDevice::DeviceInfo>& MIDIDevice::GetDevices() { return devices; }

void MIDIDevice::RefreshDevices() {
    devices.clear();
    UINT count = midiOutGetNumDevs();
    for (UINT i = 0; i < count; ++i) {
        MIDIOUTCAPSW caps{};
        if (midiOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            devices.push_back({ std::wstring(caps.szPname), i });
        }
    }
}

bool MIDIDevice::SendSysEx(const BYTE* data, size_t length) {
    if (!midiDevice || !data) return false;
    MidiHdr header{};
    header.dwFlags = 0;
    header.lpMidiMsg = const_cast<BYTE*>(data);
    header.wLength = static_cast<UINT>(length);
    if (midiOutPrepareHeader(midiDevice, &header, sizeof(header)) != MMSYSERR_NOERROR) return false;
    midiOutLongMsg(midiDevice, &header, sizeof(header));
    while (!(header.dwFlags & MHDR_DONE)) Sleep(1);
    midiOutUnprepareHeader(midiDevice, &header, sizeof(header));
    return true;
}
