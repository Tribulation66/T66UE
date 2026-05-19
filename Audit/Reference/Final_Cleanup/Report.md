# T66 Final Cleanup Fix Pass Report

Date: 2026-05-15  
Scope: execute Resolve Now and approved Decide First cleanup items from `Audit/Reference/Final_Cleanup_Backlog/Report.md`.

## Summary

The final cleanup pass removed the explicit archive shelves, retired the old generic skeletal/generic import scripts, pruned dead Retro FX CVar snapshot state, removed duplicate `r.HeterogeneousVolumes` scalability ownership, added tool-only annotations for the Arthur validation row, and staged a fresh standalone build.

One requested deletion was deferred by safety check: `Scripts/ImportStaticMeshes.py` and `Scripts/MakeGLBImportsUnlit.py` are still used by active QuadRetro, CoherentThemeKit01, projectile, arcade, world NPC/interactable, and generated-kit verification scripts. They were kept and tracked in `Scripts/pending_issues_Scripts.md`.

## Changes applied

| Area | Files / paths | Result |
|---|---|---|
| Archive deletion | `Content/Characters/_Archive/`, `Content/Materials/_Archive/`, `Content/World/Terrain/_Archive/` | Deleted all explicitly approved archive contents. Empty `_Archive` parent folders were removed. |
| Legacy folder cleanup | `Content/Characters/_Legacy/` | Deleted after confirming the folder was empty on disk. Preserved `LEGACY_MI_DIR` in `Scripts/QuadRetroCharacterPipelineDefaults.py`. |
| Dead CVar capture state | `Source/T66/Core/T66RetroFXSubsystem.h:203-206`, `Source/T66/Core/T66RetroFXSubsystem.cpp:1873-1883`, `Source/T66/Core/T66RetroFXSubsystem.cpp:1893-1899` | Removed capture fields/calls for project-owned CVars. Kept runtime-owned `r.ScreenPercentage` and `r.ScreenPercentage.MinResolutionFraction`. |
| Heterogeneous volumes CVar | `Config/DefaultScalability.ini:119-137` | Removed the four `r.HeterogeneousVolumes=` scalability assignments. Kept `Config/DefaultEngine.ini:158` as source of truth. |
| Old import scripts | `Scripts/ImportSkeletalMeshes.py`, `Scripts/RunImportSkeletalMeshesAndExit.py`, `Scripts/MakeCharacterMaterialsUnlit.py`, `Scripts/RepairStaticMeshImportBatch.py`, `Scripts/VerifyImportBatch.py`, `Scripts/RunVerifyImportBatchAndExit.py`, `Scripts/RetroactivelyNormalizeCharacterTextures.py`, `Scripts/RepairQuadRetroHeroTexturesAndExit.py`, `Scripts/InspectHeroMeshAssetDetails.py`, `Scripts/InspectSampleHeroMaterials.py` | Deleted. The last two were additional orphaned old Hero_1 Idle/Walk inspection scripts only referenced by the stale scripts pending issue. |
| Script docs | `Scripts/README.md:11-16`, `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md:7-11` | Removed retired skeletal/generic verifier references and documented domain-specific import/verification ownership. |
| Tool-only Arthur row | `Model Generation/Rigging and Animation/02_HERO_ANIMATION_PIPELINE_INSTRUCTIONS.md:163`, `Model Generation/Rigging and Animation/Tools/verify_arthur_quadretro_animation_in_unreal.py:24-25`, `Model Generation/Rigging and Animation/Tools/import_arthur_quadretro_animation_to_unreal.py:40-41` | Marked `Hero_1_Chad_QuadRetroUALQA` as a tool-only validation row. |
| Stacy Beachgoer fallback | `Source/T66/Core/T66CharacterVisualSubsystem.cpp:673-679` | Added inline comment documenting intentional fallback from missing Stacy skin rows to matching Chad skin rows. |
| Pending issue hygiene | `Source/T66/Core/pending_issues_Core.md`, `Scripts/pending_issues_Scripts.md`, `Content/Characters/pending_issues_Characters.md` | Removed resolved Heterogeneous pending section, replaced stale scripts pending issue with the still-valid static import dependency issue, and removed the resolved character audit pending file. |

## Deletion safety checks

