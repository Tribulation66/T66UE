# MOTION_RIG — T66 Master Rigging & Animation Foundation

Status: ACTIVE side lane (overnight build 2026-06-09). This document is the
authority for the MotionRig lane. It will eventually replace the deprecated
rigging/animation lanes (PhysicsFirst Stage 2/3, AnimatedToonStyle bridges,
`UT66HeroPhysicsComponent` full-limp path) once the user approves migration.
Until then, the live game path is untouched; MotionRig exists only behind
`t66.MotionRig.TestRoom` in the Test Room.

## 1. Core model (why this looks right when the old lanes didn't)

The old lanes played animation clips kinematically and switched to full limp
physics on impact. Two realities, visible seams, rigid joints.

MotionRig has ONE reality: the body is always physically simulated.

- **The bean**: the pawn's root is a simulated physics body (vertical capsule).
  ALL movement is forces/impulses on the bean: locomotion = ground force,
  jump = vertical impulse, dive = forward+up impulse with pitch-over, staying
  upright = an angular spring toward vertical (deliberately tunable to be
  imperfect — overshoot is charm). Friction/restitution on the bean's physics
  material are the slipperiness/bounciness knobs.
- **The skeleton**: the visible skeletal mesh simulates every body, attached to
  the bean by a pelvis constraint. PhysicsControl motors (parent-space joint
  drives + a few world-space assists) continuously pull the simulated bodies
  toward the pose the hidden animation evaluation outputs (UpdateTargetCaches →
  UpdateControls each tick). Animation clips are pose targets, never the final
  motion.
- **States are motor profiles, not modes**: Idle/Walk/Jump/Dive/Knockdown/GetUp
  differ ONLY in which clip feeds targets + per-set motor gains. Knockdown =
  gains near zero (body goes naturally limp); recovery = gains ramp back while
  targeting the get-up pose. There is no kinematic↔simulated switch anywhere,
  so there is no seam to look bad.

## 2. Runtime architecture (Source/T66/Gameplay/MotionRig/)

| File | Role |
|---|---|
| `T66MotionRigPawn.h/.cpp` | APawn: bean root (capsule, simulated), spring-arm camera, skeletal mesh (simulated, motor-driven), input receiver implementation |
| `T66MotionRigMovement.h/.cpp` | Force-based locomotion: ground drive, upright angular spring, jump/dive impulses, air control, ground sensing (sphere trace) |
| `T66MotionRigMotorSystem.h/.cpp` | PhysicsControl wrapper: creates controls/body modifiers from the rig spec, owns named motor profiles, applies per-set gains, feeds clip pose targets |
| `T66MotionRigStateMachine.h/.cpp` | State enum + transitions (Idle/Walk/Jump/Dive/Knockdown/GetUp), clip selection, impact reaction entry/exit rules |
| `T66MotionRigTypes.h` | Profiles/config structs (bean tuning, motor gains per set, state data) — all EditAnywhere + CVar-mirrored for live tuning |
| `T66MotionRigScenario.h/.cpp` | Deterministic test driver: scripted input sequences (walk circle, jump x3, dive, standard impact), telemetry logger (CSV per frame: bean velocity, pelvis transform, per-foot velocity, body pitch), exec-command entry points |
| `T66MotionRigInputReceiver.h` | UInterface the player controller bridge calls (SetMoveAxes, JumpPressed/Released, DivePressed) |

