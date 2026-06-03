# T66 Hygiene Candidate Manifest

Date: 2026-05-27
Status: Gate 0 inventory complete; Gate B HillTile cleanup complete; Gate A cleanup complete; Gate C generated-output cleanup complete
Durability: durable manifest, not a raw run folder

## Scope Boundary

This manifest records cleanup candidates and required proof. It does not authorize deletion. Deletions require the later Gate A/B/C confirmations described in `Reports/Hygiene/2026-05-27/README.md`.

Final per-pass scope boundary: `Reports/Hygiene/2026-05-27/hygiene_change_scope.md`.

## Gate 0 Environment Evidence

- `T66.uproject` engine association: `5.7`.
- UE commandlet path exists: `C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe`.
- Existing world asset audit script exists: `Scripts/AuditWorldAssetsAndExit.py`.
- Existing native class/member reference audit helper: not found before Gate 0B.

## Gate 0 Commandlet Evidence

Native reference dry-run:

- Script: `Scripts/AuditNativeClassReferencesAndExit.py`.
- Output: `Reports/Hygiene/2026-05-27/native_reference_audit_dryrun.json`.
- Commandlet result: success, `0 error(s)`.
- Target set: `AT66StageGate`, `AT66IdolAltar`, `GetMovementSpeedSecondaryMultiplier`, `HasQuickReviveCharge`.
- Map roots: `/Game/Maps`.
- Result counts: 2 candidate maps, 2 maps loaded, 0 map matches, 3,645 binary content packages scanned, 0 binary matches, 892 text files scanned, 37 text matches.
- Scope caveat: the 0 map/binary matches are evidence only for this target set, the `/Game/Maps` map-load scope, AssetRegistry/Blueprint-tag inspection, raw package token scan, and configured text roots. They are not global deletion clearance.

Native reference positive control:

- Output: `Reports/Hygiene/2026-05-27/native_reference_audit_positive_control.json`.
- Commandlet result: success, `0 error(s)`.
- Positive-control target set: `AT66GameMode`, `AT66HeroBase`, `GetMovementSpeedSecondaryMultiplier`, `BackroomsQuickReviveItemID`.
- Positive hits: 4 AssetRegistry hits, 4 Blueprint matches, 2 raw binary package matches, 106 text matches.
- Binary proof examples: `Content/Blueprints/Core/BP_HeroBase.uasset`, `Content/Blueprints/GameModes/BP_GameplayGameMode.uasset`.
- Text proof examples: `Source/T66/Core/T66RunStateSubsystem.h`, `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp`.

World asset audit:

- Script: `Scripts/AuditWorldAssetsAndExit.py`.
- Original output: `Saved/Audits/WorldAssetAudit.json`.
- Durable hygiene copy: `Reports/Hygiene/2026-05-27/world_asset_audit.json`.
- Copy type: point-in-time snapshot of the generated ignored audit output, not a live regenerated artifact.
- Commandlet result: success, `0 error(s)`.
- Result count: 507 world assets audited.

## Gate 0 Git Footprint Evidence

- Tracked-only status after Gate 0 still contains unrelated modified files outside the Gate 0 artifact paths.
- Gate 0 footprint from narrow untracked status: `Reports/**` plus `Scripts/AuditNativeClassReferencesAndExit.py`.
- Deleted tracked files check: `git diff --name-status --diff-filter=D` produced no deleted-file rows.
- Interpretation: Gate 0 added untracked report/tooling artifacts and did not delete tracked files. Existing unrelated tracked modifications must remain isolated before later Gate A/B/C work.

## Quick Revive

Active behavior to preserve:

- `Content/Data/Items.csv` row `Item_BackroomsQuickRevive`.
- `UT66RunStateSubsystem::BackroomsQuickReviveItemID`.
- `HasBackroomsQuickReviveItem()`.
- `ConsumeBackroomsQuickReviveItem()`.
- Backrooms reward grant and repeat-door exclusion.
- HUD icon and `QuickReviveChanged` refresh path.

Deprecated cleanup candidates:

- `UT66RunStateSubsystem::HasQuickReviveCharge()`.
- `UT66RunStateSubsystem::IsInQuickReviveDownedState()`.
- `UT66RunStateSubsystem::GetQuickReviveDownedSecondsRemaining()`.
- `UT66RunStateSubsystem::GrantQuickReviveCharge()`.
- `UT66RunStateSubsystem::ClearQuickReviveCharge()`.
- `AT66HeroBase::IsQuickReviveDowned()`.
- `AT66QuickReviveVendingMachine` class and source files.
- `/Game/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D`, if present and zero-referenced after source cleanup.

Required proof before deletion:

