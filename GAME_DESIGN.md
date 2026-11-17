# Tower Physics — Game Design Document

**Status:** Draft v0.2, pending gameplay testing  
**Date:** 2026-11-17  
**Target:** Sony PSP, LTE Game Engine 2.2, C++  
**Companion:** [ENGINE_ARCHITECTURE.md](ENGINE_ARCHITECTURE.md)

## 1. Pitch

An original, compact arcade tower-building game inspired by the *mechanical premise* of Tower Bloxx / City Bloxx. Drop cuboid blocks from a swinging rope, align them with a tower that rocks in the wind/under impacts, and build as high as possible without collapsing. Do not copy the reference game's branding, art, sounds, levels or source.

**Core pillars:** readable timing; momentum-preserving release; visible balance; escalating height and tension; fast retries.

## 2. Confirmed direction

- 3D-rendered six-faced cuboids; physics restricted to a vertical **X–Y plane** with rotation only around the perpendicular Z axis (**2.5D**).
- The active top **three tower levels** use full dynamic planar rigid-body physics: gravity, oriented-box contacts, impulses, angular inertia, friction, restitution and sleeping.
- Lower levels become a **kinematic animated foundation** shaped by a height-dependent sway spline. They are **not** individually simulated rigid bodies once frozen. This is a deliberate stylized/hybrid simulation, **not** full-body physics throughout the tower.
- A new block swings from a rope/pivot before release. Release keeps tangential velocity.
- **Target 60 FPS rendering and 60 Hz fixed-step physics**, on actual PSP hardware subject to measurement; do not promise locked 60 without benchmarking.
- One suspended/spawning block at a time.

## 3. Core loop

1. Spawn a new cuboid on a pendulum suspended from a pivot above the current tower top.
2. Swing at a predictable cadence while the tower sways.
3. Player presses Cross to release the block. Tangential momentum carries into free fall.
4. Dynamic rectangle/box contact solver handles collisions with the top blocks and with the animated foundation.
5. Accumulate score when a block lands plausibly and survives the settling window.
6. When the tower grows, advance the foundation boundary: freeze the oldest of the dynamic top blocks into a kinematic tower level; keep at most three previous tower levels dynamic. A currently falling block may temporarily be a fourth active physics body.
7. Animate the lower tower's slow bend/rock visually, advance the camera, repeat.
8. Failure if an active block falls off or the tower crosses an agreed collapse threshold; show results and allow rapid restart.

**Design note:** Promotion/freeze must occur only when a candidate block is reasonably settled, not during collision or while tipping. Never simply freeze a violently moving block to rescue an unstable stack.

## 4. Simulation architecture — hybrid physics and sway

```text
GameState / Spawn / Scoring / Camera
       │
       ├── PendulumController (suspended; kinematic before drop)
       │           └── release → dynamic body + inherited velocity
       │
       ├── PhysicsWorld2D (fixed 1/60 s step)
       │     ├── falling/released block
       │     ├── upper 3 dynamic tower levels
       │     └── contacts with moving kinematic foundation
       │
       ├── FoundationSway (low-frequency spline / analytical curve)
       │     ├── frozen lower-level visual transforms
       │     └── moving top contact/support collider(s)
       │
       └── RenderBridge (2D positions/angles → 3D scene-node transforms)
```

### 4.1 Coordinates

Each active rectangular rigid body stores `position (x,y)`, `angle theta`, `velocity (vx,vy)`, `angularVelocity omega`, width, height, mass/inverse mass, inertia/inverse inertia, material and sleep state.

Render it as an LTE scene node with `(x,y,0)` position and `(0,0,theta)` rotation, with a fixed visual depth. If using a perspective camera, keep camera depth and clipping consistent. Orthographic or weak-perspective side view is preferred for readable landings.

For uniform rectangles, planar inertia `I = m(w²+h²)/12`.

### 4.2 Box contacts

