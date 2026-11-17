# LTE Game Engine — architecture and onboarding map

> Source inspection: 2026-11-17, repository `CDragu/Lte-Game-Engine`, default branch `master`. This is a source-reading guide, **not** a claim that a PSP build or emulator test passed. Use the links and paths below to verify code after future changes.

## Executive summary

LTE Game Engine 2.2 is a **C++ PSP-oriented game engine based on Irrlicht 1.0** (`readme.txt`). The application interface follows the Irrlicht-style device / video driver / scene manager / GUI environment pattern. The PSP SDK and a custom bundled PSPGL version are required for building. The README warns of approximately 24 MB available to homebrew applications; treat that as a historical target, not a verified limit for all PSP models or firmware versions.

**Important physics distinction:** the repository contains **scene collision detection, triangle picking, ellipsoid-against-world collision response, gravity and sliding**, but the inspected engine does **not show a general rigid-body dynamics engine** (such as Bullet or ODE) with forces, masses, impulses, contacts between freely moving boxes, and stacking. Do not describe the existing collision-response animator as full box physics.

**Important entry-point distinction:** the examples use `int engineMain(unsigned int argc, void *argv)`, not a conventional `main()`. There are multiple example entry points; there is not one game `main.cpp` at the repository root. No verified example of spawning dynamically simulated blocks onto a plane was found in the inspected tree.

## Directory map

```text
Lte-Game-Engine/
├── readme.txt                Engine 2.2 overview, historical requirements, license
├── install.txt               Build/install instructions
├── changes.txt               Historical changelog
├── license.txt               LGPL license information
├── include/                  Public engine interfaces, math, scene, GUI, events
│   ├── engine.h
│   ├── engineDevice.h
│   ├── ISceneManager.h
│   ├── ISceneNode.h
│   ├── ISceneCollisionManager.h
│   ├── ISceneNodeAnimatorCollisionResponse.h
│   ├── IVideoDriver.h
│   ├── IEventReceiver.h
│   └── Keycodes.h
├── src/                      Engine implementations and vendor dependencies
│   ├── engine.cpp
│   ├── CengineDeviceWin32.cpp    Despite name: PSP controller/driver code
│   ├── CengineDeviceStub.cpp
│   ├── CSceneManager.cpp
│   ├── CSceneCollisionManager.cpp
│   ├── CSceneNodeAnimatorCollisionResponse.cpp
│   ├── CCameraFPSSceneNode.cpp
│   ├── CSoftwareDriver2.cpp
│   ├── COpenGLDriver.cpp
│   ├── C*SceneNode.cpp          Various renderable scene nodes
│   ├── audio/                   Audio backend
│   ├── jpeglib/, libpng/, zlib/ Media/image support
│   └── Makefile                 libengine.a
├── examples/
│   ├── common.h                 setupPSP() and callbacks
│   ├── 01.HelloWorld/            Basic device/scene/render loop
│   ├── 02.Quake3Map/             Level loading
│   ├── 03.CustomSceneNode/       Custom rendering node
│   ├── 04.Movement/              Event receiver and animators
│   ├── 05.UserInterface/         GUI
│   ├── 06.2DGraphics/           2D rendering
│   ├── 07.Collision/            Collision, gravity, picking
│   ├── 08.SpecialFX/           Effects
│   ├── 09.TerrainRendering/    Terrain
│   ├── 10.RenderToTexture/     Render targets
│   ├── 11.Shadows/             Shadows
│   ├── 12.Network/             Networking
│   └── Demo/                   Larger combined demonstration
├── libs/                     Prebuilt engine / PSP support libraries
├── pspsdk/                   Bundled compatibility header(s)
├── media/                    Art and models
└── docs/index.chm            Offline documentation
```

The source tree includes files named `CTestSceneNode` and `media/test.ms3d`; these names alone do **not** indicate an automated unit-test suite.

## Startup and runtime flow

```text
examples/<example>/main.cpp : engineMain(...)
    ↓ setupPSP() in examples/common.h
createDevice([IEventReceiver*])
    ↓ src/engine.cpp: default creation parameters (480 × 272)
device → video driver + scene manager + GUI environment
    ↓ create/load scene nodes, textures, camera, animators
while (device->run())           [PSP pad polling, timing, event delivery]
    driver->beginScene(...)
    sceneManager->drawAll()      [3D render passes]
    guiEnvironment->drawAll()    [if used]
    driver->endScene()           [present]
```

