# Codex Draft: FriendSlop Ragdoll Follow/Floor Guard

## Scope

Fix the active TestRoom ragdoll bugs reported by the user:

- Outer-side wipeout arm hits should keep the owning hero actor/camera anchor with the thrown ragdoll instead of snapping back near spawn.
- Ragdoll bodies should not go halfway under the ground.

## Changes Made

## Root Cause

- Camera/actor detachment: the camera follows the hero actor root, not the detached skeletal mesh. During ragdoll, repeated launches could start Chaos body bounds from a stale origin while the owning actor was elsewhere, so the actor/camera root could appear to snap away from the visible body or only catch up at recovery.
- Ground penetration: the ragdoll had no hard body-bounds floor gate, and dynamic floor traces could also mistake TestRoom obstacle tops for the floor. The fix resolves the launch/session floor once and enforces a minimum body-bounds clearance against that floor.
- PAC instability: Physical Animation Component drive on the current detached ragdoll path pulled bodies into an invalid coordinate frame in live testing. PAC is therefore gated off for detached ragdoll until a separate attached/target-space PAC path is built.

- `Source/T66/Gameplay/T66KnockbackComponent.h`
  - Added profile fields for simulated-body actor follow and floor penetration guard tuning.
  - Added helper declarations for body-bounds follow, floor tracing, floor-anchor actor Z, kinematic body sync, and floor guard enforcement.

- `Source/T66/Gameplay/T66KnockbackComponent.cpp`
  - Actor/camera follow now uses simulated physics body bounds for XY but anchors Z to resolved floor plus capsule half-height.
  - Floor guard resolves a session floor at impact and uses that for the active ragdoll, avoiding later false floors from TestRoom obstacle tops.
  - Added per-tick floor penetration correction that lifts all simulated bodies together if any body bounds dip below `floor + skin`, zeroing downward velocity on corrected bodies.
  - Added an initial body resync gate after physics bodies are created. If Chaos starts a repeated ragdoll with body bounds still near a stale origin, the bodies are translated as a group to the current actor position before launch impulse.
  - Added kinematic mesh/body sync around physics-state transitions.
  - Gated Physical Animation Component off for detached ragdoll profiles. The attached-PAC attempt was tested and crashed/dragged bodies downward; detached pure ragdoll is the stable path for this bug fix.

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - Wipeout arm profile now explicitly uses detached ragdoll and disables PAC for this prototype path so the log/proof matches runtime behavior.

## Proof

- Focused editor compile passed:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - Existing unrelated warning remains: `T66Hero1AxeAOEVFXLabActor.cpp` uses deprecated `FNiagaraEmitterInstance::IsReadyToRun`.

- Editor TestRoom auto-capture passed:
  - Video: `C:\UE\T66\Saved\AgentReviews\FriendSlopRagdollFollowGroundGuard\testragdoll_follow_floor_guard_editor_v9.mp4`
  - Frames: `C:\UE\T66\Saved\AgentReviews\FriendSlopRagdollFollowGroundGuard\frames_follow_floor_guard_editor_v9`
  - Log evidence from `Saved\Logs\T66.log`:
    - launches show `PACPending=0 DriveMode=0`
    - repeated hits show `T66Knockback initial body resync`
    - floor guard lines stay at `FloorZ=-0.0` / `FloorZ=0.0`
    - restore/recovery actor locations stay at `Z=100.00`
    - active in-flight follow samples show `ActorToTargetXY=0.0` while body center moves through the air.

- Standalone stage passed:
  - `Scripts\StageStandaloneBuild.ps1`
  - Exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Shortcut target verified as the staged exe.

- Packaged/staged TestRoom auto-capture passed:
  - Log: `C:\UE\T66\Saved\AgentReviews\FriendSlopRagdollFollowGroundGuard\testragdoll_follow_floor_guard_staged_v2.log`
  - Video: `C:\UE\T66\Saved\AgentReviews\FriendSlopRagdollFollowGroundGuard\testragdoll_follow_floor_guard_staged_v2.mp4`
  - Frames: `C:\UE\T66\Saved\AgentReviews\FriendSlopRagdollFollowGroundGuard\frames_follow_floor_guard_staged_v2`
  - Log evidence:
    - staged exe exits with code 0 and writes 120 frames.
    - `PACPending=0 DriveMode=0`
    - `T66Knockback initial body resync` on repeated hits.
    - all floor guard lines in staged proof use `FloorZ=-0.0` / `FloorZ=0.0`, not `120` or `160`.
    - lift corrections are present, e.g. `BodyMinZ=-14.0 AllowedMinZ=4.0 Lift=18.0`, proving the guard actively corrected penetration.
    - active in-flight follow samples show `ActorToTargetXY=0.0`, proving the actor/camera root tracks the body-center target before recovery.
    - recovery/restore actor locations stay at `Z=100.00`.

## Caveats

- This solves the two reported bug gates, but it does not claim final active-ragdoll feel tuning.
- PAC is deliberately disabled for the current detached ragdoll path. The attempted attached-PAC direction was rejected because live proof showed body collapse/crash. A future active-ragdoll phase should solve PAC target/component-space ownership separately before re-enabling PAC.
- The auto-capture camera is a proof harness, not a replacement for the user's manual feel test.
- The harness exercises repeated wipeout hits from the rotating arm path. It does not isolate a named "outer-side only" case in the log, but it does cover the stale-body-origin failure that caused outer-side snaps and proves actor/body XY convergence during active flight.
