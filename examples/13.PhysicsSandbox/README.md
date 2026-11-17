# 13. Tower Physics: Menu + Physics Sandbox (PSP)

First playable *vertical slice* for [GAME_DESIGN.md](../../GAME_DESIGN.md), implemented as an isolated LTE example. Does not modify existing engine source.

## Main menu and navigation

On boot, the app displays a distinct 3D title scene with a decorative swinging block and **PLAY / EXIT** options. The **D-pad Up/Down** selects a menu item, **Cross** confirms. **Play** fades into a fresh physics sandbox scene (the confirming Cross event is consumed; it will not accidentally release the block). **Exit** closes the LTE application through `engineDevice::closeDevice()`.

During gameplay, **Triangle** returns to the menu without restarting the process; selecting Play again resets the simulation. Gamepad inputs use edge detection to avoid multiple transitions on a held button. Pausing stops physics time, and the physics simulation does not advance while viewing the menu.

## Modular gameplay HUD

The gameplay screen has an always-visible, right-aligned **BOXES STACKED** counter (starting at **3**, because the sandbox begins with three already stacked blocks). It only increases to **4** when the released block has made supporting contact with another tower block and remained sufficiently still for 0.35 simulated seconds. A block that only lands on the ground does not count. Pausing or returning to the menu freezes the count, and retrying/Play resets it to the starting tower.

The HUD is deliberately separated into `HudStats.h` (C++98 data model for up to four labelled integer counters), `HudRenderer.h/.cpp` (LTE-only display of a tile per registered counter), and `StackProgress.h` (pure C++98 gameplay rule for successful stacking, using `World::isSupportedBy()`). This keeps gameplay rules and PSP widgets independent. To expand the HUD, register another field with `hudStats.addCounter(L"SCORE",0)`, hold its returned ID, and call `hudStats.setValue(id,score)` as gameplay changes.

This counter measures **cumulative successful tower placements**, including the three seeded blocks; it does not decrease when blocks later topple. The sandbox still supports only one dropped block per run.

## Loss condition and game-over flow

The **top surface of the first (lowest) seeded tower block** is the failure reference plane (approximately `Y=1.02` with the current sample dimensions). This is the prototype's **spine/foundation plane**: the future spline-frozen foundation is *not yet implemented*. The plane is set at the beginning of each round and remains fixed for that round.

After each 60 Hz physics step, `TowerFailureRule.h` checks the **center height** of every dynamic block above that foundation block. If any center falls below the reference plane (with a 0.01 unit tolerance), the screen changes to **GAME OVER** and the simulation freezes. The foundation block itself is exempt, since its center naturally starts below its own top surface. This detects missed releases even when a box lands on the static ground, as well as an upper tower block falling off.

The GAME OVER overlay reports the failure, retains the current stacked-box counter, and offers **Cross or Circle to retry** (resetting physics + HUD counters) or **Triangle to return to the main menu**. The debug HUD displays the failure height as a red horizontal line; Select toggles this alongside the debug overlay.

## Live tuning (development feature)

**Hold PSP Left Trigger (L)** while playing or on the Game Over screen to show the live tuning overlay. Use **D-pad Up/Down** to select an entry and **Left/Right** to change the highlighted value. Buttons controlling drops, pause, retry and navigation are ignored while L is held. **Release L** to save the values automatically; an on-screen status says **TUNING SAVED** or **SAVE FAILED - CHECK GAME FOLDER**.

**Configuration:** `tuning.cfg` is loaded once at boot, and the *exact same relative path* is written when L is released, using normal file I/O. The sample config is version-controlled at `examples/13.PhysicsSandbox/tuning.cfg`. **When copying `EBOOT.PBP` to the PSP/PPSSPP game folder, copy `tuning.cfg` into that same folder.** The path is interpreted relative to the game's **current working directory**; PSP launchers that set a different working directory may require changing `tuningFile` in `main.cpp` to an explicit writable Memory Stick path. If the file is absent, compiled-in defaults are used and release of L attempts to create the file. Saving errors are shown rather than silently ignored. The saved file remains human-readable, ready to commit to Git.

**Changes take effect in-game:** physics gravity, block friction/restitution, linear/angular damping update on the next 60 Hz step. Pendulum amplitude and rate change live; swing phase is integrated to avoid jumping when speed changes. Settling time changes the duration needed for a dropped block to be credited, and **SCORE** now awards configured points per newly stacked block. The existing initial three blocks start the stack counter at 3, but do not grant score. The score and simulation reset when restarting; the tuning values don't.

Tweakable names: `gravity`, `friction`, `restitution`, `linear_damping`, `angular_damping`, `swing_amplitude`, `swing_speed`, `settling_time`, `score_per_block`. `Tuning.h/.cpp` contains the registry, allowed ranges, steps and plain-text parser. Add a registry entry and consumer for future variables; `TuningOverlay` renders all registered entries automatically.

