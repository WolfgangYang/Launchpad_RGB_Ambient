// LED Controller Module
// Handles low-level LED control for Launchpad

#pragma once

#include <windows.h>
#include "../MIDI/MIDIDevice.h"

class LEDController {
public:
    static BYTE PositionToIndex(int x, int y);
    static void SetLED(int x, int y, BYTE red, BYTE green, BYTE blue);
    static void ClearAll();
};
