# Hygiene Cleanup Completion Report

Date: 2026-05-27

## Completed Gates

- Gate 0: created `Reports` routing structure, hygiene manifest, and reusable audit scripts.
- Gate B: removed the approved HillTile material/texture chain after AssetRegistry, source-reference, and load-integrity proof.
- Gate A: removed old Quick Revive vending/state wrappers, dead movement hooks, legacy world/tower helper surfaces, old Quick Revive vending/icon assets, and stale report placement under `ToonStyle/Reports`.
- Gate C: deleted only `Saved/Cooked` as 100% generated cook output after Claude rejected broader staged-build cleanup without regeneration proof.

## Confirmation Points

These are complete in the repo, but should be confirmed by the user on wake because they are more nuanced than the original shorthand request.

- Movement cleanup was a live-callsite no-op cleanup, not a pure "already unreferenced symbol" deletion. `GetMovementSpeedSecondaryMultiplier()` returned literal `1.f`, the movement formula lost only that neutral factor, and the legacy Run Summary `Move Speed Mult` `1.0x` row was intentionally removed.
- The Run Summary row removal is a separate user-visible cleanup point: the legacy `Move Speed Mult` row is gone rather than retained as a `1.0x` display.
- Report retention was implemented conservatively: raw report/proof run folders expire after 15 days, but deletion requires a durable summary outside the raw folder and no active references into that run. If the user wants literal unconditional deletion after 15 days, the policy docs should be adjusted.
- `Reports/Hygiene/2026-05-27/hygiene_change_scope.md` is the authoritative per-pass change/delete boundary. `final_diff_scope.txt` is only a mixed broad inventory that includes unrelated pre-existing dirty files.
- No commit was created; the working tree remains dirty with this pass plus unrelated pre-existing changes.

## Deleted Or Removed

- Old Quick Revive native vending actor:
  - `Source/T66/Gameplay/T66QuickReviveVendingMachine.h`
  - `Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp`
- Old Quick Revive vending/icon assets listed in `gatea_delete_backup_manifest.json`.
- Obsolete importer that only recreated the old icon: `Scripts/ImportInteractableUISprites.py`.
- Dead movement hooks and legacy movement naming surfaces:
  - `GetMovementSpeedSecondaryMultiplier`
  - `GetHeroMoveSpeedMultiplier`
  - `DefaultWalkSpeed`
- Legacy tower/world code surfaces:
  - boss beacon helpers and fields
  - idol dummy target spawn path
  - start-area scatter helper
  - old Trickster-in-tower helper
  - model showcase helper
  - stale teleporter-pad naming/comment surface
- HillTile assets:
  - `Content/World/Cliffs/MI_HillTile1.uasset`
  - `Content/World/Cliffs/MI_HillTile2.uasset`
  - `Content/World/Cliffs/MI_HillTile3.uasset`
  - `Content/World/Cliffs/MI_HillTile4.uasset`
  - `Content/World/Cliffs/T_HillTile1.uasset`
  - `Content/World/Cliffs/T_HillTile2.uasset`
  - `Content/World/Cliffs/T_HillTile3.uasset`
  - `Content/World/Cliffs/T_HillTile4.uasset`
- Generated clutter:
  - `Saved/Cooked` only.

## Preserved

- Active Backrooms Quick Revive item and behavior:
  - `Content/Items/Sprites/Item_BackroomsQuickRevive.uasset`
  - `/Game/Items/Sprites/Item_BackroomsQuickRevive.Item_BackroomsQuickRevive`
  - Backrooms reward, inventory consume, HUD icon, and lethal-save behavior.
- Backrooms door/floor/wall textures:
  - `T_Backrooms_Door`
  - `T_Backrooms_Floor`
  - `T_Backrooms_Wall`
- Mini/minigame surfaces.
- Pixal/future-feature surfaces documented in `possible_future_features.md`.
- `Saved/VideoCaptures`, because current VFX/process docs still reference evidence there.
- `Saved/D2`, because it appears to hold material/VFX experiment output and the user asked what it was rather than confirming deletion.
- `Saved/VFXResearch`, per user instruction.
- `Saved/StagedBuilds` overall, with the active `Windows` staged build refreshed and verified.
- `Saved/StagedBuildsDemo`, `WindowsHotfix`, and `WindowsTemp`, because the standard standalone stage only proves regeneration for the active `Windows` staged build.

