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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\ModelCleanupOrganization\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\ModelCleanupOrganization\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\ModelCleanupOrganization\ClaudeReview\20260605T043802-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

Ok now I want to have better cleanup and orgnaization for my models, what do you and claude suggest we do about models that are no longer used, and not just in theory but the models we have int he game right now that are not used what should be done?

# Task Contract

Working task:
Operator: Codex
Validator: Claude if the local helper is available and billing preflight is clean
Scope: inspect the live T66 model/import state, identify currently unused model assets or unused imported model groups, and produce a concrete cleanup/organization recommendation without deleting or changing assets.
Stop condition: answer gives repo-grounded categories, specific examples/paths, recommended action per category, caveats, and Claude/Codex token reporting.

# Relevant Repo Rules

- This is a read-only recommendation task. Do not delete or move assets.
- Start from live repo state, current model-generation instructions, current Unreal data references, and current audit scripts.
- For generated model cleanup, route through `Model Generation/MODEL_GENERATION_AGENTS.md`, `Model Generation/Instructions/README.md`, `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`, and relevant import/cleanup instructions.
- Model-generation generated runs are cleanup targets after imported assets are verified or rejected; runtime assets require Unreal package referencer and text/data reference proof before deletion.
- Do not use broad Git/LFS scans over `Content/`.
- Report concrete candidate categories and what should be done with each.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The repo already encodes the right model: a **two-tier cleanup policy** with existing tooling. The recommendation is to apply that policy systematically rather than invent anything new. Group the unused models into the categories below and act per category.

**Tier 1 — Generated model-generation output (delete-after-verify; not runtime).**
Per `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md` (Folder Rules / Script Lifecycle) and `05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md` ("Generated Output Retention"), the following are explicit cleanup targets once their imported assets are verified or rejected:
- Raw TRELLIS / Blender / scene / render / log output under `Model Generation/Runs/`, `Scenes`, `Archive`, `Reference`, and local-access files.
- Named one-off batch drivers under `Model Generation/Scripts/Batches/...` (e.g. `WorldNpcInteractablesRetroBatch01/`, `EnemyBosses/EnemyBossBatch01/`, `Weapons/AutoAttackProjectileBatch01/`) — "delete after the batch is done."
- Already-archived runs (e.g. `Runs/Pixal3D/Archive/...`) whose imports are confirmed in-game.
- **Action:** confirm each run's imported assets are verified/rejected, then delete the run/batch artifacts. Keep only lightweight manifests/status summaries still needed to drive an import, and migrate any durable lesson into a core script / README / instruction doc before deleting.

**Tier 2 — Runtime `Content/` assets (delete only behind referencer + reference proof).**
Repo rule (prompt line 18) requires Unreal package-referencer evidence **plus** text/data reference proof before any runtime asset is deleted. The tooling already exists:
- `Scripts/AuditCharacterModelDataAndExit.py` → produces `Saved/Audits/CharacterModelDataAudit.json` with `unused_hero_folder_candidates`.
- `Scripts/CleanupUnusedHeroCharacterAssetsAndExit.py` → deletes a hero folder only when it has no hero row, no character-visual rows, zero character-data references, and zero outside package referencers.
- `Scripts/AuditAssetReferencesAndExit.py` → for arbitrary packages, combines AssetRegistry referencers + scoped text scan + `.uasset/.umap` binary token scan.
- Companion audits: `AuditWorldAssetsAndExit.py`, `AuditCharacterModelDataAndExit.py`, `AuditNativeClassReferencesAndExit.py`.
- **Action:** run the relevant audit for each asset family (characters, world/static, weapons), review the candidate lists, and only then delete via the gated cleanup script. The git status already shows deleted legacy meshes (e.g. `SM_LoanShark_Pixal3D.uasset`, `SM_CowardiceGate.uasset`, `BossGate_Pixal3D.uasset`) being superseded by `_Pixal3D` variants — that pattern (old base mesh orphaned after a Pixal3D replacement import) is the prime Tier-2 candidate set to audit.

**Organization (separate from deletion).**
- Keep generated source strictly out of `Content/` runtime paths; runtime imports live under `/Game/...` only.
- Standardize the `_Pixal3D` suffix convention so superseded originals are easy to detect.
- For DataTable-driven assets, the audit JSON's reference counts are the source of truth, not filename matching.

**What NOT to do:** no broad Git/LFS scans over `Content/` (prompt rule); no `-SkipCook` if a deletion ends up affecting the playable standalone build; nothing deleted on filename match alone.