- Broadphase: AABB overlap, initially O(n²) is adequate for ~3 active bodies plus one falling block and small set of kinematic supports; revisit based on profiling.
- Narrowphase: 2D oriented rectangle SAT with 1–2 contact points.
- Sequential-impulse normal/friction contact solver with positional stabilization, warm starting if stacking jitter requires it.
- Static platform at the bottom; kinematic foundation top has a surface contact collider.
- Friction allows rocking and sliding; restitution should be low.
- Sleeping permitted for quiet dynamic bodies, but an impact must wake them.
- Dynamics remain genuine **for the top section** even if the base is visually stylized.

### 4.3 Pendulum

MVP: the hanging block is a kinematically controlled pendulum rather than a fully simulated rope. For rope length `L`, pivot `(px,py)`, angle `a` and angular speed `da/dt` in Y-up:
- center `x = px + L sin(a)`, `y = py - L cos(a)`
- release `vx = L cos(a) da/dt`, `vy = L sin(a) da/dt`

At release, detach the visual rope and activate a dynamic body with exactly the calculated position and velocity. The initial swing can be deterministic and periodically animated; refine to a constrained dynamic pendulum only if playtesting requires it.

### 4.4 Swaying frozen foundation

**Goal:** depict a tower increasingly swaying with height without solving contacts for every historical block.

- Keep a persistent **rest pose** for each frozen block: its level index, relative tower placement, dimensions, and local rotation/offset. Freeze is a physics state transition, not a change to visually identical cubes.
- Define a smooth height-dependent lateral displacement function `s(y,t)`, via a cubic spline through a few control points or a smooth analytic function.
- Lower levels move less; higher levels move more. Suggested prototype shape: `s(y,t) = A * (y/H)^p * sin(2π f t + phase)` for `p > 1`, with optional short-lived impulse-driven component from top impacts. These are tunable values, **not confirmed balancing settings**.
- Apply each frozen block's transform from **rest pose + current spline displacement** every frame; never accumulate delta transforms frame over frame.
- For believable rigid blocks, each frozen cuboid remains a rigid visual object: position it at the curve at its center and give it a small orientation matching the curve's slope. Do not shear/stretch individual cuboid meshes.
- Avoid visible gaps/overlaps from large inter-level angle changes: cap amplitude/slope, or choose a **single rigid kinematic foundation** and make the spline mostly visual. The prototype should test these trade-offs.
- The spline is a **stylized animation**, not physically derived bending of concrete cubes. In 2.5D toppling is still real within the active region.

### 4.5 Collision coupling at dynamic/kinematic boundary

This is the most important technical risk.

- Frozen lower levels are **kinematic**: effectively infinite mass and prescribed motion. They must present a collision surface to the active physics solver.
- The uppermost frozen cuboid or a small set of frozen top colliders provides support for dynamic bodies. Compute its transform from the same sway model **before each physics substep**.
- Calculate its translational and angular velocity from the time derivative of the sway curve (or finite differences at the fixed time step). The contact solver uses the **relative** contact-point velocity. **Do not teleport the collider without velocity** or contacts will jitter, gain energy, or tunnel.
- If the top support is slightly tilted, collide with its actual oriented rectangle, not an axis-aligned floor.
- The lower tower's visual sway cannot be allowed to move the top physics blocks by parent-node transforms as well as via collisions: physics transforms must be world-space authoritative. **Never double-apply the spline**.
- For stronger impacts, inject a bounded, decaying sway response into the base oscillator based on net impulse. This is an artistic coupling, not physically exact momentum conservation. Cap amplification to avoid runaway energy.
- To reduce numerical instability, cap sway amplitude, acceleration and support angular velocity and use conservative impact response at the boundary.

### 4.6 Freezing policy (critical to game feel)

