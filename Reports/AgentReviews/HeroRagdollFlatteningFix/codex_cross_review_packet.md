# Codex Cross-Review Packet: Hero Ragdoll Flattening Fix

## Original User Request

Fix the Hero 1 active-ragdoll flattening effect. Use Blender MCP if needed, keep working until the flattening effect is solved, and use goal tracking if helpful.

## Task Contract

Operator: Codex
Validator: Claude

Scope: Implement and verify the Hero 1 PhysicsFirst PhysicsAsset/rig/runtime physics needed to eliminate the active-ragdoll flattening effect under the TestRoom `heroactiveragdollproof` impact. Use Blender MCP only if live inspection or rig-side correction is needed; otherwise fix Unreal PhysicsAsset/physics configuration directly.

Stop condition: Stop only when current proof shows Hero 1 remains visually coherent through impact and recovery, or when a hard engine/tool limitation blocks further progress and is documented with evidence.

## Process Used

- Followed `Gameplay/Physics/PHYSICS_AGENTS.md`.
- Followed `Gameplay/Physics/PhysicsAssetPipeline.md`.
- Used Unreal-owned gameplay capture, not desktop screenshots.
- Did not use Blender MCP because the confirmed defect was Unreal runtime ownership/root simulation behavior, not the source rig mesh hierarchy.

## Root Cause Found

The flattening was not primarily a missing body-volume problem after the PhysicsAsset had 18 bodies / 17 constraints. The repeated failure in Tune9 through Tune14 was caused by simulating the pelvis/root body on the primary `ACharacter` mesh. UE/Chaos then rewrote or rebased the skeletal mesh component/physics body transform relationship, which produced world/local transform divergence and repeated pelvis resyncs. Diagnostics showed mesh relative transforms becoming world-like and pelvis/body targets separating from the capsule/actor.

## Implemented Fix Class

- Keep capsule/character as the owning locomotion frame.
- Keep the pelvis/root body kinematic under the capsule.
- Simulate child bodies below pelvis only.
- Use local physical-animation drive on child bodies.
- Push obstacle reaction into child simulated bodies and launch the capsule/character movement with the same impact velocity class.
- Harden the PhysicsAsset body/constraint report and tuning commandlet path so body volume, constraints, collision policy, mass/inertia, and solver settings remain inspectable and repeatable.

Key code locations:

- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
- `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp`

## Evidence

Focused compile:

- `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
- Result: succeeded before the final proof capture.

Final Unreal-owned proof capture:

- Video: `C:\UE\T66\Saved\AgentReviews\HeroRagdollFlatteningFix\hero_active_ragdoll_tune16_close_acceptance.mp4`
- Contact sheet: `C:\UE\T66\Saved\AgentReviews\HeroRagdollFlatteningFix\Tune16Evidence\contact_sheet.png`
- Manifest: `C:\UE\T66\Saved\AgentReviews\HeroRagdollFlatteningFix\Tune16Evidence\manifest.json`
- ffprobe from manifest: 1280x720, H.264, 12 fps, 7.0 sec, 84 frames.

Final proof log summary from `Saved\Logs\T66.log`:

- No `Pelvis divergence` warnings in the Tune16 proof tail.
- Hit transitions through `Balanced -> KnockedDown -> Recovering -> Balanced`.
- First captured hit: `Reaction Applied=1 Source=TestRoomWipeoutArm`, applied velocity roughly `V(X=1661, Y=341, Z=121)`.
- Actor/pelvis follow stays bounded after hit, including distances around 86, 37.9, and 21.9 instead of the previous 800-1200+ divergence loop.

Staged standalone verification:

- `Scripts\StageStandaloneBuild.ps1`
- Result: `BUILD SUCCESSFUL`; staged executable ready at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Shortcut check: `C:\UE\T66\T66 Standalone.lnk` targets `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Draft Closeout

PPF CLOSE

Process used: `Gameplay/Physics/PhysicsAssetPipeline.md` plus PHYSICS_AGENTS active-ragdoll runtime proof.

Matches declared process: YES

Evidence: focused compile succeeded; Unreal-owned Tune16 capture shows coherent Hero 1 silhouette through hit/recovery; log has no pelvis divergence loop; staged standalone build succeeded and shortcut target is correct.

MECHANISM CLOSE

- Body volume: PRESENT. PhysicsAsset tuning/report path now records and hardens primitive radii/length/mass/solver fields.
- Constraints block inversion: PRESENT. Runtime and commandlet tuning keep locked linear axes, limited angular constraints, projection/shock propagation, and child-local PAC drive.
- Self-collision/contact policy: PRESENT. Internal body-pair collision is disabled for the tuned asset path to avoid self-punching collapse; mesh collision blocks world static and ignores unrelated game channels.
- Solver/mass/inertia stability: PRESENT. Body solver iteration overrides, damping, inertia conditioning, CCD, and mass minimums are configured in the tuning path.
- Active recovery: PRESENT. Tune16 shows Balanced to KnockedDown to Recovering to Balanced with bounded pelvis/capsule distance.

Residual caveat: The flattening loop is solved. Fall-Guys-like feel tuning remains separate: bounce amount, wobble looseness, recovery timing, and trap impulse taste still need iterative gameplay tuning.