Key files:
- `examples/01.HelloWorld/main.cpp`: a complete introductory application; creates a device, model, camera and GUI label.
- `src/engine.cpp`: `createDevice` sets 480 × 272, 32-bit, receiver and options before delegating to `createDeviceEx`.
- `src/CengineDeviceStub.cpp`: routes events to the user event receiver, then GUI and scene manager unless consumed.
- `src/CengineDeviceWin32.cpp`: PSP-specific device initialization and controller polling, despite its inherited platform-style class name.
- `src/CSceneManager.cpp`: creates/manages nodes, animators, collision manager and rendering queues.

## Input / controls

1. The PSP pad is polled in `CengineDeviceWin32::run()` using `sceCtrlPeekBufferPositive`; analog sampling is enabled with `sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG)`.
2. Buttons are translated to engine key enumerators defined in `include/Keycodes.h`: `KEY_UP/DOWN/LEFT/RIGHT`, `KEY_TRIANGLE/CIRCLE/CROSS/SQUARE`, `KEY_START/SELECT`, etc. Triggers are mapped to `KEY_LBUTTON` and `KEY_RBUTTON` in the inspected implementation.
3. The device posts `SEvent` key events including `PressedDown` and key-release changes; `IEventReceiver::OnEvent(SEvent)` is the application hook. User handler → GUI → scene manager is the normal dispatch chain, subject to event consumption (`src/CengineDeviceStub.cpp`).
4. The analog stick changes the engine cursor position. With the visible/enabled cursor, CROSS can act as a left mouse click; with cursor hidden the face buttons are delivered as keys. Cursor visibility and enabled state change behavior; do **not** assume every button always reaches user code.
5. There is an on-screen text keyboard mode controlled by START in the device code. The event mapping changes when that mode is active.
6. The FPS camera (`src/CCameraFPSSceneNode.cpp`) registers directional key mappings by default and handles event-driven movement/looking. See `examples/04.Movement/main.cpp` for an event receiver that moves a node **on CIRCLE/SQUARE release**; it also sets the cursor invisible for FPS control.

Game-specific mappings are **not** fixed by the engine: implement your own event receiver, define an action map (move / camera / interact / pause), and decide how to handle cursor vs. gameplay input.

## Rendering

High-level rendering is scene-graph based:
- Obtain `video::IVideoDriver` from the device.
- Create scene nodes through `scene::ISceneManager` (meshes, animated meshes, cameras, lights, skybox, billboards, terrain and custom nodes).
- Assign materials and textures to nodes; texture loading is via the video driver.
- Per frame: `beginScene`, `smgr->drawAll()`, optional `guienv->drawAll()`, `endScene`.
- `CSceneManager::drawAll()` registers nodes and processes passes in order: lights/cameras, skybox, opaque solids (sorted by texture), planar shadows, shadow volumes, cel shading and transparents (sorted by camera distance), then post-render node update.

**PSP-specific backend:** `src/CengineDeviceWin32.cpp::createDriver()` calls `video::createSoftwareDriver2(...)` in the inspected source. Inspect `src/CSoftwareDriver2.cpp` and its lower-level PSP/GL glue before assuming desktop OpenGL is the active PSP renderer. `src/COpenGLDriver.cpp` also exists but its presence alone does not make it the PSP runtime path. Example Makefiles link `-lglut -lGLU -lGL -lpspvfpu` and prebuilt PSP libraries. This is a legacy fixed-function-style rendering architecture, not a modern shader-based rendering pipeline.

Resources in tutorial examples frequently refer to `ms0:/media/...` (PSP Memory Stick path), and the README says to extract `media.zip` to that directory. Missing media will prevent some demos from rendering as intended.

## Collision and physics: exact implementation map

- `include/ISceneCollisionManager.h` and `src/CSceneCollisionManager.cpp`: ray/screen-to-node picking, ray-versus-triangles, nearest collision point, moving ellipsoid vs. triangle world.
- `include/ITriangleSelector.h`, `src/CTriangleSelector.cpp`, `src/COctTreeTriangleSelector.cpp`, `src/CMetaTriangleSelector.cpp`, `src/CTriangleBBSelector.cpp`: ways of presenting scene geometry to collision queries, including octree acceleration and bounding-box selectors.
- `include/ISceneNodeAnimatorCollisionResponse.h`, `src/CSceneNodeAnimatorCollisionResponse.cpp`: applies collision response, world gravity and sliding to a single scene node as part of animation.
- `src/CSceneManager.cpp::createCollisionResponseAnimator()`: user-facing constructor for the collision animator.
- `examples/07.Collision/main.cpp`: main working reference. Loads a Quake 3 BSP, adds an octree scene node, builds an octree triangle selector and attaches collision response to the FPS camera. Example parameters: ellipsoid radius `(30,50,30)`, gravity `(0,-3,0)`, translation `(0,50,0)`. Also demonstrates ray picking and node picking.