When the newly placed block is accepted, target the **top three settled tower blocks** as dynamic. If the fourth-from-top block is stable:
1. Record its current physics pose as a permanent **frozen rest pose** relative to the foundation's current sway reference.
2. Replace its dynamic body with a kinematic counterpart, with continuous position/orientation/velocity. Recompute its world transform from the rest pose and the foundation's future spline.
3. Preserve its contact-support role for blocks above.
4. Drop it from active dynamic solver bookkeeping **only after** all contact identities/manifolds have been rebuilt safely.
5. If the candidate is not settled or is slipping/falling, **defer freezing** and temporarily allow >3 dynamic levels. Better a bounded temporary cost than snapping/cheating physics.

**Avoid a hard visible seam:** blend sway/rotation at the transition. At the precise freezing frame, the new kinematic pose and velocity must match the dynamic pose, then gradually ease to the scripted sway. If seamless matching proves too complicated, freeze only during calm periods.

### 4.7 Collapse / failure policy

For the initial prototype:
- Missed block: if dropped body falls below an agreed height/ground area, it counts as a miss.
- Structural collapse: if active tower top loses contact with support or some blocks fall far below expected stack height, trigger game over after a short grace period.
- Large lower-foundation motion is not itself game over; a visibly leaning tower may recover.
- Because frozen levels are kinematic, the lower tower **cannot actually disintegrate** without a special mode. Either:
  - **Recommended MVP:** keep foundation intact, show active top collapsing and declare game over; or
  - **Future polish:** on failure switch a limited set of nearby frozen blocks back to dynamic or play a scripted/faux collapse.
- Document this limitation in gameplay design; do not promise fully physical collapse of the entire tower.

## 5. Performance targets and instrumentation

**Targets, not guarantees:**
- Render cadence: **60 FPS** (16.67 ms per displayed frame).
- Physics update: **60 fixed updates per second**, `dt = 1/60 s`.
- Render once per update during steady state if timing permits. Accumulator can handle jitter, but cap catch-up substeps to prevent spiral-of-death stalls; never silently increase the physics timestep.
- Profile on real PSP (and PSP emulator as a secondary development aid).
- Count collision candidate pairs, solver contacts/iterations, active and sleeping bodies, kinematic-support updates, rendering draw calls, total frame time and peak heap usage.
- Start with **three top dynamic tower levels plus one falling block**, not an arbitrary unlimited rigid-body budget.
- Simplify materials, shadow passes, draw calls and texture sizes before sacrificing physics consistency.

**Measurement gates:**
1. Empty game scene maintains near 60.
2. 3 dynamic resting blocks + kinematic support: stable without jitter.
3. 1 falling block with off-center impact, dynamic tower of 3 blocks, scripted foundation sway: verify 60/60 and plausible transfer of momentum.
4. Tall tower of many *rendered* frozen blocks: inspect draw-call cost and memory.
5. Stress: worst-case contact/impact and camera motion; log dropped physics steps, glitches, penetration, and FPS.

If profiling cannot sustain 60/60, first optimize contacts/rendering and visual effects; any lower-rate fallback requires explicit future design approval.

## 6. PSP controls (proposed, revise after prototype)

| Input | In game | Notes |
| --- | --- | --- |
| Cross | Release current block | On fresh key press, once |
| D-pad left/right | Optional horizontal pivot adjustment | Disable in first one-button test |
| Start | Pause / resume | Freeze gameplay simulation timer |
| Circle | Retry on game over / back in menu | Contextual |
| Select | Show debug overlay | Development builds only |

The engine polls PSP controls in `src/CengineDeviceWin32.cpp` and dispatches `SEvent` through `IEventReceiver`. Cursor visibility changes face-button behavior, so disable/hide cursor for active gameplay and test button edges carefully.

## 7. Scoring / difficulty (starting proposals)

