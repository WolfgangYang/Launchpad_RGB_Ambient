// LED Controller Module
#pragma once

#include <windows.h>
#include "MIDI/MIDIDevice.h"

class LEDController {
public:
    struct Position { int x, y; };
    
    static void SetLED(int x, int y, BYTE red, BYTE green, BYTE blue);
    static void ClearAll();
    static BYTE PositionToIndex(int x, int y);
    
private:
    static void SendRGBMessage(BYTE ledIndex, BYTE red, BYTE green, BYTE blue);
};