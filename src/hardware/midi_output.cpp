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

        if (midiOutGetDevCapsW(
                i,
                &caps,
                sizeof(caps)) != MMSYSERR_NOERROR) {
            continue;
        }

        const std::wstring name = caps.szPname;

        // These are Windows software MIDI devices and are not useful
        // as physical Launchpad outputs.
        if (name == L"Microsoft GS Wavetable Synth" ||
            name == L"Microsoft MIDI Mapper") {
            continue;
        }

        ports.push_back({ i, name });
    }

    return ports;
}

bool MidiOutput::open(UINT deviceIndex)
{
    close();

    HMIDIOUT newDevice = nullptr;

    const MMRESULT result = midiOutOpen(
        &newDevice,
        deviceIndex,
        0,
        0,
        CALLBACK_NULL
    );

    if (result != MMSYSERR_NOERROR) {
        device_ = nullptr;
        return false;
    }

    device_ = newDevice;
    return true;
}

void MidiOutput::close()
{
    if (!device_) {
        return;
    }

    // Only clear the Launchpad while the device is still valid.
    // If the device was already unplugged, invalidateDevice()
    // has already cleared device_ and this path is skipped.
    clearGrid();

    if (device_) {
        midiOutReset(device_);
        midiOutClose(device_);
        device_ = nullptr;
    }
}

void MidiOutput::invalidateDevice()
{
    if (!device_) {
        return;
    }

    // The physical device is no longer usable. Do not call
    // clearGrid() here: that would attempt another 80 MIDI messages
    // against an already disconnected device.
    HMIDIOUT device = device_;
    device_ = nullptr;

    midiOutReset(device);
    midiOutClose(device);
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

    if (midiOutPrepareHeader(
            device_,
            &header,
            sizeof(header)) != MMSYSERR_NOERROR) {
        invalidateDevice();
        return false;
    }

    const MMRESULT result =
        midiOutLongMsg(device_, &header, sizeof(header));

    // If the driver immediately reports an error, the device is
    // most likely no longer available.
    if (result != MMSYSERR_NOERROR) {
        midiOutUnprepareHeader(
            device_,
            &header,
            sizeof(header));

        invalidateDevice();
        return false;
    }

    const DWORD start = GetTickCount();

    while ((header.dwFlags & MHDR_DONE) == 0) {
        if (GetTickCount() - start > 500) {
            midiOutUnprepareHeader(
                device_,
                &header,
                sizeof(header));

            // Treat a long-stuck SysEx transfer as a disconnected
            // or unusable device instead of blocking the UI forever.
            invalidateDevice();
            return false;
        }

        Sleep(1);
    }

    const bool completed =
        (header.dwFlags & MHDR_DONE) != 0;

    midiOutUnprepareHeader(
        device_,
        &header,
        sizeof(header));

    if (!completed) {
        invalidateDevice();
        return false;
    }

    return true;
}

int MidiOutput::launchpadFunctionKey(int index)
{
    if (index < 0 || index > 7) {
        return 0;
    }

    // Physical order is top-to-bottom: 89, 79, ..., 19.
    return 89 - index * 10;
}

int MidiOutput::launchpadTopFunctionKey(int index)
{
    if (index < 0 || index > 7) {
        return 0;
    }

    // Physical order is left-to-right: 104, 105, ..., 111.
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

bool MidiOutput::sendFrame(const std::array<std::array<BYTE, 3>, 80>& colors)
{
    if (!device_) {
        return false;
    }

    // Launchpad MK2 accepts up to 80 RGB SysEx LED updates.
    // Send the complete frame as one MIDI data block instead of
    // blocking the UI once for every LED.
    std::array<BYTE, 12 * 80> data{};
    for (int i = 0; i < 80; ++i) {
        const std::size_t offset = static_cast<std::size_t>(i) * 12;
        data[offset + 0] = 0xF0;
        data[offset + 1] = 0x00;
        data[offset + 2] = 0x20;
        data[offset + 3] = 0x29;
        data[offset + 4] = 0x02;
        data[offset + 5] = 0x18;
        data[offset + 6] = 0x0B;
        data[offset + 7] = static_cast<BYTE>(
            i < 64 ? launchpadLed(i % 8, i / 8)
                   : (i < 72 ? launchpadFunctionKey(i - 64)
                             : launchpadTopFunctionKey(i - 72)));
        data[offset + 8] = colors[i][0];
        data[offset + 9] = colors[i][1];
        data[offset + 10] = colors[i][2];
        data[offset + 11] = 0xF7;
    }

    return sendSysEx(data.data(), static_cast<DWORD>(data.size()));
}

void MidiOutput::clearGrid()
{
    if (!device_) {
        return;
    }

    std::array<std::array<BYTE, 3>, 80> colors{};
    sendFrame(colors);
}

} // namespace lra