- +100 points for each landed, surviving block.
- Placement bonus up to +50 based on horizontal alignment with supporting block(s).
- Consecutive accurate drops increase streak multiplier (e.g., up to ×2).
- Show tower height and maximum successfully placed blocks separately from score.
- Early progression: slow, predictable pendulum and forgiving friction; later: faster swings, subtle variation in block width or mild sway intensity.
- Never increase sway randomly enough to erase player control. Difficulty should arise primarily from prior placement and tower geometry.

All scoring/difficulty numbers are placeholders for tuning.

## 8. Visual/audio direction

- Clean, colorful low-poly buildings/cuboids; simple face colors or small texture atlas.
- Fixed side view with slight visible top/side faces for 3D richness.
- Clear moving contact area and rope, subtle shadow or grounding cues.
- Camera moves upward smoothly while preserving top-of-tower context.
- Short impact sounds by impulse strength, positive confirmation for a good landing, distinct wobble warning and collapse sound.
- Make HUD legible at **480 × 272**. Avoid too much on-screen text.

## 9. States

`MENU → SWINGING → DROPPED → IMPACT/SETTLING → SUCCESS/SPAWN → SWINGING`; `GAME_OVER` on miss/collapse; `PAUSED` overlays gameplay states.

Define transitions with explicit conditions so that a bouncing block cannot score twice or spawn duplicates.

## 10. Tests and acceptance criteria (to implement)

**Deterministic host-side physics unit tests**, ideally independent of PSP SDK:
- Box falls onto static floor and comes to rest with bounded penetration.
- Two boxes stack stably without accumulating energy.
- Off-center hit produces angular velocity and plausible tipping.
- Friction threshold distinguishes resting vs. sliding.
- Kinematic support with prescribed lateral velocity correctly imparts relative-motion impulses.
- Kinematic platform on a small tilt supports or releases the upper box appropriately.
- Promotion to frozen foundation produces no discontinuity in position/velocity.
- Same input sequence and initial seed produce sufficiently reproducible results.

**PSP/emulator smoke tests:**
- Swinging block visibly inherits horizontal velocity upon Cross release.
- Top three levels remain dynamic, lower levels sway without visual seams.
- Repeated button press and pause do not double-release/spawn.
- Camera preserves visibility of tower top and falling block.
- 60 FPS / 60 Hz tracked; record real hardware results.

There is no verified existing automated LTE engine test suite; see [ENGINE_ARCHITECTURE.md](ENGINE_ARCHITECTURE.md).

## 11. Implementation milestones

1. **Vertical slice A:** clean PSP application with 60/60 timing instrumentation, one static floor, one falling physics cuboid, working controls.
2. **Vertical slice B:** oriented rectangle contact solver, friction, angular dynamics, three real stacked bodies, repeatable host-side physics tests.
3. **Vertical slice C:** pendulum controller, momentum-conserving release, scoring, one-button gameplay loop.
4. **Vertical slice D:** kinematic frozen foundation spline, correct contact velocities, seamless top-three promotion/freeze.
5. **Vertical slice E:** tall-tower camera/HUD, difficulty balancing, failure/retry, PSP optimization.
6. **Polish:** art, audio, menu, high-score persistence, expressive swaying and possible expanded collapse animation.

**Engine implementation dependency:** LTE has ellipsoid-against-world collision and picking, but this design requires a separate true **planar box rigid-body solver** (custom or compatible library). Do not confuse `CSceneNodeAnimatorCollisionResponse` with a working box-stack solver.

## 12. Open design questions

- Tower block proportions: identical cubes vs. varying cuboid widths/heights?
- Is the anchor fixed over the global center, tower center of mass, or latest block?
- Should sway respond primarily to impacts, height, or a predictable breeze?
- Score until first dropped block / dislodged block, or allow recovery after a miss?
- Once game over occurs, should some frozen blocks be reactivated for a dramatic final collapse?
- Final title, visual theme and whether the first release ships with one mode or multiple.

Decisions above tagged **proposed** remain subject to gameplay tuning. The **60/60 target**, **2.5D simulation**, and **top-three dynamic + spline-driven lower foundation** are the current agreed direction.

