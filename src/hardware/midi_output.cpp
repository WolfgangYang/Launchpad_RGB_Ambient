```cpp
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

    for (UINT deviceIndex = 0; deviceIndex < count; ++deviceIndex) {
        MIDIOUTCAPSW caps{};
        const MMRESULT result = midiOutGetDevCapsW(
            deviceIndex,
            &caps,
            static_cast<UINT>(sizeof(caps))
        );

        if (result != MMSYSERR_NOERROR) {
            continue;
        }

        MidiPort port;
        port.deviceIndex = deviceIndex;
        port.name = caps.szPname;

        ports.push_back(port);
    }

    return ports;
}

bool MidiOutput::open(UINT deviceIndex)
{
    close();

    HMIDIOUT device = nullptr;

    const MMRESULT result = midiOutOpen(
        &device,
        deviceIndex,
        0,
        0,
        CALLBACK_NULL
    );

    if (result != MMSYSERR_NOERROR) {
        return false;
    }

    device_ = device;
    return true;
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
    header.lpData =
        reinterpret_cast<LPSTR>(const_cast<BYTE*>(data));
    header.dwBufferLength = length;

    MMRESULT result = midiOutPrepareHeader(
        device_,
        &header,
        sizeof(header)
    );

    if (result != MMSYSERR_NOERROR) {
        return false;
    }

    result = midiOutLongMsg(
        device_,
        &header,
        sizeof(header)
    );

    if (result != MMSYSERR_NOERROR) {
        midiOutUnprepareHeader(
            device_,
            &header,
            sizeof(header)
        );

        return false;
    }

    const DWORD start = GetTickCount();

    while ((header.dwFlags & MHDR_DONE) == 0) {
        if (GetTickCount() - start > 500) {
            break;
        }

        Sleep(1);
    }

    const bool completed =
        (header.dwFlags & MHDR_DONE) != 0;

    midiOutUnprepareHeader(
        device_,
        &header,
        sizeof(header)
    );

    return completed;
}

int MidiOutput::launchpadFunctionKey(int index)
{
    if (index < 0 || index > 7) {
        return 0;
    }

    // Physical order is top-to-bottom:
    // 89, 79, 69, 59, 49, 39, 29, 19.
    return 89 - index * 10;
}

int MidiOutput::launchpadTopFunctionKey(int index)
{
    if (index < 0 || index > 7) {
        return 0;
    }

    // Physical order is left-to-right:
    // 104, 105, 106, 107, 108, 109, 110, 111.
    return 104 + index;
}

int MidiOutput::launchpadLed(int x, int y)
{
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return 0;
    }

    return 11 + (7 - y) * 10 + x;
}

bool MidiOutput::setFunctionKey(
    int index,
    BYTE red,
    BYTE green,
    BYTE blue)
{
    const int led = launchpadFunctionKey(index);

    if (!device_ || led == 0) {
        return false;
    }

    BYTE message[12] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0B, 0x00, 0x00, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(led);
    message[8] = red;
    message[9] = green;
    message[10] = blue;

    return sendSysEx(message, sizeof(message));
}

bool MidiOutput::setFunctionKeyColor(
    int index,
    BYTE color)
{
    const int led = launchpadFunctionKey(index);

    if (!device_ || led == 0) {
        return false;
    }

    BYTE message[10] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0A, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(led);
    message[8] = static_cast<BYTE>(color & 0x7F);

    return sendSysEx(message, sizeof(message));
}

bool MidiOutput::setTopFunctionKey(
    int index,
    BYTE red,
    BYTE green,
    BYTE blue)
{
    const int led = launchpadTopFunctionKey(index);

    if (!device_ || led == 0) {
        return false;
    }

    BYTE message[12] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0B, 0x00, 0x00, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(led);
    message[8] = red;
    message[9] = green;
    message[10] = blue;

    return sendSysEx(message, sizeof(message));
}

bool MidiOutput::setTopFunctionKeyColor(
    int index,
    BYTE color)
{
    const int led = launchpadTopFunctionKey(index);

    if (!device_ || led == 0) {
        return false;
    }

    BYTE message[10] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0A, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(led);
    message[8] = static_cast<BYTE>(color & 0x7F);

    return sendSysEx(message, sizeof(message));
}

bool MidiOutput::setLed(
    int x,
    int y,
    BYTE red,
    BYTE green,
    BYTE blue)
{
    const int led = launchpadLed(x, y);

    if (!device_ || led == 0) {
        return false;
    }

    BYTE message[12] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0B, 0x00, 0x00, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(led);
    message[8] = red;
    message[9] = green;
    message[10] = blue;

    return sendSysEx(message, sizeof(message));
}

bool MidiOutput::setLedColor(
    int x,
    int y,
    BYTE color)
{
    const int led = launchpadLed(x, y);

    if (!device_ || led == 0) {
        return false;
    }

    BYTE message[10] = {
        0xF0, 0x00, 0x20, 0x29, 0x02, 0x18,
        0x0A, 0x00, 0x00, 0xF7
    };

    message[7] = static_cast<BYTE>(led);
    message[8] = static_cast<BYTE>(color & 0x7F);

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

    for (int i = 0; i < 8; ++i) {
        setFunctionKey(i, 0, 0, 0);
        setTopFunctionKey(i, 0, 0, 0);
    }
}

} // namespace lra
```