- Dependency-direction proof that live Backrooms grant/consume/HUD paths do not route through deprecated Quick Revive wrappers.
- Header exposure check plus Blueprint/config/data member-reference audit for every removed wrapper/member.
- Native class reference and map-placement audit before deleting `AT66QuickReviveVendingMachine`.
- AssetRegistry/text-reference proof before deleting any vending mesh asset.

## Movement

Cleanup candidates:

- `UT66RunStateSubsystem::GetHeroMoveSpeedMultiplier()`.
- `FT66HeroMovementTuning::DefaultWalkSpeed`.
- `UT66RunStateSubsystem::GetMovementSpeedSecondaryMultiplier()`, with the caveat that it is a live-callsite no-op refactor, not unreferenced dead code.

Known live callsites for `GetMovementSpeedSecondaryMultiplier()` from Gate 0 search:

- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`.
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`.

Required proof before deletion:

- Full callsite enumeration before editing.
- Header exposure check plus Blueprint/config/data member-reference audit.
- `Config/` and `Content/Data` text sweep for `DefaultWalkSpeed`.
- Formula and run-summary display equivalence proof after removing the `* 1.0` factor.

Preserved:

- `ApplyStatusBurn`, `ApplyStatusChill`, `ApplyStatusCurse`.
- `GetStatusMoveSpeedMultiplier`.
- `T66UniqueDebuffProjectile` status behavior.

## World And Tower Concepts

Concrete code cleanup candidates:

- `SpawnTricksterAndCowardiceGate()` no-op and callsites.
- `SpawnModelShowcaseRow()` no-op and stale caller/comment path.
- Teleporter-pad stale comments/docs only; no concrete teleporter-pad runtime package was found by narrow filename inventory.
- Boss beacon code and fields, including `BossBeaconActor`, `BossBeaconUpdateAccumulator`, `MainMapBossBeaconSurfaceLocation`, `BossBeaconSurfaceLocation`, and spawn/update/destroy helpers, if boss flow proof passes.
- `AT66TutorialGate` class, source includes, and player-controller interaction branch, if native class/map/content proof passes.

Idol test/dev cleanup candidates requiring explicit sub-gate:

- `bSpawnIdolVFXTestTargetsAtStageStart`.
- `IdolVFXTestTargets`.
- `SpawnIdolVFXTestTargets()`.
- `bSpawnPixalTestModelsAtIdolAltar`.
- `SpawnPixalTestDisplayModelsNearIdolAltar(...)`.
- Associated Pixal test display actor tracking.

Preserved:

- Active `AT66IdolAltar` gameplay.
- Active `IdolManager`, idol data, and IdolAltar assets unless a later exact asset-delete review approves them.
- Active `HouseNPC` and `Tractor` naming systems.

Not found / deferred:

- No safe exact packages found yet for legacy tree/dirt/rock/deco/scatter clutter. Narrow `Content/World` search only found unrelated RedRock generated kit assets and active IdolAltar packages.
- Broader hits under VFX/demo/source areas are not world-clutter deletes without a separate exact candidate list and referencer proof.

## Unreal Asset Delete Candidates

Approved by user for Gate B evaluation:

- `Content/World/Cliffs/MI_HillTile1.uasset`
- `Content/World/Cliffs/MI_HillTile2.uasset`
- `Content/World/Cliffs/MI_HillTile3.uasset`
- `Content/World/Cliffs/MI_HillTile4.uasset`
- `Content/World/Cliffs/T_HillTile1.uasset`
- `Content/World/Cliffs/T_HillTile2.uasset`
- `Content/World/Cliffs/T_HillTile3.uasset`
- `Content/World/Cliffs/T_HillTile4.uasset`

Required proof before deletion:

- `Saved/Audits/WorldAssetAudit.json` from `Scripts/AuditWorldAssetsAndExit.py`.
- `T_HillTile1..4` referenced only by approved `MI_HillTile1..4`.
- Each `MI_HillTile1..4` has zero external referencers.
- Targeted per-file recoverability check, not broad asset-tree Git/LFS scans.

Gate 0 current audit result:

- `MI_HillTile1..4`: zero asset referencers.
- `T_HillTile1..4`: each has one asset referencer, its paired `MI_HillTile1..4`.
- Interpretation: Gate B can evaluate these eight files as one deletion chain, but these referencer counts are only one evidence signal. They are not deletion clearance by themselves, and Gate 0 did not delete them.

Gate B completion evidence:

