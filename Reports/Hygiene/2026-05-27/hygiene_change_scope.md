# Hygiene Change Scope

This file is the per-pass scope boundary for the 2026-05-27 hygiene cleanup. The repo had unrelated dirty worktree files before and during this pass; those unrelated files were left in place and should not be attributed to this cleanup.

## Changed By This Pass

Process and report routing:

- `AGENTS.md`
- `Reports/README.md`
- `Reports/AGENTS.md`
- `Reports/Hygiene/2026-05-27/*`
- `Reports/ToonStyle/**`
- `ToonStyle/Reports/**` moved to `Reports/ToonStyle/**`

Reusable hygiene scripts:

- `Scripts/AuditAssetReferencesAndExit.py`
- `Scripts/AuditClassPropertyDefaultsAndExit.py`
- `Scripts/AuditCliffSideMaterialsAndExit.py`
- `Scripts/AuditNativeClassReferencesAndExit.py`
- `Scripts/AuditWorldAssetsAndExit.py`
- `Scripts/VerifyWorldAssetLoadIntegrityAndExit.py`
- `Scripts/ImportInteractableUISprites.py` deleted because it only recreated the removed `QuickReviveIcon`.

Source cleanup:

- `Source/T66/Core/T66GameInstance.cpp`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`
- `Source/T66/Core/pending_issues_Core.md`
- `Source/T66/Gameplay/Movement/T66HeroMovementTypes.h`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
- `Source/T66/Gameplay/T66HeroBase.h`
- `Source/T66/Gameplay/T66QuickReviveVendingMachine.h` deleted.
- `Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp` deleted.
- `Source/T66/Gameplay/T66GameMode.h`
- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/Gameplay/T66TerrainThemeAssets.cpp`
- `Source/T66/Gameplay/T66TowerMapTerrain.h`
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`
- `Source/T66/UI/T66GameplayHUDWidget.h`
- `Source/T66/UI/HUD/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp`
- `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp`
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`

Docs:

- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Movement/MASTER_MOVEMENT.md`
- `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md`
- `Gameplay/Stats/MASTER_STATS.md`
- `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`
- `Content/World/pending_issues_World.md`

Deleted assets:

- HillTile chain:
  - `Content/World/Cliffs/MI_HillTile1.uasset`
  - `Content/World/Cliffs/MI_HillTile2.uasset`
  - `Content/World/Cliffs/MI_HillTile3.uasset`
  - `Content/World/Cliffs/MI_HillTile4.uasset`
  - `Content/World/Cliffs/T_HillTile1.uasset`
  - `Content/World/Cliffs/T_HillTile2.uasset`
  - `Content/World/Cliffs/T_HillTile3.uasset`
  - `Content/World/Cliffs/T_HillTile4.uasset`
- Old Quick Revive vending/icon chain:
  - `Content/UI/Sprites/Interactables/QuickReviveIcon.uasset`
  - `Content/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D.uasset`
  - `Content/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D_Outline.uasset`
  - `Content/World/Interactables/Vending/QuickReviveVending_QuadRetro.uasset`
  - `Content/World/Interactables/Vending/Materials/MI_QuickReviveVending_Pixal3D.uasset`
  - `Content/World/Interactables/Vending/Materials/MI_QuickReviveVending_Pixal3D_Outline.uasset`
  - `Content/World/Interactables/Vending/Materials/M_QuickReviveVending_QuadRetro.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_0.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_1.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_InnerLines.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_Tint.uasset`
  - `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_QuadRetro_Pixelated_512.uasset`

Generated output deleted:

- `Saved/Cooked`

## Explicitly Not Attributed To This Pass

`Reports/Hygiene/2026-05-27/final_diff_scope.txt` is a broad targeted diff inventory and includes unrelated pre-existing dirty files. The following paths appeared there but are not attributed to this hygiene cleanup:

- `Gameplay/Traps/MASTER_TRAPS.md`
- `Gameplay/World/T66_Tower_Multi_Agent_Implementation_Plan.md`
- `Scripts/README.md`
- `Scripts/pending_issues_Scripts.md`
- `Source/T66/Core/T66LagTrackerSubsystem.cpp`
- `Source/T66/Core/T66TrapSubsystem.cpp`
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h`
- `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp`
- `Source/T66/Gameplay/Enemies/T66RangedEnemy.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp`
- `Source/T66Editor/T66Editor.Build.cs`

## Pre-Delete Evidence Interpretation

The removed movement hooks and world/tower helpers were not all zero-reference symbols before deletion; several had source callsites. The pre-delete proof is narrower and more important:

- `Reports/Hygiene/2026-05-27/native_reference_audit_gatea_pre.json` showed zero AssetRegistry hits, zero Blueprint matches, zero map actor matches, and zero binary content matches for the audited old native/member tokens. Text matches were confined to source/docs/scripts/reports.
- `Reports/Hygiene/2026-05-27/gatea_world_symbol_consumer_sweep_pre.txt` captured the source/docs callsites that were intentionally removed or rewritten as part of the approved Gate A scope.
- `Reports/Hygiene/2026-05-27/class_property_audit_gatea_pre.json` showed no Blueprint CDO matches for the audited GameMode properties; the native CDO still exposed `bSpawnIdolVFXTestTargetsAtStageStart`, which was intentionally removed.

So the claim is not "these symbols had zero source references before deletion." The claim is "there were no serialized asset, Blueprint, map actor, or binary content consumers that would survive a C++ compile after source callsites were removed."

## Recoverability Notes

- Old Quick Revive asset deletions are backed up under `Saved/HygieneBackups/2026-05-27/GateA/DeletedAssets`, with manifest `Reports/Hygiene/2026-05-27/gatea_delete_backup_manifest.json`.
- HillTile assets were tracked files deleted after Gate B referencer proof. They remain recoverable from Git history unless the user later asks to permanently purge them from history.
- `Saved/Cooked` is generated output and was deleted only after staged standalone refresh succeeded.

