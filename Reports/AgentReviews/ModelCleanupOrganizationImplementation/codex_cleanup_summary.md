# Model Cleanup And Organization - Codex Evidence Packet

## Task Contract

Working task:
Operator: Codex
Validator: Claude if the local helper is available and billing preflight is clean
Scope: perform the approved model cleanup, starting with audit-gated unused runtime models and generated model-output cleanup, then add durable cleanup/organization guidance under Model Generation so future model passes use the same approach automatically.
Stop condition: cleanup is completed only for assets/folders proven safe by the repo gates, documentation is updated, affected Unreal assets/data are verified, and any skipped candidates are reported with reasons.

## Repo Rules Applied

- Root router: `AGENTS.md`.
- Operator/Validator protocol: `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Current role state: `.t66/operator-state.json` selects Codex Operator and Claude Validator.
- Model Generation router: `Model Generation/MODEL_GENERATION_AGENTS.md`.
- Model Generation docs read: `Model Generation/README.md`, `Model Generation/Instructions/README.md`, `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`, `05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`, and `11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.
- Pending issue docs read before folder work: `Scripts/pending_issues_Scripts.md`, `Content/World/pending_issues_World.md`.

## Changes Made

### Runtime Model Cleanup Tooling

- Added `Scripts/BuildModelCleanupCandidateManifest.py`.
- Added `Scripts/DeleteModelCleanupAssetsAndExit.py`.
- Added `Scripts/CleanModelGenerationRuns.py`.
- Updated `Scripts/AuditAssetReferencesAndExit.py` with package-list input, package-path token mode, and faster binary scanning.
- Updated `Scripts/EvaluateModelCleanupExactAudit.py` with multi-group evaluation support.

### Durable Guidance

- Added `Model Generation/Instructions/12_MODEL_CLEANUP_AND_ORGANIZATION_INSTRUCTIONS.md`.
- Updated `Model Generation/README.md` with a cleanup and organization section.
- Updated `Model Generation/Instructions/README.md` to route to the new cleanup instructions.

## Runtime Cleanup Performed

The cleanup used repeated audit, exact-reference gate, and Unreal-side deletion waves.

- First gate: `Reports/Hygiene/2026-06-05/gate_all_runtime_candidates.json`
  - Approved 194 packages, skipped 49.
  - Delete report: `Reports/Hygiene/2026-06-05/delete_report_all_runtime_candidates.json`
  - Deleted 194 approved packages.
- Second gate: `Reports/Hygiene/2026-06-05/gate_post_delete_all_runtime_candidates.json`
  - Approved 59 packages, skipped 46.
  - Delete report: `Reports/Hygiene/2026-06-05/delete_report_post_delete_all_runtime_candidates.json`
  - Deleted 59 approved packages.
- Third gate: `Reports/Hygiene/2026-06-05/gate_final_all_runtime_candidates.json`
  - Approved 56 packages, skipped 46.
  - Delete report: `Reports/Hygiene/2026-06-05/delete_report_final_all_runtime_candidates.json`
  - Deleted 56 approved packages.

Total runtime packages deleted: 309.

## Runtime Cleanup Preserved

Closing gate: `Reports/Hygiene/2026-06-05/gate_closing_all_runtime_candidates.json`

- Approved 0 packages.
- Skipped 46 packages.
- Skip reason: all 46 are `external_binary_token_match`.
- Remaining groups:
  - `hero1_processed_variant_candidates`: 4 packages.
  - `other_hero_zero_ref_candidates`: 17 packages.
  - `world_gate_visual_candidates`: 4 packages.
  - `world_interactable_visual_candidates`: 18 packages.
  - `world_lootbag_visual_candidates`: 1 package.
  - `world_visualprop_candidates`: 2 packages.

These were intentionally kept because the closing gate found remaining external binary package-path references.

## Generated Run Cleanup Performed

Dry-run report: `Reports/Hygiene/2026-06-05/generated_run_cleanup_dry_run.json`

Live report: `Reports/Hygiene/2026-06-05/generated_run_cleanup_report.json`

- Durable deleted-run summaries are preserved in:
  - `Reports/Hygiene/2026-06-05/model_cleanup_candidate_manifest.json`, which records each deleted run path, name, last-write timestamp, recommended action, and rationale before deletion.
  - `Reports/Hygiene/2026-06-05/generated_run_cleanup_report.json`, which records the live deletion result for each deleted run.
- Deleted 4 generated run folders:
  - `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532_retry1`
  - `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532_retry2_R1024`
  - `Model Generation/Runs/Pixal3D/HumanoidGuidelineTest_20260522_100k`
  - `Model Generation/Runs/Pixal3D/PipelineSmoke01`
- Skipped 4 keep-review folders:
  - `Model Generation/Runs/Pixal3D/Archive`
  - `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532`
  - `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415`
  - `Model Generation/Runs/Pixal3D/HeroDemoLineup_20260522_AccuRig`

## Verification Performed

- Python compile:
  - `python -m py_compile Scripts\AuditAssetReferencesAndExit.py Scripts\BuildModelCleanupCandidateManifest.py Scripts\EvaluateModelCleanupExactAudit.py Scripts\DeleteModelCleanupAssetsAndExit.py Scripts\CleanModelGenerationRuns.py`
  - Passed.
- Character model data audit:
  - `Scripts\AuditCharacterModelDataAndExit.py`
  - Passed before cleanup and again after cleanup.
  - Report path: `Saved/Audits/CharacterModelDataAudit.json`.
- World asset audit:
  - `Scripts\AuditWorldAssetsAndExit.py`
  - Passed before cleanup and again after cleanup.
  - Report path: `Saved/Audits/WorldAssetAudit.json`.
- Exact reference gates:
  - `Reports/Hygiene/2026-06-05/exact_audit_all_runtime_candidates.json`
  - `Reports/Hygiene/2026-06-05/exact_audit_post_delete_all_runtime_candidates.json`
  - `Reports/Hygiene/2026-06-05/exact_audit_final_all_runtime_candidates.json`
  - `Reports/Hygiene/2026-06-05/exact_audit_closing_all_runtime_candidates.json`
- Staged standalone build:
  - `Scripts\StageStandaloneBuild.ps1`
  - Exit 0, `BUILD SUCCESSFUL`.
  - Staged executable exists: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Root shortcut and taskbar shortcut both target that staged executable and the target exists.

## Caveats

- I did not run an interactive gameplay visual smoke after the cleanup. The verification performed was audit/gate/delete validation plus staged standalone refresh and shortcut target verification.
- There is no `Content/*AGENTS.md`; this was treated as a documentation gap and the root/model-generation instructions were applied.
