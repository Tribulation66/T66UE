# Hero 1 Legacy Idol Alias Save Audit

Date: 2026-06-03

## Scope

- Non-Mini audit of legacy idol aliases before the Hero 1 temporary weapon/idol visual-shape work.
- Keep compatibility only where old persisted save/backend data needs migration.
- Do not rename sprite asset paths in `Content/Data/Idols.csv`; those are current icon assets, not idol row IDs.

## Audit Result

- Active saves in `Saved/SaveGames` and `Saved/StagedBuilds/Windows/T66/Saved/SaveGames` had zero legacy idol alias hits.
- Archived non-Mini staged backups under `Saved/StageBackups` still contain legacy IDs such as `Idol_Light` in `EquippedIdols` and `DamageBySource`.
- Therefore, deleting every alias outright is not safe. The safe boundary is a save/backend migration table plus canonical runtime IDs everywhere else.

## Changes Made

- Centralized legacy idol ID migration in `Source/T66/Core/T66SaveMigration.h`.
- Made `T66NormalizeEquippedIdolSaveArrays` canonicalize legacy equipped idol IDs while normalizing slot count and tiers.
- Routed `UT66IdolManagerSubsystem::NormalizeLegacyIdolID` through the save migration helper.
- Removed legacy-specific localization name/tooltip branches; localization now normalizes and uses canonical category suffixes.
- Removed legacy imported-VFX fallback branches; imported VFX routing now depends on normalized canonical IDs.
- Removed the disabled legacy idol-specific combat status branch and replaced the call target with a no-op lambda.
- Added explicit run-summary proof checks for direct save-array migration and backend JSON parse migration.
- Added localization proof checks for canonical idol text and legacy-id migration into canonical tooltip text.
- Changed electricity idol color to purple for the accepted Hero 1 temporary-visual plan; ice is already light blue.

## Allowed Remaining Legacy Strings

- `Source/T66/Core/T66SaveMigration.h`: the migration table.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`: proof inputs for the migration checks.
- `Source/T66/Gameplay/T66CombatComponent.cpp` and `Source/T66/Gameplay/T66CombatVFX.cpp`: historical `Idol_Water` diagnostic/proof labels.
- `Content/Data/Idols.csv`: current sprite asset paths named with old idol words.
- Existing docs and prior review packets.

## Verification

- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`
  - First attempt hit transient UnrealBuildTool mutex contention.
  - Second attempt succeeded; UBT build, cook, stage, package completed with `BUILD SUCCESSFUL`.
  - Staged executable refreshed at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Root and taskbar standalone shortcuts were refreshed to that executable.
- Staged save proof:
  - Command: staged `T66.exe` with `-T66Entry=Run:Tower -T66GameplayAutoCapture=runsummaryroundtrip`.
  - Manifest: `C:\UE\T66\Saved\Automation\run_summary_roundtrip_legacy_idols_20260603.json`.
  - Log: `C:\UE\T66\Saved\Automation\run_summary_roundtrip_legacy_idols_20260603.log`.
  - Result: `ok=true`.
  - New checks passed:
    - `Legacy idol save IDs migrate to canonical IDs`.
    - `Legacy idol backend IDs parse as canonical IDs`.
    - `Canonical idol localization resolves`.
    - `Legacy idol tooltip resolves through migration`.
- VFX sanity proof attempted:
  - Command: `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1 -OutputRoot Saved/VideoCaptures/Hero1LegacyAliasAudit_VFXProof_20260603_Full -TimeoutSeconds 240`.
  - Summary: `C:\UE\T66\Saved\VideoCaptures\Hero1LegacyAliasAudit_VFXProof_20260603_Full\Hero1AxeIdolCategoryNativeImpactProofSummary.md`.
  - Result: partial/red. Captures completed, but existing Hero 1 category-native gates still report failures for several idol categories. This is not accepted as green VFX proof and should be handled in the next VFX-shape phase, not treated as save-alias proof.

## Closeout Boundary

- Safe to move on from runtime legacy alias support: runtime/localization/VFX fallback branches for the old idol IDs were removed or routed through canonical normalization.
- Not safe to delete the migration table: archived staged save backups still contain legacy IDs, and migration-only compatibility is still required for old persisted/backend payloads.
