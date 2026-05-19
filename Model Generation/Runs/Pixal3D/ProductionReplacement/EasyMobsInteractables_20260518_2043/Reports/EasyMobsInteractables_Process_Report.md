# Pixal3D Easy Mobs and LootCrate Process Report

Run root: `C:\UE\T66\Model Generation\Runs\Pixal3D\ProductionReplacement\EasyMobsInteractables_20260518_2043`

Scope:
- Easy mobs: `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`, `TombSpider`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, `CryptWraith`.
- Interactable: `LootCrate`.
- Stage-active Easy mobs in `Content/Data/Stages.csv` Stage_01 through Stage_04: `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`.
- Easy roster rows not currently assigned to Stage_01 through Stage_04: `TombSpider`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, `CryptWraith`.

## Stage Timing

Times are local workstation time unless noted. Remote Pixal3D generation timestamps are stored in UTC in `Logs/pixal3d_generation_status.jsonl`.

| Stage | Time spent | Evidence |
| --- | ---: | --- |
| Instruction/source rule update and scope mapping | about 25 min | Updated `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md` and `09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`; mapped images to `CharacterVisuals`, `MobVertexAnimations`, `Stages`, and `T66CrateInteractable`. |
| Faithful image regeneration and source gate | 44 min | v01 outputs 19:50-20:06; v02 faithful outputs 20:09-20:34; `source_gate_report_v02.json` and `faithful_fixes_contact_sheet_v02.png` written at 20:34. |
| Manifest finalization, pod health, and source staging | about 9 min | `source_stage_manifest.json` written 20:43; manifest validation passed. |
| Pixal3D remote generation | 43 min 08 sec | Remote job 2026-05-18T23:45:41Z to 2026-05-19T00:28:49Z; 11/11 HTTP 200 GLBs. |
| Download and GLB/settings verification | about 7 min | 11 nonzero GLBs downloaded; every response header reported `X-Pixal3D-Export-Decimation=200000`, `X-Pixal3D-Export-Remesh=1`, `X-Pixal3D-Export-Safe-Fill-Holes=0`. |
| Blender ToonStyle foundation processing | 1 hr 23 min | Processing wrapper start about 21:36; first manifest `Slime` at 21:44:33; final manifest `LootCrate` at 22:59:20. |
| Unreal import and hard validator report | about 9 min | `*_ue_verify.json` files 23:00:38-23:08:58; `Pixal3D_ToonStyle_Production_Import_Report.json` at 23:09:28 with `ok=true`, 11 assets, 0 errors. |
| Runtime data/code wiring | about 12 min, overlapped with Blender | Updated `CharacterVisuals.csv`, disabled the 10 Easy `MobVertexAnimations.csv` rows, changed `T66CrateInteractable.cpp` to `SM_LootCrate`, and updated manifest status metadata to `verified`. |
| Data table reload | 35 sec | `SetupCharacterVisualsDataTable.py` and `SetupMobVertexAnimationsDataTable.py` both completed with 0 errors. |
| Build/cook/stage | about 5 min including retry | First stage compiled/cooked but failed cleaning a locked staged DLL; stopped stale `T66.exe`; second `StageStandaloneBuild.ps1 -ClientConfig Development` succeeded. |
| Shortcut and staged boot verification | about 1 min | Repo and taskbar `T66 Standalone.lnk` targets both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`; staged smoke boot exited 0 with no fatal/errors/asserts/ensures. |

Total wall-clock elapsed from first faithful image output to staged smoke: about 3 hr 27 min. Most of that was automated generation, Blender processing, import, and build/cook time.

## Asset Results

| Asset | Generation | ToonStyle validation | Runtime connection |
| --- | --- | --- | --- |
| Slime | OK, 9,863,824 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/Slime/SM_Slime`; VAT disabled. |
| BoneWalker | OK, 14,039,404 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/BoneWalker/SM_BoneWalker`; VAT disabled. |
| RatPack | OK, 11,521,664 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/RatPack/SM_RatPack`; VAT disabled. |
| CaveBat | OK, 9,360,312 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/CaveBat/SM_CaveBat`; VAT disabled. |
| HexSlinger | OK, 9,175,824 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/HexSlinger/SM_HexSlinger`; VAT disabled. |
| TombSpider | OK, 10,005,500 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/TombSpider/SM_TombSpider`; VAT disabled. |
| StoneSentinel | OK, 10,400,676 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/StoneSentinel/SM_StoneSentinel`; VAT disabled. |
| MimicLure | OK, 11,039,416 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/MimicLure/SM_MimicLure`; VAT disabled. |
| BoneConjurer | OK, 11,123,436 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/BoneConjurer/SM_BoneConjurer`; VAT disabled. |
| CryptWraith | OK, 13,215,324 bytes | OK | `CharacterVisuals` uses `/Game/Characters/Mobs/CryptWraith/SM_CryptWraith`; VAT disabled. |
| LootCrate | OK, 10,846,268 bytes | OK | `T66CrateInteractable.cpp` now loads `/Game/World/Interactables/Crate/SM_LootCrate.SM_LootCrate`. |

