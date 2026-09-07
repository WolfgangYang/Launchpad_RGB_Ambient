# Launchpad RGB Ambient v0.3

Windows native C++17 application for controlling an RGB Launchpad over MIDI.

## v0.3 focus

v0.3 keeps the verified v0.3 behavior and adds two foundation changes:

- CMake is now the primary build system.
- The eight right-side Launchpad MK2 function/scene keys are included in the effect render and mirror the corresponding row's rightmost RGB pad.

## Current implementation

- MIDI output device enumeration
- Launchpad MK2 RGB SysEx output
- Rainbow / Breathe / Wave / Stars / Solid effects
- Brightness and speed controls
- CPU and RAM monitoring
- Separate GPU and temperature indicator paths (data sources are not faked yet)
- Chinese / English UI based on Windows UI language
- Right-side function keys synchronized with the RGB effects
- CMake + GitHub Actions Windows/MSVC build

## Build on GitHub

The repository includes `.github/workflows/build.yml` and `CMakeLists.txt`.

GitHub Actions configures a Visual Studio 2022 x64 CMake build and uploads the Release EXE as an artifact.

## Local build

Open a **Developer Command Prompt for Visual Studio** and run:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Architecture

See `docs/ARCHITECTURE.md`.


## Launchpad MK2 function keys

The 8 right-side and 8 top-side round keys are rendered as part of the same effect frame. Right keys mirror the corresponding row; top keys mirror the corresponding column. MK2 top controllers are 104-111, and right-side controllers are 89,79,...,19.