## Reports Organization

- Added root `Reports/` with `Reports/README.md` and `Reports/AGENTS.md`.
- Added hygiene run folder: `Reports/Hygiene/2026-05-27`.
- Moved tracked ToonStyle reports from `ToonStyle/Reports` to `Reports/ToonStyle`.
- Updated root `AGENTS.md` with report routing and 15-day whole-run cleanup rules.

Retention note:

- The user requested whole run folders be deleted after 15 days. The implemented rule keeps that 15-day threshold, but adds guardrails: delete whole raw report/proof run folders only after confirming a durable summary exists outside the raw folder and no active references still point at the run.

## Verification

- Claude reviews:
  - Gate 0 completion approved: `Saved/AgentReviews/20260527T_hygiene_gate0_completion/20260527T092756-pass3/claude_review_pass3.md`
  - Gate B approved: `Saved/AgentReviews/20260527T_hygiene_gateb_hilltile/20260527T101835-pass6/claude_review_pass6.md`
  - Gate A approved: `Saved/AgentReviews/20260527T_hygiene_gatea_cleanup/20260527T105513-pass5/claude_review_pass5.md`
  - Gate C revised then approved: `Saved/AgentReviews/20260527T_hygiene_gatec_generated_output/20260527T115426-pass2/claude_review_pass2.md`
- Focused editor compile passed:
  - `Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE`
- Full staged standalone refresh passed:
  - `Scripts\StageStandaloneBuild.ps1`
- Staged shortcut verification passed:
  - `Reports/Hygiene/2026-05-27/shortcut_verification_gatec.json`
- Staged packaged smoke capture passed:
  - `Reports/Hygiene/2026-05-27/staged_main_menu_smoke.png`
- Backrooms runtime QA passed from the packaged staged exe:
  - `Reports/Hygiene/2026-05-27/backrooms_consume_qa_verification.json`
  - `Reports/Hygiene/2026-05-27/backrooms_consume_qa_log_excerpt.txt`
  - Evidence: reward granted, HUD item visible when owned, lethal damage consumed the item, hero remained alive with HP after consume.
  - Expected post-revive HP baseline: lethal damage sets HP to the first heart capacity; default tier-0 heart capacity is 20 HP (`HPPerRedHeart`), matching the observed `HPAfter=20.0`.
- Hero movement runtime QA passed from the packaged staged exe:
  - `Reports/Hygiene/2026-05-27/hero_movement_qa_verification.json`
  - `Reports/Hygiene/2026-05-27/hero_movement_qa_log_excerpt.txt`
  - `Reports/Hygiene/2026-05-27/hero_movement_qa_staged_frames`
  - Evidence: 90 Unreal screenshot frames, 15 movement samples, stable `MaxWalkSpeed=840.0`, max velocity `840.0`, max delta speed `886.2`, forward displacement `5406.0`, and a jump Z excursion.
- Run Summary smoke/equivalence proof passed:
  - `Reports/Hygiene/2026-05-27/run_summary_smoke.png`
  - `Reports/Hygiene/2026-05-27/run_summary_smoke_dump.json`
  - `Reports/Hygiene/2026-05-27/run_summary_smoke_verification.json`
  - `Reports/Hygiene/2026-05-27/movement_run_summary_equivalence.md`
  - Evidence: staged direct `RunSummary` screen rendered and dumped, the old `Move Speed Mult` label is absent, and the removed movement multiplier was a literal `1.f` no-op in the movement formula.
- Backrooms texture preservation passed:
  - `Reports/Hygiene/2026-05-27/preservation_and_policy_verification.json` confirms `T_Backrooms_Door`, `T_Backrooms_Floor`, and `T_Backrooms_Wall` still exist.
- Report retention and ToonStyle routing policy were documented:
  - `AGENTS.md`
  - `Reports/README.md`
  - `Reports/AGENTS.md`
