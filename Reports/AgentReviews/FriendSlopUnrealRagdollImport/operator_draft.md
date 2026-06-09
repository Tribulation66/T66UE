# Operator Draft - FriendSlop Hero 1 TestRoom Ragdoll Import

## Scope

Implemented the Unreal side of the FriendSlop Hero 1 skeletal-ragdoll TestRoom spike. Scope stayed isolated to the TestRoom override/proof path and generated import/validation tooling. Normal Hero_1 static visual data, CharacterVisuals CSVs, ToonStyle production import paths, Quad Retro, tint, and outline sidecars were not intentionally changed.

## Changes

- Imported `Hero_1_Chad_Male_FriendSlop_Skeletal.fbx` into `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal`.
- Created/assigned `PA_Hero_1_Chad_Male_FriendSlop_TestRoom` for the new skeletal mesh.
- Added import and validation commandlet/Python tooling under `Model Generation/Rigging and Animation/Tools`.
- Updated the TestRoom Hero 1 Chad skeletal override to use the new FriendSlop skeletal mesh, +X visual-front rotation, pelvis/spine body selection, and physics report logging.
- Updated `testragdoll`/`testragdollproof` automation capture to use an inside-the-room proof camera that frames the wipeout arm and skeletal Chad.
- Updated `Source/T66/Gameplay/pending_issues_Gameplay.md`: capture framing resolved; PAC instability remains a major follow-up.

## Evidence

- Import report: `C:\UE\T66\Reports\AgentReviews\FriendSlopUnrealRagdollImport\friendslop_humanoid_skeletal_import_report.json`
- Validation report: `C:\UE\T66\Reports\AgentReviews\FriendSlopUnrealRagdollImport\friendslop_humanoid_skeletal_validation_report.json`
  - `ok: true`
  - mesh height `180.0 cm`
  - assigned PhysicsAsset `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/PA_Hero_1_Chad_Male_FriendSlop_TestRoom`
  - material slot populated with `MI_SK_Hero_1_Chad_Male_FriendSlop`
- PhysicsAsset report: `C:\UE\T66\Reports\AgentReviews\FriendSlopUnrealRagdollImport\friendslop_humanoid_physics_asset_report.json`
  - `ok: true`
  - `body_count: 24`
  - `constraint_count: 23`
  - missing required body bones: none
- Focused editor build passed:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
- Unreal-owned passive ragdoll proof capture:
  - `C:\UE\T66\Saved\AgentReviews\FriendSlopUnrealRagdollImport\testragdoll_friendslop_passive_final.mp4`
  - 120 frames, 1280x720, 12 fps, 10 seconds; ffmpeg decode passed.
  - sampled frames show skeletal Chad, wipeout arm contact, ragdoll on floor, and later arm passage.
  - log confirms `PhysicalAnimation=0`, 24 bodies, 23 constraints, recovery start, and hero control restore.
- PAC-enabled comparison:
  - Command applied `t66.TestRoom.WipeoutArmEnablePhysicalAnimation=1`.
  - Log confirmed first impact with `PhysicalAnimation=1`.
  - Capture wrote only 3 frames and timed out after 300 seconds, so PAC remains disabled by default and documented as a follow-up.
- Staged standalone build passed:
  - `Scripts\StageStandaloneBuild.ps1`
  - staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - both standalone shortcuts point to the staged exe.

## Caveats

- The generated PhysicsAsset is acceptable for a passive TestRoom proof, but it is not production-tuned. It includes broad generated bodies, including bodies on non-deform `Hero_1_Chad_Male_FriendSlop_Armature` and `root`, and needs hand tuning before PAC can be accepted.
- The visual proof validates the baseline passive ragdoll/incapacitation loop, not final Fall Guys-style active-ragdoll feel.
- No Git staging/commit was requested or performed.

## PPF Close

Process used: T66 Unreal import/validation plus TestRoom proof capture, using the Blender-produced FriendSlop raw humanoid rig as the source artifact.

Matches declared process: Partial, because the PAC active drive remains unaccepted.

Evidence: Import, PhysicsAsset creation, validation, focused build, passive TestRoom video proof, and staged standalone build are complete. PAC active drive was tested but remains unstable and disabled by default, so the current result is a validated passive-ragdoll infrastructure spike, not final active-ragdoll feel.
