# Tower Physics for PSP

This repository contains the Tower Physics game and the legacy LTE engine it
uses. The game is the maintained product; old LTE tutorial/demo applications
and their media have been removed so there is one PSP application entry point.

## Layout

- `game/` — gameplay, PSP presentation, live tuning, and native tests.
- `include/` and `src/` — LTE engine headers and source.
- `libs/` — historical prebuilt PSP libraries.
- `GAME_DESIGN.md` — design direction and implemented milestones.
- `readme.txt`, `changes.txt`, `license.txt` — upstream LTE documentation and
  licensing history.

## Native tests

```sh
make -C game/tests test
```

The native suite builds with C++98, warnings-as-errors, and no PSP SDK. It
covers physics, game rules, menus, tuning persistence, procedural sound, the
audio callback adapter, weather, colours, and invalid-input robustness.

## PSP build

Install a compatible PSP toolchain/PSPSDK, then build the engine and game:

```sh
make -C src
make -C src install
make -C game
```

Copy the resulting `game/EBOOT.PBP` and `game/tuning.cfg` into the same PSP or
PPSSPP game directory. See `game/README.md` for controls and current limits.