| Check | Result |
|---|---|
| Archive asset path references across `Source`, `Content/Data`, `Scripts`, `Config` | Zero active references before deletion. |
| Deleted skeletal/generic script references across active `Scripts`, `Model Generation`, `Gameplay`, `Source`, `Config`, `Content/Data` | Zero active references after deletion. Historical audit reports still reference them as past evidence. |
| Remaining `_Archive` folders | None found at `Content/Characters/_Archive`, `Content/Materials/_Archive`, or `Content/World/Terrain/_Archive`. |
| Remaining `_Legacy` folder | None found at `Content/Characters/_Legacy`. |
| Remaining `r.HeterogeneousVolumes=` assignments | Only `Config/DefaultEngine.ini:158` remains. |
| Removed CVar snapshot fields | No remaining exact fields for `OriginalSecondaryScreenPercentage`, `OriginalScreenPercentageMinResolution`, `OriginalUpscaleQuality`, `OriginalAntiAliasingMethod`, or `OriginalTemporalAAUpsampling`. |

## Deferred deletion

`Scripts/ImportStaticMeshes.py` and `Scripts/MakeGLBImportsUnlit.py` were not deleted. Safety check found active imports from:

- `Scripts/ImportQuadRetroEnemyVisuals.py`
- `Scripts/ImportQuadRetroBossVisuals.py`
- `Scripts/ImportQuadRetroHeroVisuals.py`
- `Scripts/ImportWeaponProjectileMeshesAndSetup.py`
- `Scripts/ImportArcadeReplacementBatch01AndExit.py`
- `Scripts/ImportWorldNpcInteractablesRetroBatch01AndExit.py`
- `Scripts/RunImportCoherentThemeKit01AndExit.py`
- `Scripts/RunImportNewSourceAssetsAndExit.py`
- generated-kit scale/optimization verification scripts

New tracking entry: `Scripts/pending_issues_Scripts.md`.

## Verification

| Verification | Result |
|---|---|
| Python syntax check | `python -m py_compile` passed for `verify_arthur_quadretro_animation_in_unreal.py` and `import_arthur_quadretro_animation_to_unreal.py`. |
| Build/cook/stage | `powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames` completed successfully. |
| Staged executable | Verified at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`. |
| Taskbar shortcut | Verified target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`. |
| Frontend smoke | Exit code `0`; log `Saved/StandaloneLogs/FinalCleanup_Frontend.log`; screenshot `Saved/FinalCleanup/frontend_smoke.png`. |
| Gameplay smoke | Exit code `0`; log `Saved/StandaloneLogs/FinalCleanup_Stage1.log`; screenshot `Saved/FinalCleanup/stage1_smoke.png`. |

## Warning readout

| Log | Total warnings | `r.HeterogeneousVolumes` | `r.Upscale.Quality` | `SetByGameSetting` | Disabled DMI/MPC/pixelation null warnings | Fatal/error markers |
|---|---:|---:|---:|---:|---:|---:|
| `Saved/StandaloneLogs/FinalCleanup_Frontend.log` | 39 | 0 | 2 | 0 | 0 | 0 |
| `Saved/StandaloneLogs/FinalCleanup_Stage1.log` | 36 | 0 | 2 | 0 | 0 | 0 |

Baseline comparison from `Saved/StandaloneLogs/VisualCleanup_Iteration01_Stage1.log`: `r.HeterogeneousVolumes` warnings dropped from `2` to `0`. The two remaining `r.Upscale.Quality` warnings are the known scalability-vs-project-setting behavior retained because `r.Upscale.Quality=1` is the documented nearest-style upscaling baseline from Visual Lock Iteration 02.

## Smoke artifacts

![Frontend smoke](../../../Saved/FinalCleanup/frontend_smoke.png)

![Stage 1 smoke](../../../Saved/FinalCleanup/stage1_smoke.png)

## Pending issues

| File | Topic |
|---|---|
| `Scripts/pending_issues_Scripts.md` | Static mesh import core still owns shared GLB helpers, so `ImportStaticMeshes.py` and `MakeGLBImportsUnlit.py` were deferred instead of deleted. |
| `Source/T66Mini/pending_issues_T66Mini.md` | Stage build warns that `T66Mini.Build.cs` references missing `Source/T66Mini/Public/UI/Components`. Build succeeds; cleanup belongs to the T66Mini module. |

## Notes

- `Content/Characters/_Legacy/MaterialInstances_QuadRetro` assets already appeared deleted in the working tree; this pass left that deletion state intact and removed the empty folder from disk.
- Historical audit/reference markdown still mentions deleted scripts and archive paths as past evidence. Active instructions and active scripts were cleaned.

