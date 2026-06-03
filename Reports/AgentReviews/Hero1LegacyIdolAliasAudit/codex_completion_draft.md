Result: OK

Operator: Codex
Validator: Claude
Scope: Non-Mini legacy idol alias save audit and scoped removal before Hero 1 temporary visual-shape work.

Summary:
- The save audit found no legacy idol aliases in active saves under `Saved/SaveGames` or `Saved/StagedBuilds/Windows/T66/Saved/SaveGames`.
- Archived non-Mini staged backups under `Saved/StageBackups` still contain legacy IDs such as `Idol_Light` in `EquippedIdols` and `DamageBySource`, so a total deletion of compatibility aliases is not safe.
- I kept legacy compatibility at the save/backend migration boundary and removed duplicated runtime/localization/VFX fallback branches.

Files changed in this pass:
- `Source/T66/Core/T66SaveMigration.h`
  - Added `T66MigrateLegacyIdolID`.
  - Updated `T66NormalizeEquippedIdolSaveArrays` to canonicalize legacy equipped idol IDs.
- `Source/T66/Core/T66IdolManagerSubsystem.cpp`
  - Routed `NormalizeLegacyIdolID` through `T66MigrateLegacyIdolID`.
  - Changed canonical electricity idol color to purple. Ice remains light blue.
- `Source/T66/Core/T66LocalizationSubsystem.cpp`
  - Removed legacy-specific idol name/tooltip branches.
  - Localization now normalizes the ID and uses canonical category suffix text.
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - Removed legacy imported-VFX fallback branches and the explicit `Idol_Storm` BP fallback.
  - Canonical normalized IDs still route to imported temporary VFX paths.
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - Removed the disabled legacy-id status-effect branch and replaced it with a no-op local lambda.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Added save proof checks for direct legacy equipped-idol migration, backend JSON parse migration, and localization migration.
- `Reports/AgentReviews/Hero1LegacyIdolAliasAudit/codex_audit_and_change_summary.md`
  - Captures the audit result, allowed remaining aliases, and proof paths.

Allowed remaining legacy strings:
- `Source/T66/Core/T66SaveMigration.h`: migration table.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`: proof inputs only.
- `Source/T66/Gameplay/T66CombatComponent.cpp` and `Source/T66/Gameplay/T66CombatVFX.cpp`: historical `Idol_Water` proof/diagnostic labels.
- `Content/Data/Idols.csv`: sprite asset paths, not idol IDs.
- Existing docs/review packets.

Verification:
- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development`
  - First attempt failed before compile due to `ConflictingInstance` UnrealBuildTool mutex.
  - Process audit showed no live UBT/dotnet/AutomationTool/editor/game process except the tray app.
  - Second attempt succeeded with `BUILD SUCCESSFUL`.
  - Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Root and taskbar shortcuts were refreshed to that staged executable.
- Staged save proof:
  - Ran staged `T66.exe` with `-T66Entry=Run:Tower -T66GameplayAutoCapture=runsummaryroundtrip -T66RunSummaryRoundTripProof=C:\UE\T66\Saved\Automation\run_summary_roundtrip_legacy_idols_20260603.json`.
  - Exit code `0`.
  - Manifest `C:\UE\T66\Saved\Automation\run_summary_roundtrip_legacy_idols_20260603.json` has `ok=true`.
  - New checks passed:
    - `Legacy idol save IDs migrate to canonical IDs`: `Idol_Light, Idol_Water, Idol_Storm, Idol_Poison` -> `Idol_Electricity_Pierce, Idol_Ice_AOE, Idol_Electricity_AOE, Idol_Nature_DOT`.
    - `Legacy idol backend IDs parse as canonical IDs`: same canonical result through `T66BackendRunSummaryParser`.
    - `Canonical idol localization resolves`: canonical fire AOE and legacy water display names resolve.
    - `Legacy idol tooltip resolves through migration`: legacy water tooltip resolves to canonical AOE tooltip.
  - Log marker: `FPlatformMisc::RequestExitWithStatus(0, 0, T66RunSummaryRoundTripComplete)`.
- VFX sanity proof:
  - Ran `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1 -OutputRoot Saved/VideoCaptures/Hero1LegacyAliasAudit_VFXProof_20260603_Full -TimeoutSeconds 240`.
  - Captures completed, but summary remained partial/red with existing category-native failures.
  - Summary path: `C:\UE\T66\Saved\VideoCaptures\Hero1LegacyAliasAudit_VFXProof_20260603_Full\Hero1AxeIdolCategoryNativeImpactProofSummary.md`.

Caveats:
- I did not rename or delete `Content/Data/Idols.csv` sprite asset paths with old idol words because those are current icon asset references.
- I did not clean historical docs/proof labels because the requested deletion target was runtime alias support, not proof vocabulary/doc history.
- I do not treat the VFX sanity proof as green; it should be addressed during the next Hero 1 temporary-shape/VFX phase.
