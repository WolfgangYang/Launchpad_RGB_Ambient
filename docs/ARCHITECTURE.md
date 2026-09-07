# Architecture

```text
                    +----------------------+
                    |      main.cpp        |
                    +----------+-----------+
                               |
                               v
                    +----------------------+
                    |    MainWindow (UI)   |
                    +----------+-----------+
                               |
                               v
                    +----------------------+
                    |     Application      |
                    |  state / orchestration|
                    +---+-------+-------+--+
                        |       |       |
          +-------------+       |       +-------------+
          v                     v                     v
   +-------------+      +---------------+      +-------------+
   | EffectEngine|      |  MidiOutput   |      |SystemMonitor|
   +------+------+      +-------+-------+      +-------------+
          |                     |
          v                     v
     LED frame             MIDI SysEx
          |
          +--------------------> Launchpad
```

## Dependency direction

- `core` has no dependency on other project modules.
- `effects` depends on `core`.
- `hardware` is isolated from effects and UI.
- `monitor` is isolated from effects and UI.
- `localization` owns user-facing strings.
- `app` composes the modules.
- `ui` talks to `app`; it does not implement effects or MIDI protocol.

This makes later changes safer. For example, adding a new effect should normally touch only `effects/` plus the effect selection enum/UI binding.

## Planned extension points

1. `EffectEngine`
   - new effect classes
   - palette/color configuration
   - per-effect parameters

2. `MidiOutput`
   - Launchpad model abstraction
   - batched SysEx/frame transmission
   - device capability detection

3. `SystemMonitor`
   - real GPU usage provider
   - real temperature provider
   - provider fallback/error state

4. `Application`
   - profiles/presets
   - persistent settings
   - update/render scheduling

5. `MainWindow`
   - grouped controls
   - selected-state visualization
   - richer configuration panels

## Important design rule

The LED effect should produce a frame of RGB values. The hardware layer should be responsible for converting that frame into device-specific MIDI messages. This prevents the animation logic from becoming tied to the Launchpad protocol.
