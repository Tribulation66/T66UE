# Codex Draft: Wipeout Force Launch / Ground Snap

## Scope

Fix the current TestRoom wipeout-arm behavior where the hero could sink below the floor, appear to return near the original location, and fail to be launched far enough out of the trap radius. Keep the path pure C++ / pure Chaos for heroes; PAC must remain disabled for hero ragdoll.

## Changes

- `Source/T66/Gameplay/T66KnockbackComponent.h`
  - Added reusable `FT66KnockbackProfile` physics-response fields:
    - `RagdollLinearDampingOverride`
    - `RagdollAngularDampingOverride`
    - `RagdollFrictionOverride`
    - `RagdollRestitutionOverride`
  - Added runtime state for caching/restoring body damping and physical-material overrides during ragdoll.

- `Source/T66/Gameplay/T66KnockbackComponent.cpp`
  - Clamps the new profile overrides when a knockback starts.
  - Caches each skeletal physics body's pre-impact damping and physical material override.
  - Applies optional runtime damping and a transient `UPhysicalMaterial` override to all simulated ragdoll bodies.
  - Restores each body's previous damping/material settings on recovery.
  - Keeps the existing current-XY floor trace/floor guard and carry-velocity restore behavior.
  - Keeps hero PAC disabled; packaged proof logs show `PACPending=0 DriveMode=0`.

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - Increased wipeout-arm radial launch from the earlier stronger pass to `10500`.
  - Increased `WipeoutArmRagdollMaxSeconds` to `3.10` for a wall-contact window.
  - Added TestRoom cvars for ragdoll damping/friction/restitution:
    - `t66.TestRoom.WipeoutArmLinearDamping=0.01`
    - `t66.TestRoom.WipeoutArmAngularDamping=0.02`
    - `t66.TestRoom.WipeoutArmFriction=0.04`
    - `t66.TestRoom.WipeoutArmRestitution=0.72`
  - Wipeout impact log now reports damping/friction/restitution.

- `Source/T66/T66.Build.cs`
  - Added public `PhysicsCore` dependency because `UPhysicalMaterial` is now used in a public component header.

## Git/Tracking Caveat

`Source/T66/Gameplay/T66KnockbackComponent.cpp` and `.h` are currently untracked in this working tree. The focused build and staged standalone did compile them from disk, but they are not tracked by Git at the moment. I did not stage or change tracking state.

## Verification

- Focused build:
  - Command: `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - Result: succeeded after adding `PhysicsCore`.

- Staged standalone refresh:
  - Command: `C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
  - Result: `BUILD SUCCESSFUL`
  - Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Shortcut target verified: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

- Packaged TestRoom capture:
  - Log: `C:\UE\T66\Saved\AgentReviews\WipeoutForceLaunchGroundSnap\testragdoll_force_launch_staged_v3.log`
  - Frames: `C:\UE\T66\Saved\AgentReviews\WipeoutForceLaunchGroundSnap\frames_staged_v3`
  - Frame count: 220 PNGs.
  - Launch log:
    - `Velocity=V(X=10280.41, Y=2136.15, Z=750.00)`
    - `MaxRagdoll=3.10s`
    - `LinearDamping=0.01 AngularDamping=0.02 Friction=0.04 Restitution=0.72`
    - `PACPending=0 DriveMode=0`
  - Impact log:
    - `Launch=V(X=10280.41, Y=2136.15, Z=750.00)`
    - `Radial=V(X=1.00) Tangent=V(X=0.18, Y=0.98)`
    - `PAC=0 DriveMode=0`
  - Actor follow samples:
    - Start: `Actor=V(X=850.11, Y=-0.13, Z=100.00)`
    - Mid launch: `Actor=V(X=7131.97, Y=1296.18, Z=100.00)`
    - Far wall/bounce area: `Actor=V(X=14731.09, Y=2881.40, Z=100.00)`
    - Post-contact/recovery area: `Actor=V(X=14373.04, Y=3367.68, Z=100.00)`
    - Every sampled line reports `ActorToTargetXY=0.0`.
  - Bounce evidence:
    - Actor moves out from `X=850.11` to `X=14731.09`, then returns to `X=14373.04`.
    - Restore carry velocity is negative on X/Y: `CarryVelocity=V(X=-625.44, Y=-701.46)`.
  - Floor guard evidence:
    - Initial floor correction: `BodyMinZ=-14.0 AllowedMinZ=4.0 Lift=18.0`.
    - Recovery floor correction: `BodyMinZ=-20.0 AllowedMinZ=4.0 Lift=24.0`.
    - Actor remains restored at `Z=100.00`.
    - Live code traces from the current simulated body-bounds center before falling back to cached/pre-impact floor Z (`ResolveRagdollFloorZ` probes `BodyBounds.GetCenter()` first). This proof is still only a flat TestRoom-floor proof, not an uneven-terrain proof.
  - Log check:
    - No `Fatal`, `Critical error`, `Assertion`, or `ensure` lines matched in the proof parse.

## Remaining Caveats

- The mechanics now launch far and bounce back, but the capture shows the wipeout arm can fill/obstruct the camera during the high-speed launch. That is a camera-obstruction/feel tuning problem, not the original actor-detach snap-back bug.
- The current forced ragdoll window is `3.10s` to prove wall contact and bounce. If the desired final play feel returns control faster, the next tuning pass should decide whether recovery should happen immediately after a confirmed wall/ground contact instead of on a fixed max timer.

## PPF Close

Process used: existing T66 C++ knockback/TestRoom trap path with pure Chaos hero ragdoll, profile-level physics response tuning, focused build, staged standalone refresh, and Unreal-owned packaged capture/log proof.

Matches declared process: YES

Evidence: build/stage/capture paths above; proof log shows PAC disabled, radial launch, zero actor-follow divergence, floor-guard corrections, and bounce-back carry velocity.