**Multiplayer seam (build-for, don't build-yet):** the bean is the only
gameplay-authoritative object — its transform/velocity/state enum is what would
replicate in the 4-player expansion. The skeleton+motors are strictly cosmetic,
always local, never replicated. All input enters through one interface
(`IT66MotionRigInputReceiver`), so a replicated input path can replace the
local one without touching the motion code.

**External hooks (the ONLY edits outside the lane):**
1. `T66.uproject` — PhysicsControl plugin enabled (done).
2. `T66.Build.cs` — `PhysicsControl` module dependency.
3. `AT66GameMode_TestRoom::GetDefaultPawnClassForController_Implementation`
   override — returns the MotionRig pawn when `t66.MotionRig.TestRoom` != 0 and
   the selected hero is Hero_1 (any skin). Default 1 (test room only).
4. `T66PlayerController` movement handlers — if the pawn implements
   `IT66MotionRigInputReceiver`, forward move axes/jump/dive (Leap key) and
   return early. No change for normal hero pawns.

## 3. Rig spec (Content/Characters/MotionRig/Hero_1/ + Blender pipeline)

Master spec for ALL future MotionRig characters (enemies, bosses):

- **Bones (17)**: `pelvis` (root body), `spine_01`, `spine_02`, `head`,
  `clavicle_l/r` (weight-only, no physics body), `upperarm_l/r`,
  `lowerarm_l/r`, `hand_l/r`, `thigh_l/r`, `calf_l/r`, `foot_l/r`.
  No fingers, no twist bones, no IK bones. Chunky readability over detail.
- **Skinning**: up to 4 influences/vertex, smooth falloff across joints
  (the old lane's 1-influence hard skinning is banned — it reads as rigid
  plastic; soft blends read as rubber).
- **Proportions/axes**: 180 cm normalized height, Blender front = -Y, exported
  FBX lands UE forward = +X, scale 1.0, root at origin between feet.
- **Physics bodies (15)**: capsules per bone except clavicles; mass distribution
  pelvis-heavy (pelvis+spine ≈ 45%, head ≈ 8%, each arm ≈ 6%, each leg ≈ 17.5%
  of total ~70 kg). Generated programmatically at import (no hand-tuned asset).
- **Clips are pose targets** (30 fps, root motionless — the bean owns
  translation): `Idle` (60f loop, weight-shift sway), `Walk` (30f loop, one
  full cycle, big arm swing), `Jump` (34f: crouch-launch-tuck-reach),
  `Dive` (42f: arms-forward superman hold), `GetUp_Front`/`GetUp_Back` (50f).
  Cadence contract: Walk clip authored at 1.75 steps/sec; runtime scales play
  rate with bean ground speed so feet match ground velocity.

Pipeline (Scripts/MotionRig/): `blender.exe --background --python
BuildMotionRig.py` consumes the raw GLB → emits skeletal FBX + clip FBXs +
QA JSON + proof renders; `ImportMotionRig.py` (UE python commandlet) imports
into `Content/Characters/MotionRig/Hero_1/`, builds the PhysicsAsset
programmatically, and saves an import report. Both deterministic, re-runnable.

## 4. Feel rubric (the measurable definition of "Fall Guys feel")

Telemetry is engine ground truth logged by `T66MotionRigScenario` at 60 Hz.
Targets marked (p) are provisional until user taste pass.

| # | Axis | Metric | Target |
|---|---|---|---|
| 1 | Cadence coupling | foot-bone world speed during stance / bean ground speed | < 15% slide (p) |
| 2 | Acceleration lean | bean pitch into acceleration | 8–18° at full drive (p) |
| 3 | Wobble character | arm/head oscillation after perturbation: frequency + overshoot count | 1.5–3 Hz, 1–2 overshoots, settle < 1.2 s (p) |
| 4 | Jump shape | time-to-apex / landing absorb dip / re-extend time | ≈0.45 s / pelvis dips 12–20 cm / < 0.4 s (p) |
| 5 | Dive signature | launch angle / pose hold error vs clip / slide distance / time-to-feet | ≈35° / motors visibly win in air / 1–2 m / < 1.6 s (p) |
| 6 | Knockdown | limpness onset / tumble angular-velocity decay / settle / get-up duration | < 0.1 s / monotonic, no jitter / < 2 s / ≈1.5 s (p) |
| 7 | Upright spring | recovery from standard push | 1–2 overshoots, no oscillation tail (p) |
| 8 | Responsiveness | input edge → bean force application → visible velocity change | force same tick; velocity response < 100 ms (p) |

Anti-jank invariants (hard fails regardless of taste): no high-frequency
(>10 Hz) bone jitter, no interpenetration with floor at rest, no NaN/explosion
(bodies > 10 m from bean), no visible teleport/seam at any state transition.

Evidence format per iteration: contact sheet PNG grids (fixed cameras: front,
side, 3/4; full speed + 0.25x slow-mo) + `metrics.csv` + pass/fail table vs
this rubric. Comparison constants derive from technique-level study of public
Fall Guys material (timings/proportions only — no assets are copied).

## 5. Tuning knobs (all live, no rebuild)

- Bean: drive force, max ground speed, friction, restitution (bounciness),
  upright spring strength/damping, jump impulse, dive impulse/pitch.
- Motors per set (legs/spine/arms/head, per state): linear+angular strength,
  damping ratio, max force.
- All mirrored as `t66.MotionRig.*` CVars; scenario re-runs read live values.

## 6. Out of scope (this lane, for now)

Replication code, combat/damage integration, non-Hero-1 characters, migration
of the live game path, removal of old lanes. Each is a separate user decision
after the Test Room proof.
