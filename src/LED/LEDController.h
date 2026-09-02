#pragma once
#include <windows.h>
#include "../MIDI/MIDIDevice.h"

class LEDController {
public:
    static void SetLED(int x, int y, BYTE r, BYTE g, BYTE b);
    static void ClearAll();
    static BYTE PositionToIndex(int x, int y);
};