## Problems Found

- The first regenerated image batch drifted away from the original style. The source rule now explicitly requires faithful regeneration: keep identity, silhouette, pose, camera, proportions, and line/style character; only technical brightness/color corrections are allowed.
- Some dark designs were in tension with strict luminance requirements. The accepted solution was to prioritize faithful v02 sources and record the gate tradeoff, because pushing brightness harder changed the asset identity.
- The initial Pixal3D local launch failed because the path with `Model Generation` was quoted incorrectly. The later run used the manifest wrapper with fully quoted paths.
- The remote Pixal3D job completed, but local SSH polling timed out while waiting for the status JSON. The solution was to treat the remote status file and downloaded outputs as the source of truth, then verify every GLB and response header locally.
- Python rewrites of the CSV files hit a Windows mapped-section/file-use error. The solution was a temp-file PowerShell `Import-Csv`/`Export-Csv` rewrite, then moving the temp file into place.
- The old Easy mob VAT rows would have overridden `CharacterVisuals`, so the new static meshes would not spawn. Disabling only the ten Easy VAT rows let the existing `AT66EnemyBase` fallback path use the new Pixal3D meshes.
- Unreal import returned exit code 3 after each verify JSON was written. The wrapper treats this as a warning; the hard validator report confirmed all assets were imported and valid.
- The first stage attempt failed because an old taskbar-launched `T66.exe` was still running and locking `dbghelp.dll`. Stopping that stale staged process allowed the second stage pass to complete.
- Runtime mob static meshes under `/Game/Characters/Mobs/` still flow through the existing shared QuadRetro material override path. The new meshes and imported textures are used, and the production import materials/outline meshes validate, but a later runtime rendering pass should decide whether mobs should attach the generated outline mesh/material pair directly instead of the current shared material path.

## Improvements for Next Batch

- Add a source-regeneration checklist that scores faithfulness separately from luminance so brightness fixes cannot silently mutate the object.
- Add a before/after contact sheet gate for every regenerated image before any Pixal3D run starts.
- Make the batch launcher path-quoting hard fail earlier when the run root contains spaces.
- Increase SSH polling timeout or make the remote status file polling resilient to long single-model runs.
- Add an automated pre-stage check that stops or reports staged `T66.exe` processes before `StageStandaloneBuild.ps1` tries to clean the old staging directory.
- Add a validator that checks gameplay override systems, such as `MobVertexAnimations`, before import so data-table shadowing is caught earlier.
- Add a production decision for mob outline usage: either keep the current shared material path intentionally, or extend the runtime visual subsystem to attach the imported ToonStyle outline mesh/material pair for Pixal3D mobs.

## Verification

- `run_pixal3d_toonstyle_production_import.py validate`: manifest valid.
- Pixal3D generation: 11/11 OK, HTTP 200, decimation 200000, remesh enabled, safe-fill fallback not used.
- Blender process: 11/11 manifests written with Tint, close-the-gap B, and inner-line texture outputs.
- Unreal import: 11/11 `*_ue_verify.json` files written.
- Hard validator: `Pixal3D_ToonStyle_Production_Import_Report.json` has `ok=true`, 11 assets, 0 errors.
- Data table reload: `SetupCharacterVisualsDataTable.py` and `SetupMobVertexAnimationsDataTable.py` succeeded with 0 errors.
- Build/stage: `StageStandaloneBuild.ps1 -ClientConfig Development` succeeded after stopping the stale staged game process.
- Cooked asset spot-checks passed for representative new mob meshes, `SM_LootCrate`, `DT_CharacterVisuals`, and `DT_MobVertexAnimations`.
- Shortcut verification: both standalone shortcuts target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged smoke: `T66.exe /Game/Maps/GameplayLevel -nullrhi -nosound -unattended -ExecCmds=Quit` exited 0; smoke log had 0 `Fatal`, 0 `Error:`, 0 `Ensure condition failed`, and 0 `Assertion failed`.