## Evidence Checked
- `Reports/AgentReviews/ModelCleanupOrganization/original_prompt.md`
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`, `MODEL_GENERATION_AGENTS.md`, `05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`
- `Scripts/AuditAssetReferencesAndExit.py`, `Scripts/CleanupUnusedHeroCharacterAssetsAndExit.py`; presence of `AuditWorldAssetsAndExit.py`, `AuditCharacterModelDataAndExit.py`, `AuditNativeClassReferencesAndExit.py`
- `Model Generation/Scripts/Batches/...` one-off batch drivers; `Runs/Pixal3D/Archive/...`
- Working-tree git status showing deleted legacy `SM_*`/gate meshes superseded by `_Pixal3D` variants

## Questions Or Blockers
None requiring the user. The audits are read-only and Codex/the editor can run them to produce the concrete candidate lists. No user-only decision, approval, or scope change is needed to deliver this recommendation.

## Caveats
- I did not launch the Unreal editor or execute the audit scripts, so I have **not** produced the concrete enumerated list of currently-unused runtime packages — that list comes from running the audits. The deleted meshes in git status are inferred candidates, not audit-confirmed.
- Codex's draft should include the actual audit output (or a clear note that the audits must be run to populate the candidate list) plus the required Claude/Codex token reporting, which the stop condition demands.
- Tier-2 deletions must each show both referencer and text/data proof; do not let any "looks unused" example bypass the gate.

</validator_independent_answer>

<codex_draft>
# Codex Draft - Model Cleanup Organization Recommendation

## Working Recommendation

Use a two-lane cleanup policy:

1. Generated model-generation output is not runtime content. Delete or archive it after the imported asset is verified/rejected and any durable lesson is moved into a process doc or reusable script. This applies to `Model Generation/Runs/Pixal3D/*`, archived runs, old smoke/guideline test runs, and old one-off batch folders under `Model Generation/Scripts/Batches`.
2. Runtime `Content/` packages require proof before deletion. Do not delete by filename. Run Unreal package referencer checks plus broad text/data/package token checks, then delete with an Unreal cleanup script so redirectors and dependent assets are handled correctly.

## Live Audit Evidence

Current read-only commandlet audits were run on 2026-06-05:

- `Scripts/AuditCharacterModelDataAndExit.py` via `UnrealEditor-Cmd.exe -run=pythonscript`, exit 0.
- `Scripts/AuditWorldAssetsAndExit.py` via `UnrealEditor-Cmd.exe -run=pythonscript`, exit 0.

Character audit output: `Saved/Audits/CharacterModelDataAudit.json`.

- 660 character assets scanned.
- 12 hero rows, 136 CharacterVisual rows.
- 0 missing character data refs.
- 0 redirectors.
- 0 whole unused hero folder candidates.
- 143 individual zero-reference character assets, spread across all 12 hero folders.
- 7 Stacy visual rows are placeholder/shared/blank candidates, but this is a data cleanup subject, not a model deletion by itself.

World audit output: `Saved/Audits/WorldAssetAudit.json`.

- 471 world assets scanned.
- 99 orphan candidates from the first-pass world audit.
- 50 orphan static meshes, 26 material instances, 23 textures.
- Static mesh candidates group as: Interactables 17, Boosts 16, LootBags 8, Gates 5, VisualProps 4.

Important caveat: `AuditWorldAssetsAndExit.py` text-scans only `Source`, `Config`, and `Content/Data`. It is useful as a first-pass candidate list, not deletion clearance. Exact deletion gates should run `Scripts/AuditAssetReferencesAndExit.py` for each package group because that helper scans wider text roots and binary `.uasset/.umap` token matches.

## Concrete Current Candidates And Actions

Hero content:

- Do not delete whole `/Game/Characters/Heroes/Hero_*` folders. The fresh audit found zero whole-folder candidates.
- Start with Hero 1 old processed variants because Hero 1 now uses raw FriendSlop:
  - `/Game/Characters/Heroes/Hero_1/Chad/AnimatedToonStyle/*`
  - `/Game/Characters/Heroes/Hero_1/Chad/DemoSkin/AnimatedToonStyle/*`
  - `/Game/Characters/Heroes/Hero_1/Chad/Pixal3DToonStyle/*`
  - `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/*`
  - related Beachgoer/Stacy processed variants under `Hero_1`
- Treat similar old Beachgoer/QuadRetro/loose rig assets under Heroes 2-12 as second-wave candidates, not immediate bulk delete, because some hero rows still use processed/skeletal visuals.
- Recommended action: create a `HeroProcessedVariantCleanup` manifest from `CharacterModelDataAudit.json`, exact-audit those packages with `AuditAssetReferencesAndExit.py`, then delete only packages still proving zero reference/text/binary hits. Leave active `CharacterVisuals.csv` row assets untouched.

World/interactable content:

- First-pass world candidates include old boost static meshes and outlines under `/Game/World/Boosts`, old gate variants under `/Game/World/Gates`, old QuadRetro/outline meshes for chest/crate/fountain/idol altar/loot wheel/vehicle/weapon altar, and old loot bag color variants.
- Recommended action: split into cleanup packs by owner:
  - `BoostVisualCleanup`
  - `GateVisualCleanup`
  - `InteractableVisualCleanup`
  - `LootBagVisualCleanup`
  - `VisualPropOutlineCleanup`
- For each pack, run exact `AuditAssetReferencesAndExit.py` package proof, compare against current data owners such as `WorldVisualProps.json`, `VehicleInteractables.json`, and runtime class defaults, then delete obsolete chain assets as a group: mesh + paired material instance + paired texture only when all are proven orphaned.

Generated runs:

- Current run folders include `FriendSlopProbe_Hero1Male_20260604_1415`, `FriendSlopEasyBatch_20260604_1532`, two retry folders, `HumanoidGuidelineTest_20260522_100k`, `PipelineSmoke01`, `HeroDemoLineup_20260522_AccuRig`, and `Archive`.
- Recommended action: preserve the latest FriendSlop manifest/report needed for import provenance, then delete failed retry/guideline/smoke/generated output folders that are not needed to re-drive imports. Keep only durable summaries under `Reports/Hygiene/<date>` or process docs.

## What Should Be Done

Next implementation pass should not manually drag assets around in the editor. It should:

1. Write a durable model cleanup manifest under `Reports/Hygiene/2026-06-05/`.
2. Add/extend a reusable cleanup report script that consumes the two audit JSON files and produces package groups.
3. Run exact `AuditAssetReferencesAndExit.py` for the first group, starting with Hero 1 old processed variants.
4. Delete only the packages that pass the exact gate, using Unreal's asset APIs.
5. Reload affected data assets if any source refs changed.
6. Stage standalone only if playable runtime content actually changed.

## Do Not Do

- Do not delete whole hero folders yet.
- Do not delete world candidates from the first-pass world audit alone.
- Do not keep generated run output indefinitely once imports are verified.
- Do not run broad Git/LFS status/diff over `Content/` as the cleanup proof.

</codex_draft>
