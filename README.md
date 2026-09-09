# Launchpad RGB Ambient v0.5

Windows-native C++17 ambient-light controller for Novation Launchpad MK2.

## v0.5 changes

- Rainbow no longer depends on the selected palette color; the palette controls are hidden while Rainbow is active.
- System monitoring indicators use a defined overlay layout so CPU/RAM bars do not accidentally overwrite the GPU/temperature corner indicators.
- Added a basic English-letter text engine with horizontal scrolling.
- Text input explicitly accepts only `A-Z` / `a-z`.
- Added a Stop control to the Effects, System and Text pages. Stop clears all output and disables active indicators.
- Added a Settings option to hide the application to the Windows system tray when the window is closed.
- Updated application/project/build artifact version to 0.5.

## Existing features

- MIDI output-device enumeration with Windows software MIDI devices filtered out.
- Launchpad MK2 connection and runtime unplug handling.
- Rainbow, Breathe, Wave, Stars and Solid effects.
- Brightness and speed controls.
- CPU/RAM monitoring indicators; GPU/temperature paths remain placeholders until real data sources are implemented.
- Live desktop preview of the full Launchpad MK2 layout: 8x8 RGB grid, 8 right-side keys and 8 top-side keys.
- Chinese/English UI based on Windows UI language.
- CMake + GitHub Actions Windows/MSVC build.

## Build

```text
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```