- Review: `Saved/AgentReviews/20260527T_hygiene_gateb_hilltile/20260527T101835-pass6/claude_review_pass6.md`, exact `Verdict: APPROVE`.
- Supplemental audit script: `Scripts/AuditCliffSideMaterialsAndExit.py`.
- Pre-delete CliffSideMaterials audit: `Reports/Hygiene/2026-05-27/cliff_side_materials_audit.json`.
- Pre-delete audit summary: 0 GameMode Blueprint matches, 0 Blueprint load failures, 8 target-only content token matches, 0 non-target content token matches.
- Runtime source cleanup: removed HillTile preload paths from `Source/T66/Core/T66GameInstance.cpp` and removed legacy cliff material defaults from `Source/T66/Gameplay/T66TerrainThemeAssets.cpp`.
- Focused compile: `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE` succeeded before asset deletion.
- Deleted exactly the eight approved `Content/World/Cliffs/*HillTile*.uasset` files listed above.
- Post-delete world audit: `Reports/Hygiene/2026-05-27/world_asset_audit_gateb_post.json` plus `world_asset_audit_gateb_post.meta.json`.
- Post-delete world audit summary: 499 world asset rows, 0 HillTile rows, fresh AssetRegistry scan recorded.
- Post-delete load integrity: `Reports/Hygiene/2026-05-27/world_asset_load_integrity.json`, 2 maps loaded, 499 remaining world assets loaded, 0 failures.
- Post-delete runtime reference sweep: no HillTile matches in `Source`, `Config`, or `Content/Data`; remaining name mentions are confined to this manifest and the Gate B audit helper.

Preserved:

- `Content/World/Backrooms/Textures/T_Backrooms_Door.uasset`
- `Content/World/Backrooms/Textures/T_Backrooms_Floor.uasset`
- `Content/World/Backrooms/Textures/T_Backrooms_Wall.uasset`

## Gate A Completion Evidence

Review:

- Gate A review packet: `Saved/AgentReviews/20260527T_hygiene_gatea_cleanup/plan_packet.md`.
- Claude greenlight: `Saved/AgentReviews/20260527T_hygiene_gatea_cleanup/20260527T105513-pass5/claude_review_pass5.md`, exact `Verdict: APPROVE`.

Deleted old Quick Revive surfaces:

- Removed native vending actor source: `Source/T66/Gameplay/T66QuickReviveVendingMachine.h` and `Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp`.
- Deleted the old Quick Revive vending/icon asset chain listed in `Reports/Hygiene/2026-05-27/gatea_delete_backup_manifest.json`.
- Deleted obsolete `Scripts/ImportInteractableUISprites.py`, whose only job was to recreate the old `QuickReviveIcon` asset.
- Refactored the active HUD revive indicator to use Backrooms-specific names and the active `/Game/Items/Sprites/Item_BackroomsQuickRevive` item path.

Deleted old movement/world surfaces:

- Removed dead movement speed hooks and `DefaultWalkSpeed` technical debt from source/docs.
- Removed no-op or legacy world/tower code surfaces for boss beacon, teleporter pad naming debt, idol dummy targets, start-area scatter, Trickster-in-tower, model showcase rows, and related beacon surface fields.

Reports/process organization:

- Moved tracked ToonStyle reports from `ToonStyle/Reports` to `Reports/ToonStyle`.
- Added the `Reports` routing rules and 15-day whole-run cleanup rule to `AGENTS.md`, `Reports/README.md`, and `Reports/AGENTS.md`.
- Added possible-future-feature index: `Reports/Hygiene/2026-05-27/possible_future_features.md`.

Gate A verification:

- Focused compile passed after source edits: `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`.
- Packaged hero movement QA passed after movement source cleanup: `Reports/Hygiene/2026-05-27/hero_movement_qa_verification.json` records 90 frames, 15 movement samples, stable `MaxWalkSpeed=840.0`, max velocity `840.0`, max delta speed `886.2`, forward displacement `5406.0`, and jump Z movement.
- Run Summary smoke/equivalence proof passed: `Reports/Hygiene/2026-05-27/run_summary_smoke_verification.json` records a staged direct `RunSummary` render/dump with zero `Move Speed Mult` labels, and `Reports/Hygiene/2026-05-27/movement_run_summary_equivalence.md` records the `1.f` no-op formula proof.
- Quick Revive asset audit: `Reports/Hygiene/2026-05-27/asset_reference_audit_gatea_quickrevive_post.json`, clean commandlet exit after the audit stopped asking Unreal for referencers on already-deleted packages.
- Native symbol audit: `Reports/Hygiene/2026-05-27/native_reference_audit_gatea_post.json`; deleted vending class does not resolve, live maps have no matching old actors, and member-token matches are confined to historical reports.
- Class/property audit: `Reports/Hygiene/2026-05-27/class_property_audit_gatea_post.json`; removed CDO properties are unreadable on `AT66GameMode`, with no Blueprint CDO matches.
- World asset audit: `Reports/Hygiene/2026-05-27/world_asset_audit_gatea_post.json` plus `.meta.json`, fresh scan recorded.
- World load integrity: `Reports/Hygiene/2026-05-27/world_asset_load_integrity_gatea_post.json`, 2 maps loaded, 488 remaining world assets loaded, 0 failures.
- Live old-symbol sweep across `Source`, `Config`, `Content/Data`, `Gameplay`, and `Scripts` returned no matches for the exact removed symbols, excluding explicit Mini paths.
- Pre-dirty hunk allow-list check passed: `Reports/Hygiene/2026-05-27/gatea_predirty_allowlist_check.json`.
- Post-edit diff snapshot: `Reports/Hygiene/2026-05-27/gatea_postedit_diff_snapshot.patch`.

