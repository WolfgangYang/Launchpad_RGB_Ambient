# Architecture

## v0.4 rendering model

```text
                 Effect Engine
                       |
                       v
                +--------------+
                | Launchpad    |
                | Frame State  |
                +--------------+
                  /          \
                 v            v
            Desktop Preview  MIDI Output
```

The 8x8 grid and the 16 function keys are rendered into one frame. The desktop preview reads that same frame, while the MIDI layer sends it to the physical Launchpad. This keeps future text, icons, audio, monitoring and other renderers from needing separate preview logic.

## Color model

Launchpad MK2 has a built-in 128-color palette. v0.4 stores the selected palette index in `AppState::paletteIndex`. Effects may generate intermediate RGB values internally, but the final frame is quantized to the 128-color palette before both preview and MIDI output consume it.

The palette RGB values are desktop-preview approximations of the hardware colors; the palette index remains the authoritative user-facing color choice.

## UI structure

The Win32 UI is organized into five tab pages:

- Effects: effect selection, monitoring toggles, brightness, speed and the 128-color palette.
- System: reserved for future system-monitoring controls.
- Text: reserved for future text/icon rendering.
- Device: reserved for device/MIDI options.
- Settings: reserved for application settings.
