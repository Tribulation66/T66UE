# Source State Classification

Date: 2026-05-28T11:00:41.0465313-03:00

## Decision

Use the existing isolated measurement source at C:\UE\T66_B11B12_Worktree for Phase 1 and Phase 3. Do not stage or capture from the live C:\UE\T66 dirty tree. This preserves user-owned/runtime-affecting changes and the separate agent's Git work, while still giving this pass a stable, classifiable source surface.

## Live Worktree Classification

The live tree is dirty and includes runtime-affecting changes outside this pass. These are treated as user-owned or prior-pass state, not as edits for this pass:

`	ext
 M AGENTS.md
 M Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md
 M Gameplay/Combat/MASTER_COMBAT.md
 M Gameplay/Combat/pending_issues_Combat.md
 M Gameplay/Movement/MASTER_MOVEMENT.md
 M Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md
 M Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md
 M Gameplay/Stats/MASTER_STATS.md
 M Gameplay/Traps/MASTER_TRAPS.md
 M Gameplay/World/T66_MAP_DESIGN_REFERENCE.md
 M Gameplay/World/T66_Tower_Multi_Agent_Implementation_Plan.md
 M Scripts/AuditWorldAssetsAndExit.py
 M Scripts/CaptureT66GameplayVideo.ps1
 D Scripts/ImportInteractableUISprites.py
 M Scripts/README.md
 M Scripts/SetupWeaponsDataTable.py
 M Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp
 M Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp
 M Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp
 M Source/T66/Core/T66GameInstance.cpp
 M Source/T66/Core/T66GameInstance.h
 M Source/T66/Core/T66LagTrackerSubsystem.cpp
 M Source/T66/Core/T66RunStateSubsystem.h
 M Source/T66/Core/T66TrapSubsystem.cpp
 M Source/T66/Core/pending_issues_Core.md
 M Source/T66/Data/T66DataTypes.h
 M Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h
 M Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp
 M Source/T66/Gameplay/Enemies/T66RangedEnemy.h
 M Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp
 M Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp
 M Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp
 M Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp
 M Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp
 M Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp
 M Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp
 M Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp
 M Source/T66/Gameplay/Movement/T66HeroMovementTypes.h
 M Source/T66/Gameplay/T66BossBase.cpp
 M Source/T66/Gameplay/T66BossGate.cpp
 M Source/T66/Gameplay/T66BossGate.h
 M Source/T66/Gameplay/T66BossProjectile.h
 M Source/T66/Gameplay/T66CombatComponent.cpp
 M Source/T66/Gameplay/T66CombatComponent.h
 M Source/T66/Gameplay/T66CombatShared.cpp
 M Source/T66/Gameplay/T66CombatShared.h
 M Source/T66/Gameplay/T66CombatVFX.cpp
 M Source/T66/Gameplay/T66EnemyBase.cpp
 M Source/T66/Gameplay/T66EnemyDirector.cpp
 M Source/T66/Gameplay/T66GameMode.cpp
 M Source/T66/Gameplay/T66GameMode.h
 M Source/T66/Gameplay/T66HeroBase.h
 M Source/T66/Gameplay/T66PlayerController_Overlays.cpp
 D Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp
 D Source/T66/Gameplay/T66QuickReviveVendingMachine.h
 M Source/T66/Gameplay/T66TerrainThemeAssets.cpp
 M Source/T66/Gameplay/T66TowerDescentHole.h
 M Source/T66/Gameplay/T66TowerMapTerrain.cpp
 M Source/T66/Gameplay/T66TowerMapTerrain.h
 M Source/T66/Gameplay/T66TutorialManager.cpp
 M Source/T66/Gameplay/pending_issues_Gameplay.md
 M Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp
 M Source/T66/UI/HUD/T66GameplayHUDWidget.cpp
 M Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp
 M Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp
 M Source/T66/UI/Screens/T66RunSummaryScreen.cpp
 M Source/T66/UI/T66CollectorOverlayWidget.cpp
 M Source/T66/UI/T66GameplayHUDWidget.h
 M Source/T66/UI/T66ItemCardTextUtils.cpp
 M Source/T66Editor/T66Editor.Build.cs
 M T66.uproject
`

Per-path decision summary:

- AGENTS.md: intended process-rule change already reviewed and applied; not part of staged Unreal measurement.
- Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/*: intended review/report artifacts for this pass; not part of staged Unreal measurement.
- Enemy infrastructure paths (Source/T66/Gameplay/T66EnemyDirector.cpp, T66MobBase.*, T66MobManagerSubsystem.*, T66ProjectileManagerSubsystem.*, boss/miniboss related files): prior baseline/current workstream candidates; handled in isolated source state.
- Combat VFX, weapon data, RunState, movement, terrain/world, UI/HUD, scripts, Config, and deleted Content/QuickRevive assets shown above: user-owned or prior unrelated runtime state; not reverted, not cleaned, not used as live staging source by this pass.

## Process Snapshot

Plain git.exe processes were observed and are attributed to the separate user-requested Git agent. No git-lfs.exe, RunUAT, UnrealEditor-Cmd, or staged T66.exe process was observed in this classification snapshot.

`	ext
ProcessName    Id StartTime             Path
-----------    -- ---------             ----
git          7248 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         14964 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         16036 5/28/2026 10:58:28 AM C:\Program Files\Git\mingw64\bin\git.exe
git         18224 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         18732 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         20220 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         24300 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         24804 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         25436 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         25832 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         27448 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         30708 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         31380 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         32508 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         33956 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         34008 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         34328 5/28/2026 10:58:19 AM C:\Program Files\Git\mingw64\bin\git.exe
git         35032 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
git         35724 5/28/2026 10:58:33 AM C:\Program Files\Git\mingw64\bin\git.exe
`

## Measurement Source Decision

Phase 1 and Phase 3 use C:\UE\T66_B11B12_Worktree and record a source manifest plus staged executable SHA before captures. The live tree remains untouched except for reviewed process/report artifacts and later integration patches after Claude approval.