**Capabilities observed:** collision queries, simple gravity response, sliding against level geometry, FPS-style world navigation, triangle/node selection.

**Not demonstrated/verified:** a rigid-body world with dynamic box-versus-box contacts, rotational dynamics, masses/inertia, solver iterations, constraints, friction/restitution materials, or stable stacking. A pile of boxes falling on a floor would require a separate dynamics implementation/integration; the existing collision animator is not proof that it already works.

**Suggested first physics investigation:** trace `CSceneNodeAnimatorCollisionResponse::animateNode` → `CSceneCollisionManager::getCollisionResultPosition` → `collideEllipsoidWithWorld`, and examine transform/unit/time assumptions. For a physics-driven game, evaluate a PSP-feasible rigid-body solver separately before designing gameplay around stacking.

## Tests and examples

**Automated tests:** no dedicated repository-root `tests/` directory or obvious automated C++ test runner was seen in the complete recursive tree at inspection time. There are vendor files such as `src/libpng/pngtest.c` and `src/jpeglib/testimg*`, which are dependency test assets, not LTE engine regression coverage. `CTestSceneNode` is an engine render/test primitive, not a unit-test framework.

**Runnable/manual examples:** twelve numbered tutorials plus `examples/Demo/`. Each numbered tutorial has its own `main.cpp` and Makefile; these are useful **manual smoke tests**, not assertion-based automated tests. Start with:
- `01.HelloWorld`: boot, scene drawing, textures, GUI.
- `04.Movement`: PSP key event delivery, scene-node movement and FPS camera.
- `07.Collision`: collision/gravity and picking.
- `10.RenderToTexture`, `11.Shadows`: advanced rendering.
- `Demo`: combined features.

**Build commands described by `install.txt`:**
```sh
cd src
make
make install

cd ../examples/01.HelloWorld
make
```

Requires a compatible PSP toolchain/PSPSDK, PSPGL and historical dependencies. Example Makefiles build a PSP `EBOOT.PBP` with firmware-related flags. **No compile or emulator execution was performed during this documentation pass.**

**Recommended regression test plan (not yet implemented):** PSP/emulator smoke boot; button down/up event test; analog cursor test; image/mesh loading; opaque/transparent render order; triangle ray intersection; static floor ellipsoid collision; gravity and wall sliding; memory/framerate telemetry. Add deterministic host-side math/collision unit tests if code can be isolated from PSP headers.

## Practical map for future chats

| Question | Start here |
|---|---|
| How to start a game? | `examples/01.HelloWorld/main.cpp`, `examples/common.h`, `src/engine.cpp` |
| Where is the PSP event loop? | `src/CengineDeviceWin32.cpp::run` |
| How to bind buttons? | `include/IEventReceiver.h`, `include/Keycodes.h`, `examples/04.Movement/main.cpp` |
| How does FPS camera move? | `src/CCameraFPSSceneNode.cpp` |
| How are objects drawn? | `src/CSceneManager.cpp::drawAll`, `include/IVideoDriver.h` |
| Which renderer does PSP initialize? | `src/CengineDeviceWin32.cpp::createDriver`, `src/CSoftwareDriver2.cpp` |
| How does collision work? | `examples/07.Collision/main.cpp`, `src/CSceneCollisionManager.cpp` |
| Where is gravity applied? | `src/CSceneNodeAnimatorCollisionResponse.cpp` |
| Is block rigid-body simulation present? | Not verified; no integrated rigid-body world located in inspected paths |
| How to build? | `install.txt`, `src/Makefile`, example Makefiles |

## Constraints, uncertainties and next steps

- This documentation describes **LTE Game Engine**, not a confirmed bespoke game prototype. Do not infer a block-spawning game from `examples/07.Collision`.
- Repository tree was read completely; representative examples and core implementations were inspected, **not** every vendor source file or every rendering function.
- No automated test execution, physical PSP run or PSP emulator run has occurred; engine behavior described from sources must be checked at runtime.
- Next useful step: create a standalone `examples/13.PhysicsSandbox` with a floor and a few visual cubes, then separately decide whether the design needs true rigid-body stacking. Establish input, camera, rendering and performance before choosing a dynamics library.
- For the forthcoming game design document, specify genre, moment-to-moment loop, player actions, target PSP framerate, physics fidelity, camera and level structure first; map each feature to supported engine functionality vs. required new development.
