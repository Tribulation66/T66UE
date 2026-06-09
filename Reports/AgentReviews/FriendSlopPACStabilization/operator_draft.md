# FriendSlop PAC Stabilization Operator Draft

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: make the FriendSlop Hero 1 TestRoom active-ragdoll path stable enough for design feel tuning; stop before subjective bounce/feel tuning that needs player/designer review.
Stop condition: PAC-enabled TestRoom capture completes without hang/crash, logs PhysicalAnimation=1, shows impact/ragdoll/recovery, focused build passes, staged standalone is refreshed and shortcuts point at the staged exe, and remaining subjective tuning knobs are identified.

## Changed Scope

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - Enabled the TestRoom wipeout-arm PAC prototype by default for the Hero 1 Chad FriendSlop skeletal override.
  - Kept full-body Chaos simulation and impulse for the launch, then enables PAC after a short delay so pose buffers/runtime bodies exist before `UPhysicalAnimationComponent` updates.
  - Added PAC drive mode and activation-delay CVars, using core-chain drive mode by default.
  - Uses world-space PAC drive with reduced strength/force constants.
  - Restores visual ragdoll by removing forced ref-pose rendering after transform allocation.
  - Clamps restore height so recovery does not reattach the hero below the floor.

- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Changed the TestRoom ragdoll proof camera to use the pawn actor location for X/Y, a fixed low focus Z, and a clamped camera Z. This prevents ragdoll physics bounds from placing the camera above/inside the TestRoom ceiling.

- `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - Replaced the obsolete unresolved PAC-stability entry with a resolved entry pointing at the accepted proof and listing tuning knobs still requiring design review.

- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
  - Fixed a pre-existing, unrelated UE 5.7 build break required to let the focused build pass by treating `FSlateApplication::Get().GetRenderer()` as a raw `FSlateRenderer*` instead of `TSharedPtr<FSlateRenderer>`.

## Verification

- Focused build passed:
  - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`

- Accepted Unreal-owned gameplay proof:
  - Video: `C:\UE\T66\Saved\AgentReviews\FriendSlopPACStabilization\testragdoll_friendslop_pac_hero1_observer_delay05.mp4`
  - Frames: `C:\UE\T66\Saved\AgentReviews\FriendSlopPACStabilization\frames_pac_hero1_observer_delay05`
  - ffprobe: 1280x720, 12 FPS, 10.0 seconds, 120 frames.
  - Visual samples show the arm contact/launch and visible flipped ragdoll pose, then recovery.
  - Log markers include repeated PAC activation across the clip:
    - `TestRoom wipeout arm active ragdoll impact`
    - `PhysicalAnimation=1 DriveMode=2 DrivenBodies=6 RuntimeBodies=24 ComponentTransforms=24 BoneTransforms=24`
    - `TestRoom wipeout arm ragdoll recovery started`
    - `TestRoom wipeout arm restored hero control`
  - Latest accepted proof log did not emit fatal or assertion output. There is not yet per-frame PAC telemetry; stability evidence is the 120-frame no-timeout capture plus repeated activation/recovery/restore markers rather than a per-frame PAC state stream.

- Staged standalone refreshed:
  - `Scripts\StageStandaloneBuild.ps1`
  - Staged exe exists: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Both `C:\UE\T66\T66 Standalone.lnk` and the taskbar pinned shortcut target the staged exe.

## Remaining Tuning Boundary

The infrastructure is ready for subjective feel tuning, but the values are not production feel-approved. The knobs intentionally left for review are:

- Launch XY/Z.
- PAC activation delay.
- PAC strength and driven body set.
- Incap duration and max ragdoll time.
- Settle speed/hold and blend-out seconds.
- Wipeout arm speed, height, length, and collision feel.
- Gameplay/proof camera framing.

## Caveats

- This is still a TestRoom Hero 1 prototype on the FriendSlop skeletal mesh and generated test PhysicsAsset. It is not yet the production all-hero/boss active-ragdoll architecture.
- Some touched files had pre-existing unrelated edits in the working tree. The relevant changes above are the scope reviewed here; unrelated edits were left untouched except for the UI build-break fix required to compile.
