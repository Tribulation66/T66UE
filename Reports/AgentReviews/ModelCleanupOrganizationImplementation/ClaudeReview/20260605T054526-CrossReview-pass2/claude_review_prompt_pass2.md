You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\ModelCleanupOrganizationImplementation\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\ModelCleanupOrganizationImplementation\codex_cleanup_summary.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\ModelCleanupOrganizationImplementation\ClaudeReview\20260605T044714-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

Ok go for it do the cleanup and then add somewhere maybe under read.me in the model generation the organization and cleanup approach for future models, so we dont need to manually do this everytime

# Task Contract

Working task:
Operator: Codex
Validator: Claude if the local helper is available and billing preflight is clean
Scope: perform the approved model cleanup, starting with audit-gated unused runtime models and generated model-output cleanup, then add durable cleanup/organization guidance under Model Generation so future model passes use the same approach automatically.
Stop condition: cleanup is completed only for assets/folders proven safe by the repo gates, documentation is updated, affected Unreal assets/data are verified, and any skipped candidates are reported with reasons.

# Rules And Constraints

- Codex is Operator; Claude is Validator.
- Claude is read-only in this pass.
- Runtime `Content/` assets may only be deleted after Unreal package reference proof and text/data/binary reference proof.
- Do not delete whole hero folders unless the current audit marks them as whole-folder candidates.
- Do not use broad Git/LFS scans over `Content/`.
- Generated model-output folders under `Model Generation/Runs`, `Scenes`, `Archive`, `Reference`, and one-off batch folders are cleanup targets only after imported assets are verified or rejected and durable summaries/manifests are preserved.
- Use Unreal asset APIs for runtime package deletion, not raw filesystem deletion.
- Add reusable Model Generation cleanup/organization guidance so future model imports do not require manual rediscovery.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

This prompt is internally actionable by the Operator/Validator pair — the user already approved ("go for it"), so no user-only decision is pending. Codex can execute it within the contract's gates. The correct shape of the answer:

1. **Runtime `Content/` model deletion** — only delete assets that pass *both* gates: Unreal package reference proof (via asset registry / reference viewer, not raw FS) **and** text/data/binary reference proof. Use Unreal asset APIs for the actual delete. Do **not** remove whole hero folders unless the current audit explicitly marks them whole-folder candidates. Every candidate that fails a gate must be listed as *skipped, with the reason*.

2. **Generated model-output cleanup** (`Model Generation/Runs`, `Scenes`, `Archive`, `Reference`, one-off batch folders) — delete only after imported assets are verified/rejected and durable summaries/manifests are preserved. The existing README "Cleanup Policy" (lines 18–24) already states this intent; the durable per-model workflow should be added there so it isn't rediscovered each pass.

3. **Documentation** — the user asked for the org/cleanup approach "under read.me in the model generation." Best home is `Model Generation/README.md` and/or `Instructions/` (the README points to `Instructions/README.md` as canonical cleanup-policy index). Add a concrete, repeatable checklist: gate sequence, which folders are transient vs durable, what manifest/summary must survive, and the Unreal-API delete step.

One gap Codex should close before reporting done: I found **no dedicated audit artifact** for this specific model cleanup in the task folder (only `original_prompt.md` and the review prompt). The scope says "audit-gated." Codex must point to the actual audit that marks safe candidates (or generate one) — deletions without that proof violate the gates. This is a Codex fixable item, not a user blocker.

## Evidence Checked

- `original_prompt.md` — confirms user approval and full gate set.
- `Model Generation/README.md` — existing "Cleanup Policy" and workspace-shape sections; the natural doc target.
- `Instructions/README.md` referenced as canonical cleanup-policy index.
- Glob of `Model Generation/**/*.md` — confirms Runs/Archive/Experiments/Production output folders exist as cleanup targets.
- Task folder contents — no model-cleanup audit artifact present yet.

## Questions Or Blockers

None requiring the user. The cleanup was already approved and the gates are defined; remaining items (locating/producing the gating audit, performing reference proofs) are Operator work.

## Caveats

- Deletions are hard to reverse. The "audit-gated" requirement is load-bearing — if Codex's draft proceeds to delete without citing the per-asset reference proofs and the governing audit, that draft is not safe to ship and should be corrected before answering the user.
- Confirm the doc lands where the user expects (Model Generation README / Instructions), and that it's a *reusable* workflow, not a one-time changelog of this pass.
- I was read-only; I did not verify individual asset reference counts. Those proofs are the Operator's responsibility per the contract.

</validator_independent_answer>

<codex_draft>
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

</codex_draft>