## 13. Implemented prototype failure plane (2026-11-17)

In `examples/13.PhysicsSandbox`, the first loss rule uses a **fixed Y plane at the top surface of the lowest seeded tower block** (the current prototype's foundation/spine). Any *upper* dynamic block whose **center** falls below this plane triggers `GAME_OVER`. The lowest foundation block is excluded because it begins below its own top surface. The simulation stops on the failing physics step; Cross or Circle restarts the sandbox and Triangle returns to Play/Exit menu. A red debug line marks the plane. This catches a missed block even if it lands beside the tower on the static ground, where testing for `Y < 0` would fail.

When spline-frozen lower levels are implemented later, this rule can be updated to derive the reference plane from the moving kinematic foundation. The threshold is intentionally fixed for this vertical slice, and game-over conditions beyond this loss rule remain future design work.

## 14. Developer live tuning (2026-11-17)

The PSP vertical slice now reads an editable `tuning.cfg` at startup. During gameplay or Game Over, **hold L**, use D-pad up/down to select a value and left/right to adjust it; release L to save back to **the same file path**. Gravity, friction, restitution, linear/angular damping, swinging pendulum amplitude/speed, required settling seconds, and points awarded for a successfully settled block are supported. The game retains a modular HUD `SCORE` tile alongside `BOXES STACKED`. New successful placements award the current `score_per_block`, while the three seed blocks do not award points. Live tuning does not stop an unpaused simulation, and a game-over screen remains static. Settings persist across retries, and when saved, across reboots.

The editable config sits beside EBOOT in the homebrew working directory, or may be adapted to an explicit Memory Stick path. For shipping, distribute the tuned config with the game or compile defaults generated from it. See `examples/13.PhysicsSandbox/README.md` for placement, controls and failure reporting. PSP read/write permissions and EBOOT rendering still require on-device verification.

## 15. Prototype sound feedback (2026-11-17)

The PSP vertical slice provides ten distinct short, **procedurally synthesized** effects: D-pad menu navigation, menu confirmation, back, block release, first falling-block contact, successful stacking, game-over, pause/resume, tuning adjustment and tuning saved. A dedicated `GameSounds` module uses one free channel in LTE's existing PSP callback audio mixer; it stores generated PCM once and replays it without disk assets or synchronous playback on the 60 Hz game thread. A `SoundCues` latch prevents repeated impact sounds from persistent contacts. It remains a prototype sound palette rather than final art; real-device latency, sound quality and multi-voice mixing are pending QA.

## 16. Wind weather system (2026-11-17)

Wind is an active gameplay mechanic with a **deterministic cyclic 2.5D horizontal force** and smaller faster gusts. It uses the same 60 Hz fixed timestep as rigid-body physics. Exposed vertical cuboid faces receive force and a small torque at an elevated point, which affects stacking and balance. The swinging rope block leans into wind, including tangential velocity on release. Static ground receives no wind force. Menu, paused and Game Over states do not advance the weather clock; a new game/retry resets it.

A **bottom-left** wind icon with animated-state direction (`<<` / `>>`), severity and strength percentage tells the player what the tower is experiencing. A 3D arrow to the left of the tower is assembled from three 3D box-shaped arrow pieces, pointing along the force direction. Its colour and length indicate relative strength; calm hides the arrow. Weather severity is based on absolute force, independently of the configured cap. The panel uses colours from blue to red for progressively stronger winds.

Weather settings are added to the existing live-tuning `tuning.cfg`: `wind_strength` (arcade horizontal force, default 1.3), `wind_gustiness` (default 0.4), `wind_cycle` (seconds, default 14). The hold-L tuning UI now paginates its 12 settings. This phase adds **wind only**; rain/clouds and physical weather systems may be future enhancements. The real PSP rendering and physics performance must be checked separately from native tests.
