#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>

namespace lra {

struct MidiPort {
    UINT deviceIndex = 0;
    std::wstring name;
};

class MidiOutput {
public:
    MidiOutput() = default;
    ~MidiOutput();

    MidiOutput(const MidiOutput&) = delete;
    MidiOutput& operator=(const MidiOutput&) = delete;

    std::vector<MidiPort> enumerate() const;
    bool open(UINT deviceIndex);
    void close();
    bool isOpen() const { return device_ != nullptr; }

    bool sendSysEx(const BYTE* data, DWORD length);
    bool setLed(int x, int y, BYTE red, BYTE green, BYTE blue);
    void clearGrid();

private:
    HMIDIOUT device_ = nullptr;

    static int launchpadLed(int x, int y);
};

} // namespace lra
