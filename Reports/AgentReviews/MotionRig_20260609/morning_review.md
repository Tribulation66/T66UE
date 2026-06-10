# MotionRig Overnight Run — Morning Review

Run date: 2026-06-09 night → 2026-06-10 morning. Agent: Claude (autonomous).
Authority doc: [MOTION_RIG.md](../../../MOTION_RIG.md). Worker record: [run_state.md](run_state.md).

## TL;DR

(filled at end of run)

## How to try it yourself

1. Stage/launch the standalone build (or `UnrealEditor.exe T66.uproject /Game/Maps/GameplayLevel -game`).
2. Pick **Hero 1 (Chad)** and enter the **Test Room**.
3. You spawn as the MotionRig physics pawn: WASD walk, Space jump, Leap key = dive,
   wipeout arm / scenario impacts = knockdown + recovery.
4. Escape hatch: `t66.MotionRig.TestRoom 0` restores the regular hero pawn (console or cmdline).

Deterministic review captures (each writes video + contact sheets + telemetry):

```powershell
pwsh Scripts/MotionRig/CaptureMotionRig.ps1 -Scenario walkcircle -Camera side -Label review_walk
pwsh Scripts/MotionRig/CaptureMotionRig.ps1 -Scenario dive -Camera threequarter -Label review_dive
pwsh Scripts/MotionRig/CaptureMotionRig.ps1 -Scenario impact -Camera side -Label review_impact -SloMo 0.25
python Scripts/MotionRig/AnalyzeTelemetry.py Reports/AgentReviews/MotionRig_20260609/captures/review_walk/telemetry.csv
```

## What was built

- **Runtime** (`Source/T66/Gameplay/MotionRig/`): simulated bean pawn (forces/
  impulses for walk/jump/dive, springy upright + yaw), PhysicsControl motor
  system (always-simulated skeleton chasing clip pose targets; states are
  motor-gain profiles — no kinematic/simulated switches anywhere), knockdown →
  settle → get-up flow, deterministic scenario driver + 60Hz telemetry logger.
- **Master rig spec + pipeline** (`Scripts/MotionRig/BuildMotionRig.py`):
  18-bone fresh rig on the raw Hero 1 GLB, distance-based smooth skinning
  (max 4 influences), six procedural pose-target clips
  (Idle/Walk/Jump/Dive/GetUp_Front/GetUp_Back), QA JSON + proof renders.
- **Physics asset commandlet** (`-run=T66MotionRigPhysicsAsset`): deterministic
  capsule bodies (18) + constraints (17), FBX root body culled.
- **Capture + rubric harness**: `CaptureMotionRig.ps1` (scenario → frames →
  MP4 + contact sheets + telemetry) and `AnalyzeTelemetry.py` (8-axis feel
  rubric + anti-jank invariants).
- **Hooks (only edits outside the lane)**: test-room pawn override
  (`t66.MotionRig.TestRoom`, default 1, Hero_1 only), controller input bridge
  (interface-based), `PhysicsControl` plugin + module dependency.

## Evidence

(links filled as iterations land — see `captures/`)

| Iteration | What changed | Result |
|---|---|---|
| walkcircle_v1 | first end-to-end capture | pipeline PASS; character imported at 1/100 scale (Blender m → UE cm), bean frozen by extreme constraint mass ratio |
| walkcircle_v2 | x100 scale baked in Blender export, PA regenerated | (pending) |

## Rubric status

(final `AnalyzeTelemetry` outputs per scenario — filled at end of run)

## Known issues / punch list

(filled at end of run)

## Multiplayer seam (for the 4-player expansion)

The bean is the only gameplay-authoritative object — replicate its transform,
velocity, and state enum and you are done; the skeleton + motors are always
local cosmetics. All input enters through `IT66MotionRigInputReceiver`, so a
replicated input path slots in without touching motion code.
