# Launchpad RGB Ambient v0.2

Windows native C++17 application for controlling an RGB Launchpad over MIDI.

## Goals of v0.2

v0.2 restructures the original v0.1 single-file prototype into independent modules:

- `app/` — application orchestration and state transitions
- `core/` — shared data types and color conversion
- `effects/` — LED animation generation
- `hardware/` — MIDI / Launchpad output
- `localization/` — language selection and UI strings
- `monitor/` — CPU/RAM/GPU/temperature data sources
- `ui/` — Win32 window and controls

The module boundaries are intentional: effects do not know about Win32 controls, and the UI does not know the Launchpad SysEx protocol.

## Current implementation

- MIDI output device enumeration
- Launchpad MK2 RGB SysEx output
- Rainbow / Breathe / Wave / Stars / Solid effects
- Brightness and speed controls
- CPU and RAM monitoring
- Separate GPU and temperature indicator paths (data sources are not faked yet)
- Chinese / English UI based on Windows UI language
- GitHub Actions Windows/MSVC build

## Build on GitHub

The repository includes:

`.github/workflows/build.yml`

The workflow builds every `.cpp` file with MSVC and packages the resulting EXE as a workflow artifact.

No third-party runtime or package manager is required.

## Local build

Open a **Developer Command Prompt for Visual Studio** and run:

```bat
cl /nologo /O2 /std:c++17 /EHsc /utf-8 ^
  src\main.cpp ^
  src\app\application.cpp ^
  src\core\color.cpp ^
  src\effects\effect_engine.cpp ^
  src\hardware\midi_output.cpp ^
  src\localization\localization.cpp ^
  src\monitor\system_monitor.cpp ^
  src\ui\main_window.cpp ^
  /link user32.lib gdi32.lib winmm.lib comctl32.lib pdh.lib ^
  /SUBSYSTEM:WINDOWS /OUT:Launchpad_RGB_Ambient_v0.2.exe
```

## Architecture

See `docs/ARCHITECTURE.md`.

## Scope note

The current renderer intentionally keeps the original v0.1 visual behavior close to the prototype. The point of this version is to establish a clean foundation before adding more effects, profiles, mappings, hardware-specific monitoring and a richer UI.
