Working task:
Operator: Codex
Validator: Claude
Scope: fix the hero-scoped FriendSlop ragdoll smear/drag and camera/controller disconnect while keeping the knockback infrastructure reusable.
Stop condition: code/data changes are made, focused compile and Unreal-owned TestRoom proof are attempted, staged standalone validation is attempted, and remaining visual/rigging limitations are stated.

Codex implementation summary:
- Updated `FT66KnockbackProfile` in `Source/T66/Gameplay/T66KnockbackComponent.h` with profile-driven knobs:
  - `bSimulateAllPhysicsBodies`
  - `bUsePreImpactActorToFollowBoneOffset`
  - `bSuppressLookInput`
- Updated `UT66KnockbackComponent` runtime behavior:
  - Ragdoll start now simulates all PhysicsAsset bodies by default instead of only bodies below pelvis.
  - Actor/camera follow now centers on the live ragdoll follow bone by default instead of preserving the pre-impact capsule-to-pelvis offset.
  - Controller look input is suppressed while incapacitated and restored on recovery, matching existing movement suppression.
  - Launch logs now report all-body simulation, centered follow, and look suppression flags.
- Updated TestRoom wipeout-arm profile creation in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` with CVars:
  - `t66.TestRoom.WipeoutArmSimulateAllBodies`
  - `t66.TestRoom.WipeoutArmCenterActorOnRagdoll`
  - `t66.TestRoom.WipeoutArmSuppressLookInput`
- Fixed `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp` so the controlled PhysicsAsset remains a connected body graph after pruning helper bodies:
  - The generator now adds missing constraints from kept child bodies to nearest kept ancestors.
  - This repaired the two missing upper-arm constraints after clavicle-body pruning.
- Regenerated `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/PA_Hero_1_Chad_Male_FriendSlop_TestRoom`.

Key evidence:
- Focused compile succeeded:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
- PhysicsAsset commandlet succeeded:
  - Reconnected `upperarm_l` to `spine_03`.
  - Reconnected `upperarm_r` to `spine_03`.
  - Report now shows 18 bodies and 17 constraints.
  - Report path: `Reports/AgentReviews/FriendSlopUnrealRagdollImport/friendslop_humanoid_controlled_physics_asset_report.json`.
- Unreal-owned chase-camera capture succeeded:
  - `Saved/AgentReviews/FriendSlopRagdollCameraFollow/testragdoll_chase_after_followfix.mp4`
  - Sampled frames show the ring/camera centered on the ragdolled body during knockdown/recovery; the earlier long smear-to-origin is not present in sampled frames.
- Runtime logs from chase capture show:
  - `SimulateAllBodies=1`
  - `CenterActorOnFollow=1`
  - `SuppressLook=1`
  - `RuntimeBodies=18`
  - `PhysicsConstraints=17`
  - PAC activation with `DrivenBodies=6`
  - recovery and restore logs.
- Fixed proof-camera capture also succeeded:
  - `Saved/AgentReviews/FriendSlopRagdollCameraFollow/testragdoll_proof_after_graphfix.mp4`
  - It is less useful for final visual judgment because the ragdoll moves out of the fixed camera and the wipeout arm occludes the frame.
- `git diff --check` on touched source files passed, with only an LF-to-CRLF warning for the existing TestRoom file.
- `StageStandaloneBuild.ps1 -ClientConfig Development` succeeded on the second attempt. The first attempt hit the known AutomationTool mutex conflict; no live AutomationTool/dotnet process was found, then rerun succeeded.
- Packaged direct-entry TestRoom screenshot smoke succeeded:
  - exe: `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`
  - screenshot: `Saved/Codex/FriendSlopRagdollCameraFollow/staged_testroom_smoke.png`
  - log: `Saved/StandaloneLogs/FriendSlopRagdollCameraFollow_TestRoomScreenshotSmoke.log`
  - log proves direct entry configured `Run:TestRoom`, selected `Hero_1`, loaded FriendSlop skeletal mesh/animations, scheduled wipeout arm, and produced knockback logs with `SimulateAllBodies=1`, `CenterActorOnFollow=1`, `SuppressLook=1`, `PhysicsConstraints=17`.

Residual caveat:
- The structural engine problems are addressed, but if the user still calls the remaining orange/black protrusion a drag effect, that is likely mesh weighting/fused coat geometry rather than the old disconnected-physics path. The next pass would be Blender/skin-weight cleanup or costume segmentation, not more C++ tuning.

Please cross-review for missed constraints, risky assumptions, verification gaps, or wording that overclaims visual quality.