- `Reports/ToonStyle` layout was verified:
  - `Reports/Hygiene/2026-05-27/toonstyle_report_layout_verification.json`, 27 files under `Reports/ToonStyle`, and `ToonStyle/Reports` absent.
- World load integrity passed:
  - `Reports/Hygiene/2026-05-27/world_asset_load_integrity_gatea_post.json`, 2 maps, 488 world assets, 0 failures.
- Old-symbol sweep passed over `Source`, `Config`, `Content/Data`, `Gameplay`, and `Scripts`, including Mini paths.
- Path existence proof:
  - old vending/icon assets absent
  - active `Item_BackroomsQuickRevive.uasset` present
  - `Saved/Cooked` absent
  - staged exe present
- Final evidence existence check:
  - `Reports/Hygiene/2026-05-27/final_evidence_existence_verification.json` confirms the key pre/post audit files exist, the 12 Quick Revive deleted-asset backups all exist, Backrooms QA values match the report, Hero Movement QA passed, and Run Summary smoke/equivalence artifacts exist.

Runtime scope note:

- I used the existing `-T66BackroomsAutoQA=Consume` runtime route rather than adding new automation for this cleanup pass.

Dirty worktree boundary:

- This repo already had unrelated dirty files when the pass started. I preserved them rather than reverting them.
- `Reports/Hygiene/2026-05-27/hygiene_change_scope.md` is the clean per-pass scope boundary for files/assets changed or deleted by this hygiene pass.
- Gate A pre-dirty overlap was checked in `Reports/Hygiene/2026-05-27/gatea_predirty_allowlist_check.json`; all hunks in the pre-dirty overlap files matched the approved hygiene tokens.
- `Reports/Hygiene/2026-05-27/final_diff_scope.txt` is a broad targeted diff inventory and includes unrelated pre-existing dirty files; it should not be read as "all changes made by this pass."

Pre-delete evidence interpretation:

- The deleted movement hooks and old world/tower helpers were not all zero-reference symbols before deletion; several had source callsites.
- The pre-delete proof is that `native_reference_audit_gatea_pre.json`, `class_property_audit_gatea_pre.json`, and `gatea_world_symbol_consumer_sweep_pre.txt` showed no serialized asset, Blueprint, map actor, or binary content consumers that would survive after the approved source callsites were removed or rewritten.
- The movement hook work was a live-callsite no-op cleanup, not a pure "already unreferenced symbol" deletion. The removed movement multiplier returned literal `1.f`; movement runtime QA and the Run Summary smoke/equivalence note cover the behavior/display risk.

VFX distinction:

- `Saved/VFXResearch` was preserved.
- Gate A deleted only the approved in-code idol dummy/VFX test target spawner surfaces (`SpawnIdolVFXTestTargets`, `bSpawnIdolVFXTestTargetsAtStageStart`, `IdolVFXTestTargets`) and preserved Pixal test display/gallery and VFX research material.

Recoverability:

- Old Quick Revive vending/icon assets are backed up under `Saved/HygieneBackups/2026-05-27/GateA/DeletedAssets`, with manifest `Reports/Hygiene/2026-05-27/gatea_delete_backup_manifest.json`.
- HillTile assets were tracked files deleted after referencer proof and remain recoverable from Git history unless the user later asks to purge history.
- `Saved/Cooked` was generated output and was deleted after staged standalone refresh passed.
- `Saved/Cooked` can be regenerated through the normal Unreal cook/stage flow, including `Scripts\StageStandaloneBuild.ps1` for the active standalone staged build.

## Remaining Deferred Cleanup

- `Saved/D2`: likely material/VFX experiment output. Top-level children are `D1_PureGraph`, `D2_GraphWorldPosOnly`, `D3_GraphDistance`, `D4_CameraOffset`, `D5_CustomNoPosition`, `D6_CustomInlineNoInclude`, `V1_VertexColorA`, and `V2_VertexColorACustom`.
- `Saved/VideoCaptures`: not runtime hero-selection video storage, but current VFX evidence references still point there.
- `Saved/StagedBuildsDemo`, `WindowsHotfix`, and `WindowsTemp`: generated-looking, but not deleted without a proven regeneration owner.
