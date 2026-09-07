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
  RGB frame + 8 side-key   MIDI SysEx
      RGB values                |
          |                     v
          +----------------> Launchpad
```

## Dependency direction

- `core` has no dependency on other project modules.
- `effects` depends on `core`.
- `hardware` is isolated from effects and UI.
- `monitor` is isolated from effects and UI.
- `localization` owns user-facing strings.
- `app` composes the modules.
- `ui` talks to `app`; it does not implement effects or MIDI protocol.

## Right-side function keys

The effect engine produces eight additional RGB values, one for each right-side function/scene key. The first implementation mirrors `frame[row][7]`, so every effect and row-based indicator automatically stays synchronized.

`MidiOutput` maps these keys to the Launchpad MK2 IDs `19, 29, ..., 89` using the same RGB SysEx command family as the grid.

This is intentionally kept in the hardware layer so the effect engine does not need to know the Launchpad protocol.


## Launchpad MK2 function keys

The 8 right-side and 8 top-side round keys are rendered as part of the same effect frame. Right keys mirror the corresponding row; top keys mirror the corresponding column. MK2 top controllers are 104-111, and right-side controllers are 89,79,...,19.
