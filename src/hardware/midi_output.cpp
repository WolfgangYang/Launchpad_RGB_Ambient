#include "midi_output.h"

#include <cstring>

namespace lra {

MidiOutput::~MidiOutput()
{
    close();
}

std::vector<MidiPort> MidiOutput::enumerate() const
{
    std::vector<MidiPort> ports;
    const UINT count = midiOutGetNumDevs();

    for (UINT i = 0; i < count; ++i) {
        MIDIOUTCAPSW caps{};
        if (midiOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            ports.push_back({i, caps.szPname});
        }
    }
    return ports;
}

bool MidiOutput::open(UINT deviceIndex)
{
    close();
    return midiOutOpen(&device_, deviceIndex, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR;
}

void MidiOutput::close()
{
    if (!device_) {
        return;
    }

    clearGrid();
    midiOutReset(device_);
    midiOutClose(device_);
    device_ = nullptr;
}

bool MidiOutput::sendSysEx(const BYTE* data, DWORD length)
{
    if (!device_ || !data || length == 0) {
        return false;
    }

    MIDIHDR header{};
    header.lpData = reinterpret_cast<LPSTR>(const_cast<BYTE*>(data));
    header.dwBufferLength = length;

    if (midiOutPrepareHeader(device_, &header, sizeof(header)) != MMSYSERR_NOERROR) {
        return false;
    }

    const MMRESULT result = midiOutLongMsg(device_, &header, sizeof(header));

    const DWORD start = GetTickCount();
    while ((header.dwFlags & MHDR_DONE) == 0) {
        if (GetTickCount() - start > 500) {
            break;
        }
        Sleep(1);
    }

    midiOutUnprepareHeader(device_, &header, sizeof(header));
    return result == MMSYSERR_NOERROR && (header.dwFlags & MHDR_DONE) != 0;
}

int MidiOutput::launchpadLed(int x, int y)
{
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return 0;
    }
    return 11 + (7 - y) * 10 + x;
}

bool MidiOutput::setLed(int x, int y, BYTE red, BYTE green, BYTE blue)
{
    BYTE message[12] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0B, 0x00, 0x00, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(launchpadLed(x, y));
    message[8] = red;
    message[9] = green;
    message[10] = blue;

    return sendSysEx(message, sizeof(message));
}

void MidiOutput::clearGrid()
{
    if (!device_) {
        return;
    }

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            setLed(x, y, 0, 0, 0);
        }
    }
}

} // namespace lra
