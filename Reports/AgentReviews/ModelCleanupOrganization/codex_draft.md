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
