# Launchpad RGB Ambient v0.4

Windows-native C++17 ambient-light controller for Novation Launchpad MK2.

## v0.4 changes

- Added a live desktop preview of the full Launchpad MK2 layout:
  - 8x8 RGB grid
  - 8 right-side function keys
  - 8 top-side function keys
- Preview and hardware rendering are driven from the same rendered frame.
- Added tabbed UI structure for future features: Effects, System, Text, Device, Settings.
- Added the Launchpad MK2 standard 128-color palette instead of an arbitrary RGB picker.
- Effects use the selected palette color where applicable.
- Final rendered colors are quantized to the MK2's 128-color palette so the preview models the device's practical color gamut.
- Kept the existing MIDI RGB SysEx output path for hardware compatibility.

## Existing features

- MIDI output-device enumeration and Launchpad MK2 connection.
- Rainbow, Breathe, Wave, Stars and Solid effects.
- Brightness and speed controls.
- CPU/RAM monitoring indicators.
- GPU/temperature indicator paths remain placeholders until their data sources are implemented.
- Chinese/English UI based on Windows UI language.
- CMake + GitHub Actions Windows/MSVC build.

## Build

```text
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```
