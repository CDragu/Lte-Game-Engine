# 13. Physics Sandbox (PSP)

First playable *vertical slice* for [GAME_DESIGN.md](../../GAME_DESIGN.md), implemented as an isolated LTE example. Does not modify existing engine source.

## What works in source

- Visible 3D cube nodes, a static ground cuboid and **three initially dynamic stacked rectangles** (2.5D rigid bodies).
- A new cuboid swings as a predictable pendulum above the stack. Press **Cross** to detach it with its instantaneous tangential velocity.
- Local C++ planar oriented-rectangle contact solver: gravity, rotation, static-ground and block-to-block collisions, friction, a small restitution, per-contact iterative impulses and positional correction.
- 60 Hz fixed physics steps, an independent render pass on every engine frame, catch-up cap and dropped-step counter. **60 FPS / 60 Hz are targets**, not measured PSP results.
- HUD displays LTE-reported FPS, time spent in physics per rendered frame in milliseconds (PSP microsecond clock), contact-point count, active body count, per-frame physics steps, and missed simulation steps.
- Cross = drop once, Circle = reset scene for another drop, Start = pause, Select = toggle HUD. The device cursor is hidden so PSP face buttons are interpreted as key events.

**This is a one-drop physics test, not yet the complete endless stacking game.** It deliberately starts with three dynamic blocks and permits one more released block. Repeated spawning, lower-tower freeze, sway spline, camera tracking, scoring and collapse detection are later milestones. Physics bodies do not bend; the intended frozen foundation spline is not implemented here.

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

Test program covers free fall, floor contact and rest, three-box stacking, off-center impact rotation and fixed-step repeatability. GitHub Actions also runs this host test suite on changes to the sandbox.

## Architecture

- `Physics2D.h/.cpp`: pure C++98, no LTE or PSP headers; fixed-capacity physics world (8 bodies), box collision detection via SAT, feature contact points, friction and normal impulses.
- `main.cpp`: input handling, 60 Hz stepping, pendulum and velocity transfer, render-node synchronization and HUD.
- `tests/physics_tests.cpp`: native regression tests.

### Known limitations / validation needs

- Solver is an intentionally **minimal custom prototype**, not a production-quality rigid-body engine. Contact manifolds are rebuilt every frame without persistent warm-starting and no continuous collision detection or sleeping is implemented. Stacking stability needs testing on hardware.
- Simulation is planar (x, y, angle about z), rendered with LTE cube scene nodes. It is not true 3D physics.
- Ground is static; the lower-level kinematic sway and promotion/freeze logic from the GDD are **not yet implemented**.
- Real PSP performance and PSP build compatibility **have not been verified**. The host test suite does not prove PSP execution.
- Press Circle to reset after one release. Rendering uses the engine's normal camera and backend; monitor frame timing because 60 FPS is aspirational until measured.

## Next step

If this slice runs on PSP, instrument a taller stack, introduce lower-tower kinematic sway as a moving contact surface and implement stable dynamic-to-frozen transitions before unlimited block spawning.