This is a developer feature. For release, **package the tuned file alongside the EBOOT** or later generate baked-in defaults from the tuned `tuning.cfg`. Live settings are applied in real time while the simulation runs; the game can still fail while tuning.

## What works in source

- Visible 3D cube nodes, a static ground cuboid and **three initially dynamic stacked rectangles** (2.5D rigid bodies).
- A new cuboid swings as a predictable pendulum above the stack. Press **Cross** to detach it with its instantaneous tangential velocity.
- Local C++ planar oriented-rectangle contact solver: gravity, rotation, static-ground and block-to-block collisions, friction, a small restitution, per-contact iterative impulses and positional correction.
- 60 Hz fixed physics steps, an independent render pass on every engine frame, catch-up cap and dropped-step counter. **60 FPS / 60 Hz are targets**, not measured PSP results.
- A **persistent gameplay counter** displays boxes successfully stacked. Separately, the existing Select-toggle debug telemetry displays LTE-reported FPS, time spent in physics per rendered frame in milliseconds (PSP microsecond clock), contact-point count, active body count, per-frame physics steps, and missed simulation steps.
- Main menu: D-pad Up/Down = choose Play/Exit, Cross = confirm. In game: Cross = drop once, Circle = reset scene for another drop, Start = pause, Select = toggle HUD, Triangle = return to menu. The device cursor is hidden so PSP face buttons are interpreted as key events.

**This is a one-drop physics test, not yet the complete endless stacking game.** It deliberately starts with three dynamic blocks and permits one more released block. Repeated spawning, lower-tower freeze, sway spline, camera tracking, scoring and more advanced collapse detection are later milestones. Physics bodies do not bend; the intended frozen foundation spline is not implemented here.

## PSP build

Requires the historical PSP toolchain and LTE engine/static library described in the repository's `install.txt` and `readme.txt`.

```sh
cd src
make
make install
cd ../examples/13.PhysicsSandbox
make
```

The resulting `EBOOT.PBP` is intended for PSP homebrew execution / a PSP emulator. No external media files are required by the scene (built-in test cube meshes and font only). Note: the SDK/build are legacy and may need a compatible historical setup.

## Native solver tests (host C++ compiler)

```sh
cd examples/13.PhysicsSandbox/tests
make test
```

Native tests cover free fall, floor contact and rest, three-box stacking, off-center impact rotation, fixed-step repeatability, menu/navigation/game-over/retry transitions, HUD counters/stack detection, loss-plane conditions and tuning file read/write + live physics bindings. GitHub Actions runs all five suites on changes to the sandbox.

## Architecture

- `Physics2D.h/.cpp`: pure C++98, no LTE or PSP headers; fixed-capacity physics world (8 bodies), box collision detection via SAT, feature contact points, friction and normal impulses.
- `main.cpp`: 3D title screen, input handling, scene switching, 60 Hz stepping, pendulum and velocity transfer, render-node synchronization and orchestration.
- `HudStats.h`: independent HUD metric registry.
- `HudRenderer.h/.cpp`: PSP/LTE tile renderer for all registered metrics.
- `StackProgress.h`: counts only settled, tower-supported dropped cuboids.
- `TowerFailureRule.h`: native-testable foundation-plane loss rule.
- `Tuning.h/.cpp`: testable parameter registry and plain-text persistence.
- `TuningPhysics.h`: native-testable bridge for applying settings to the rigid-body world.
- `TuningOverlay.h/.cpp`: LTE-only held-L settings panel.
- `tuning.cfg`: editable defaults file shipped alongside EBOOT.PBP.
- `MenuFlow.h`: standalone, dependency-free menu state and navigation.
- `tests/physics_tests.cpp`, `tests/menu_tests.cpp`, `tests/hud_tests.cpp`, `tests/failure_tests.cpp` and `tests/tuning_tests.cpp`: native regression tests.

### Known limitations / validation needs

- Solver is an intentionally **minimal custom prototype**, not a production-quality rigid-body engine. Contact manifolds are rebuilt every frame without persistent warm-starting and no continuous collision detection or sleeping is implemented. Stacking stability needs testing on hardware.
- Simulation is planar (x, y, angle about z), rendered with LTE cube scene nodes. It is not true 3D physics.
- Ground is static; the lower-level kinematic sway and promotion/freeze logic from the GDD are **not yet implemented**.
- Real PSP performance and PSP build compatibility **have not been verified**. The host test suite does not prove PSP execution.
- Press Circle to reset after one release, or Triangle to return to the main menu; on GAME OVER press Cross/Circle to retry. Rendering uses the engine's normal camera and backend; monitor frame timing because 60 FPS is aspirational until measured.
- The menu/HUD/tuning overlay use built-in shapes and fonts. PSP execution, rendering, config path resolution and file write permissions still need verification on hardware or PPSSPP.

## Next step

If this slice runs on PSP, instrument a taller stack, introduce lower-tower kinematic sway as a moving contact surface and implement stable dynamic-to-frozen transitions before unlimited block spawning.
