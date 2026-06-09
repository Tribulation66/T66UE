# Operator Draft: FriendSlop Hero 1 Skeletal/Ragdoll Phase

Task contract:
- Operator: Codex
- Validator: Claude
- Scope: make FriendSlop skeletal Hero 1 Chad the default selected/runtime hero with compatible walk/idle/jump/roll animation assets, remove TestRoom skeletal override dependency so TestRoom uses selected hero visuals, improve wipeout-arm camera follow during ragdoll, and replace the loose generated ragdoll with a controlled/limited PhysicsAsset profile.
- Animation note: the implemented FriendSlop animation assets are generated compatible action-sources using the same conservative gameplay clip roles as the previous AnimatedToonStyle bridge (`Walk_Fwd_Loop_LegsTorsoOnly`, `Idle_No_Loop`, `DoubleJump_LegsTorsoOnly`, `Roll_LegsTorsoOnly`). They are not a true authored/retargeted copy of a prior old-skeleton AnimSequence.
- Stop condition: implementation complete, assets imported/reloaded where needed, focused verification and staged standalone attempted, caveats reported.

Implemented:
- Added `Model Generation/Rigging and Animation/Tools/create_friendslop_raw_humanoid_animation_sources.py` to generate FriendSlop-compatible raw humanoid animation-source FBXs for idle, walk, jump, and roll.
- Updated `Model Generation/Rigging and Animation/Tools/import_friendslop_raw_humanoid_rig_to_unreal.py` to import the skeletal mesh plus those animation assets.
- Updated `Content/Data/CharacterVisuals.csv` so `Hero_1_Chad` now resolves to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop` with FriendSlop walk/idle/jump/roll AnimSequences. The static mesh path is empty for that default row.
- Reloaded `/Game/Data/DT_CharacterVisuals`.
- Removed the stale `t66.TestRoom.UseSkeletalChadOneOverride` cvar/helper/state from `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`; TestRoom now uses the selected hero visual path.
- Updated wipeout-arm ragdoll impact behavior to simulate from the selected ragdoll root, use pelvis/root impulse instead of equal impulse on every body, start Physical Animation after a short delay, and suppress control/auto-attack during incapacitation.
- Updated wipeout-arm actor/camera follow so the hero actor tracks the ragdoll pelvis X/Y while preserving bounded gameplay Z. This fixes the prior camera dive below the room.
- Detached the skeletal mesh during ragdoll with world transform preservation, then reattached/restored it on recovery. This keeps the simulated body visible while the actor/camera follow target moves with the pelvis.
- Added `t66.TestRoom.ShowCeiling` with default hidden ceiling render in the TestRoom capture path; collision remains, but the low ceiling no longer blocks visual proof by default.
- Scope additions surfaced for final answer: idle/jump/roll were imported alongside walk to keep the existing `CharacterVisuals.csv` animation contract complete, and `t66.TestRoom.ShowCeiling` was added to unblock TestRoom ragdoll readability during capture.
- Updated `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp` to generate a controlled FriendSlop PhysicsAsset: 18 bodies, 15 constraints, pruned body set, limited swing/twist ranges, disabled constrained-body collision, damping/mass scaling, and core Physical Animation drive support.
- Updated `Source/T66/Gameplay/GameMode/pending_issues_GameMode.md` back to no current GameMode pending issues after the ragdoll visibility/proof-camera issue was resolved.

Verification:
- Blender animation-source generation completed exit 0 and wrote FBXs/manifest under `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/Rigging/AnimationSources/`.
- Unreal full-editor import completed exit 0 and imported FriendSlop skeletal plus idle/walk/jump/roll AnimSequences. Report: `Reports/AgentReviews/FriendSlopUnrealRagdollImport/friendslop_humanoid_skeletal_import_report.json`.
- DataTable reload via `Scripts/SetupCharacterVisualsDataTable.py` completed exit 0.
- `Scripts/VerifyFriendSlopHero1SkeletalVisualAndExit.py` completed exit 0; report has `ok: true`, expected `Hero_1_Chad` skeletal/animation paths, and all assets loaded.
- PhysicsAsset commandlet completed exit 0; report: `Reports/AgentReviews/FriendSlopUnrealRagdollImport/friendslop_humanoid_controlled_physics_asset_report.json`.
- Editor build completed exit 0 after the final detach/reattach and ceiling-render changes.
- Gameplay capture completed: `Reports/AgentReviews/FriendSlopImplementationPhase/wipeout_arm_selected_hero_chase_detached_early.mp4` with 120 frames at 1280x720. Frames `frame_0000.png`, `frame_0015.png`, and `frame_0035.png` show the selected FriendSlop Hero 1 visible through impact/flop, ground drag/settle, and recovery.
- Final capture logs prove direct TestRoom selected Hero 1, `VisualID=Hero_1_Chad`, active ragdoll impact with 18 runtime bodies/18 physics bodies/15 constraints, PAC activation with 6 driven bodies, natural settle recovery, and restored control at sane actor Z around 102.
- `Scripts/StageStandaloneBuild.ps1` completed exit 0; staged standalone is ready at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Shortcut checks passed for `C:\UE\T66\T66 Standalone.lnk` and the taskbar shortcut; both target the staged executable and the target exists.

Caveats:
- The animation assets are compatible FriendSlop raw humanoid placeholder/action-source assets using the same conservative gameplay clip roles as the previous bridge, not final production-authored locomotion polish and not a true old-skeleton retarget.
- Mechanical proof and basic visual readability are good enough to proceed to user feel tuning; the exact launch, duration, PAC strength, and recovery values are intentionally not final.
- A previous `UnrealEditor-Cmd.exe -run=pythonscript` import attempt hit a UE Slate/Interchange assertion; the full-editor import path then succeeded and produced the usable assets.

PPF close:
- Process used: FriendSlop raw humanoid rigging/import path plus accepted raw humanoid animation-source path, then Unreal-owned import, commandlet, capture, and staged standalone verification.
- Matches declared process: YES for the implemented phase.
- Evidence: import/report JSONs, DataTable verifier JSON, controlled PhysicsAsset report JSON, editor build, gameplay capture/log markers, staged standalone build, shortcut verification.