## Reports Migration

Target structure:

- `Reports/AgentReviews`.
- `Reports/Hygiene`.
- `Reports/Proof/<Domain>/<TaskSlug>`.
- `Reports/ToonStyle`.
- `Reports/ModelGeneration`.
- `Reports/UI`, `Reports/Gameplay`, `Reports/World`, `Reports/Performance`.

Required proof before moves:

- Code/script/doc/manifest/generated-metadata consumer sweep for old report paths.
- Backward-compatible discoverability for legacy `Saved/AgentReviews`.
- Active review run remains findable until this pass completes.

## Generated Output Cleanup Candidates

Gate C candidates only after remeasurement and keep-lists:

- `Saved/Cooked/Windows` - generated cook output and safest generated delete.
- `Saved/StagedBuilds/WindowsHotfix` - old staged output candidate.
- `Saved/StagedBuilds/WindowsTemp` - old staged output candidate.
- `Saved/StagedBuildsDemo/Windows` - only if the user confirms no active demo target.
- `Saved/VideoCaptures` - only after proving hero selection uses `RuntimeDependencies/T66/Video`, and after migrating or keep-listing cited evidence.
- `Saved/D2` - inferred cleanup addition, not user-named; requires explicit user confirmation after doc keep-list and `Saved/VFXResearch` cross-check.

Preserved:

- `Saved/VFXResearch`.
- Active `Saved/StagedBuilds/Windows` unless restaged and shortcut-verified in the same approved pass.

Gate C completion evidence:

- Initial inventory: `Reports/Hygiene/2026-05-27/gatec_generated_output_inventory.json`.
- Review packet: `Saved/AgentReviews/20260527T_hygiene_gatec_generated_output/plan_packet.md`.
- Claude pass 1 result: `Saved/AgentReviews/20260527T_hygiene_gatec_generated_output/20260527T115231-pass1/claude_review_pass1.md`, `Verdict: REVISE`; accepted objection that `WindowsHotfix`, `WindowsTemp`, and `StagedBuildsDemo` had no proven regeneration owner.
- Claude pass 2 result: `Saved/AgentReviews/20260527T_hygiene_gatec_generated_output/20260527T115426-pass2/claude_review_pass2.md`, exact `Verdict: APPROVE`, with deletion narrowed to `Saved/Cooked`.
- Final editor compile: `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE` returned `Result: Succeeded`.
- Staged standalone refresh ran before deleting cooked output: `Scripts\StageStandaloneBuild.ps1` returned success, refreshed loose runtime roots, and updated both standalone shortcuts.
- Deleted exactly `C:\UE\T66\Saved\Cooked` after resolving the path and verifying the exact `Cooked` leaf under `C:\UE\T66\Saved\`.
- Delete report: `Reports/Hygiene/2026-05-27/gatec_delete_report.json`; pre-delete size was 13.167 GB / 9,304 files, post-existence was `false`, and the staged exe still existed.
- Post inventory: `Reports/Hygiene/2026-05-27/gatec_generated_output_inventory_post.json`; `Saved/Cooked` is absent, while `Saved/StagedBuilds`, `Saved/StagedBuildsDemo`, `Saved/VideoCaptures`, `Saved/D2`, and `Saved/VFXResearch` remain.
- Shortcut verification: `Reports/Hygiene/2026-05-27/shortcut_verification_gatec.json`; both `T66 Standalone.lnk` targets point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged smoke capture: `Reports/Hygiene/2026-05-27/staged_main_menu_smoke.png` captured from the packaged staged exe.

Deferred cleanup notes:

- `Saved/D2` appears to contain material/VFX experiment folders named `D1_PureGraph`, `D2_GraphWorldPosOnly`, `D3_GraphDistance`, `D4_CameraOffset`, `D5_CustomNoPosition`, `D6_CustomInlineNoInclude`, `V1_VertexColorA`, and `V2_VertexColorACustom`; preserved pending explicit confirmation.
- `Saved/VideoCaptures` is not the runtime hero-selection video location, but current VFX/process docs still reference evidence under that tree, so it was preserved rather than treated as 100% irrelevant clutter.
- `Saved/StagedBuildsDemo`, `Saved/StagedBuilds/WindowsHotfix`, and `Saved/StagedBuilds/WindowsTemp` are generated-looking but preserved until a regeneration owner or stale status is proven.
