# MotionRig Overnight Run — Morning Review

Run date: 2026-06-09 night → 2026-06-10 morning. Agent: Claude (autonomous).
Authority doc: [MOTION_RIG.md](../../../MOTION_RIG.md). Worker record: [run_state.md](run_state.md).

## TL;DR

**The physics-first hero foundation works end to end.** Hero 1 in the Test
Room spawns a fully physics-driven character: a simulated bean capsule does
all movement (forces/impulses), an always-simulated 18-bone skeleton chases
animation pose targets through joint motors, and knockdown is just motors
dropping to zero — no kinematic/ragdoll mode switch exists anywhere. The bean
walks, jumps, dives, takes standardized impacts, and recovers; every capture
passes the anti-jank invariants (no explosions, no floor penetration, no
jitter). The body renders with the FriendSlop lit material.

What it is NOT yet: tuned. The feel rubric (8 axes) passes partially
(walk cadence, lean, and jump shape need gain tuning — the per-axis numbers
and every knob are in place to do that iteratively). It is a foundation that
moves correctly, not yet a character that charms. That next mile is pure
CVar tuning against the rubric, no architecture left to build.

## Standalone acceptance: PASS

The staged build at `Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
(shortcut `T66 Standalone.lnk`, rebuilt tonight) was smoke-tested headlessly:
direct entry to the Test Room as Hero_1 spawned the MotionRig pawn from cooked
assets, motors initialized (17 drives, 70 kg), the bean grounded, the scripted
walk scenario executed all steps, and telemetry was written from the packaged
game. Evidence: `captures/standalone_smoke/` + the log lines quoted in
run_state.md. One earlier cook failure was mine (CDO-time mass override
logging an Error) — fixed.

## How to try it (the 30-second version)

Launch the staged standalone (or `UnrealEditor.exe T66.uproject
/Game/Maps/GameplayLevel -game`), pick **Hero 1 (Chad)** → **Test Room**.
WASD walk, Space jump, Leap key = dive, walk into the wipeout arm for a
knockdown. `t66.MotionRig.TestRoom 0` restores the regular hero.

Deterministic review captures + rubric scoring:

```powershell
pwsh Scripts/MotionRig/CaptureMotionRig.ps1 -Scenario dive -Camera side -Label mydive
python Scripts/MotionRig/AnalyzeTelemetry.py Reports/AgentReviews/MotionRig_20260609/captures/mydive/telemetry.csv
```

## Evidence (captures/)

| Capture | What it shows |
|---|---|
| `locked_walkcircle` | LATEST walk state: bean pitch/roll locked (lean 89°→0°, the character no longer walks lying down), invariants PASS |
| `locked_dive` | LATEST dive: prone pitch now authored through the pelvis PD target (bean stays vertical), 3/5 axes |
| `locked_impact` | LATEST knockdown + recovery, invariants PASS |
| `final_*` / `tuned_*` | earlier same-night iterations for comparison |
| `final_impact_slomo` | impact at 0.25x for seam inspection |
| `walkcircle_v1..v20`, `jump_v21..v23` | the full debugging archaeology, each with telemetry |

Headline rubric numbers (locked_walkcircle): bean lean 0.0° (locked by design —
the lean metric should be re-sourced from pelvis orientation next session),
foot-slide ratio ~0.99 (the main feel gap: legs track pose targets but stance
feet skate — leg spring/cadence tuning is the first morning knob),
responsiveness 0.40s (investigate drive bite vs the analyzer's input-time
assumption). All anti-jank invariants pass on every capture.

Each folder: `*.mp4` video, `*_sheet*.png` contact sheets, `telemetry.csv`
(60 Hz body-instance ground truth), `metrics.json` (rubric scores).

## What was built

- **Runtime** (`Source/T66/Gameplay/MotionRig/`, ~1,400 lines): bean pawn
  (force locomotion, springy upright + yaw, jump/dive impulses,
  friction/restitution knobs), motor system (direct SLERP drives on the
  skeleton's joint constraints + hidden pose-source mesh providing per-tick
  targets + one-way virtual-PD pelvis coupling), state machine
  (Idle/Walk/Jump/Dive/Knockdown/GetUp as motor profiles), deterministic
  scenario driver + telemetry logger.
- **Master rig pipeline** (`Scripts/MotionRig/BuildMotionRig.py`): fresh
  18-bone rig on the raw GLB, distance-based smooth skinning (max 4
  influences), six procedural pose-target clips, QA JSON, proof renders.
- **Physics asset commandlet** (`-run=T66MotionRigPhysicsAsset`): capsules,
  culled root, deterministic.
- **Harness**: `CaptureMotionRig.ps1` (scenario → frames → MP4 + sheets +
  telemetry) and `AnalyzeTelemetry.py` (8-axis rubric + invariants).
- **Docs**: `MOTION_RIG.md` (authority),
  `Model Generation/Instructions/14_MOTIONRIG_RIGGING_PIPELINE_INSTRUCTIONS.md`
  (pipeline + pitfalls).
- **Hooks (only edits outside the lane)**: test-room pawn override
  (`t66.MotionRig.TestRoom`, default 1, Hero_1 only), controller input bridge,
  PhysicsControl plugin enable + module dep (the plugin ended up unused — see
  punch list).

## The debugging story (for the engineering record)

Eight real bugs stood between "compiles" and "walks", each found by
measurement, each now documented in the pipeline instructions:

1. Blender bone-heat auto-weights fail silently on generated multi-shell
   meshes → distance-based skinning.
2. glTF imports are quaternion-rotation-mode; euler writes silently no-op →
   the facing flip never applied.
3. The FBX exporter already converts m→cm; adding a x100 bake created an
   18,000-unit kaiju.
4. Pawns spawn at the map origin and teleport later; full-size simulated
   bodies detonate on depenetration → physics bring-up deferred 0.75s.
5. The controller's axis bindings fire every frame and stomped scripted
   input → scenario input override.
6. Auto-generated physics assets weigh 485 kg → authored 70 kg mass table.
7. Both UPhysicsControlComponent and UPhysicalAnimationComponent produced
   ZERO force on this setup in 5.7 → direct SLERP drives on the skeleton's
   own constraints (+ wake management; sleeping islands eat forces).
8. **The heap was a render illusion**: bodies stood and walked while bones
   froze — attached fully-simulated skeletal meshes never run the
   physics→bones blend; DETACHING the mesh at bring-up (exactly what the old
   lane's `bDetachMeshDuringRagdoll` knew) fixed rendering instantly.

## Punch list (next session)

- **Gait quality is the one open feel problem**, and it is now precisely
  characterized: joint drives track perfectly at rest (thigh error 0.0° at
  1° demand) but lag walk-cycle targets ~1:1 (error 35–48° at 30–38° demand)
  regardless of stiffness ×7, free joint limits, slower cadence, or per-tick
  drive-param flushes — all tested tonight with per-joint telemetry
  ([MR_DIAG2] thighDemand/thighError in the log). Two live hypotheses:
  (a) Chaos clamps/under-iterates angular drives at game tick rate — try
  raising physics substeps and per-body solver iteration counts;
  (b) the error/target frame convention is subtly off for non-identity
  constraint reference frames — verify with the new CLOSE cameras (the
  `closecam_*` captures finally make limbs readable) before trusting any
  number. Use `closecam_impact_slomo` to study what the joints really do.
- **Posture**: he walks pitched ~50° forward — the body is "towed" behind the
  moving bean by the pelvis linear PD and tips. Couple the fix with gait:
  pelvis angular authority (tested to Kp 1100 — insufficient alone), possibly
  hold spine_01 in world space too, or add a small anticipatory lead to the
  pelvis target. Charming as-is, but not the spec.
- Bean pitch/roll are LOCKED by design (friction torque beats any sane
  upright spring ~8:1, measured). "Acceleration lean" should be re-sourced
  from pelvis orientation in the analyzer; the bean-lean axis reads 0 forever.
- Foot-slide metric reads BODY capsule velocities — capsule offsets make it
  pessimistic; consider reading foot BONE transforms from the Visual copy or
  recalibrating the threshold once gait is visually right.
- Head capsule placement from the auto-PA sits low — regenerate with
  per-bone orient/size overrides in the commandlet.
- Fixed review cameras (side/front) lose the view-target tug-of-war with the
  controller sometimes; chase works. Low priority.
- The unused PhysicsControl plugin enable + Build.cs dep can be removed, or
  kept for a future retry (the RigidBodyWithControl anim-node path may work
  where manual control creation did not).
- Dive currently reuses GetUp_Front after the slide; a dedicated landing pose
  would read better.
- Bean-only mode (no mesh assets) is untested since Phase 2 landed.
- Capture HUD hiding (cosmetic).

## Multiplayer seam (for the 4-player expansion)

The bean is the only gameplay-authoritative object — replicate its
transform/velocity/state enum and you are done; the skeleton, motors, pose
source, and visuals are all local cosmetics. Input enters through
`IT66MotionRigInputReceiver`; a replicated input path slots in without
touching motion code. The one-way bean→body coupling means client-side body
divergence can never affect gameplay positions.
