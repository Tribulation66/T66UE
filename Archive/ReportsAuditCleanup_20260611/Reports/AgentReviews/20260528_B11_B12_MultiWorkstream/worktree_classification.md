# B.11/B.12 Multi-Workstream Worktree Classification

Generated: 2026-05-28T08:17:03-03:00

## Result

Resolved by process update before Stage 0a staging.

The current tracked worktree is not a narrow Lightweight Actor source state. It contains the expected uncommitted enemy-infrastructure baseline from the prior B.10/miniboss/boss-projectile work, but it also contains unrelated runtime-affecting Combat VFX, weapon data, Quick Revive/world-asset removals, core/UI/HUD changes, and report/script additions.

The active packet was amended after Claude-approved root-process changes so Codex can default to a non-destructive isolated measurement source state when unrelated dirty runtime work is present. Stage 0a will therefore be staged from an isolated source state that leaves the current dirty worktree untouched and records which paths are included or excluded.

## Commands Used

- `git status --porcelain=v1 --untracked-files=no -- Source/T66/Gameplay/T66MobBase.h ... Config`
- `git diff -- Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp`
- `git diff --stat -- Source/T66/Gameplay/T66PlayerController_Overlays.cpp Content/Data/Weapons.csv Content/Data/DT_Weapons.uasset`
- `git status --porcelain=v1 --untracked-files=no -- Content`
- `git status --porcelain=v1 --untracked-files=no -- Source PerformanceSystem Gameplay Reports Scripts Config`

During classification, stale read-only `git.exe` workers from `rev-parse`, `remote -v`, and `status --porcelain` were found idle and terminated before continuing. No `git-lfs.exe`, `RunUAT`, `UnrealEditor-Cmd`, or staged `T66.exe` process was left running after classification checks.

## Proposed Include: Prior Enemy Infrastructure Baseline

These paths appear to be pre-existing, intended state from the already-approved enemy infrastructure passes and are needed for a meaningful Stage 0a baseline:

| Path | Status | Decision | Rationale |
|---|---:|---|---|
| `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp` | M | include, with caveat | HP override cap to 50000 is required for current autocapture survival. The removed Quick Revive stubs have no remaining `rg` hits, but they are not part of this pass. |
| `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h` | M | include | Deprecated enemy projectile actor state from projectile-manager migration. |
| `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp` | M | include | Ranged routing/projectile-manager baseline. |
| `Source/T66/Gameplay/Enemies/T66RangedEnemy.h` | M | include | Ranged routing/projectile-manager baseline. |
| `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | M | include | Placed miniboss floor-door system baseline. |
| `Source/T66/Gameplay/T66BossBase.cpp` | M | include | Boss projectile-manager migration baseline. |
| `Source/T66/Gameplay/T66BossGate.cpp` | M | include | Boss/miniboss infrastructure baseline. |
| `Source/T66/Gameplay/T66BossGate.h` | M | include | Boss/miniboss infrastructure baseline. |
| `Source/T66/Gameplay/T66BossProjectile.h` | M | include | Deprecated boss projectile actor state from projectile-manager migration. |
| `Source/T66/Gameplay/T66EnemyDirector.cpp` | M | include | Route attribution/miniboss promotion removal baseline. |
| `Source/T66/Gameplay/T66GameMode.cpp` | M | include | Current tower/enemy infrastructure baseline. |
| `Source/T66/Gameplay/T66GameMode.h` | M | include | Current tower/enemy infrastructure baseline. |
| `Source/T66/Gameplay/T66TowerDescentHole.h` | M | include | Placed miniboss gate helper accessor. |
| `Source/T66/Gameplay/pending_issues_Gameplay.md` | M | include | Prior pass issue tracking. |
| `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp` | M | include | B.10.1B I/O mitigation baseline. |

## Needs Pablo Decision: Mixed Or Unrelated Runtime State

These paths affect runtime and are not part of the approved B.11/B.12 workstreams. They cannot be silently absorbed into Stage 0a without making the baseline ambiguous.

| Path | Status | Proposed decision needed |
|---|---:|---|
| `Content/Data/CombatVFXBindings.csv` | A | include as current live source, or exclude/clean before Stage 0a |
| `Content/Data/DT_CombatVFXBindings.uasset` | A | include as current live source, or exclude/clean before Stage 0a |
| `Content/Data/DT_Weapons.uasset` | M | include as current live source, or exclude/clean before Stage 0a |
| `Content/Data/Weapons.csv` | M | include as current live source, or exclude/clean before Stage 0a |
| `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset` | A | include as current live source, or exclude/clean before Stage 0a |
| `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | M | mixed: contains projectile/boss smoke hooks relevant to this pass, but also unrelated Hero1Axe/VFX proof changes |
| `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Core/T66GameInstance.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Core/T66GameInstance.h` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Core/T66LagTrackerSubsystem.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Core/T66RunStateSubsystem.h` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Core/T66TrapSubsystem.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Data/T66DataTypes.h` | M | unrelated data/schema change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/GameMode/T66GameMode_Lab.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/Movement/T66HeroMovementTypes.h` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66CombatComponent.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66CombatComponent.h` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66CombatShared.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66CombatShared.h` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66EnemyBase.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66HeroBase.h` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66TerrainThemeAssets.cpp` | M | unrelated runtime/content selection change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66TowerMapTerrain.cpp` | M | unrelated runtime/world change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66TowerMapTerrain.h` | M | unrelated runtime/world change unless Pablo declares it part of current live baseline |
| `Source/T66/Gameplay/T66TutorialManager.cpp` | M | unrelated runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/UI/HUD/T66GameplayHUDWidget.cpp` | M | unrelated UI runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Build.cpp` | M | unrelated UI runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/UI/HUD/T66GameplayHUDWidget_Refresh.cpp` | M | unrelated UI runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` | M | unrelated UI runtime change unless Pablo declares it part of current live baseline |
| `Source/T66/UI/T66GameplayHUDWidget.h` | M | unrelated UI runtime change unless Pablo declares it part of current live baseline |
| `Source/T66Editor/T66Editor.Build.cs` | M | unrelated editor/build change unless Pablo declares it part of current live baseline |

## Needs Pablo Decision: Deleted Runtime Assets And Source

These deletions are runtime/content-affecting and unrelated to B.11/B.12 unless Pablo declares the current tree the baseline:

| Path | Status | Proposed decision needed |
|---|---:|---|
| `Content/UI/Sprites/Interactables/QuickReviveIcon.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/MI_HillTile1.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/MI_HillTile2.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/MI_HillTile3.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/MI_HillTile4.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/T_HillTile1.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/T_HillTile2.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/T_HillTile3.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Cliffs/T_HillTile4.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Materials/MI_QuickReviveVending_Pixal3D.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Materials/MI_QuickReviveVending_Pixal3D_Outline.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Materials/M_QuickReviveVending_QuadRetro.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/QuickReviveVending_QuadRetro.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D_Outline.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_0.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_1.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_InnerLines.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_Pixal3D_Tint.uasset` | D | include or restore/clean before Stage 0a |
| `Content/World/Interactables/Vending/Textures/T_QuickReviveVending_QuadRetro_Pixelated_512.uasset` | D | include or restore/clean before Stage 0a |
| `Source/T66/Gameplay/T66QuickReviveVendingMachine.cpp` | D | include or restore/clean before Stage 0a |
| `Source/T66/Gameplay/T66QuickReviveVendingMachine.h` | D | include or restore/clean before Stage 0a |
| `Scripts/ImportInteractableUISprites.py` | D | include or restore/clean before Stage 0a |

## Documentation, Reports, And Scripts

These do not directly enter the staged runtime binary, but they are still dirty and should not be hidden:

| Group | Status | Decision |
|---|---:|---|
| `Gameplay/Combat/*` new Combat VFX process docs | A/M | unrelated to this pass; leave untouched |
| `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/*` | A | unrelated proof/review artifact; leave untouched |
| `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/*` | A | unrelated proof artifact; leave untouched |
| `Scripts/BuildT66VideoEvidenceBundle.py` | A | unrelated tool; leave untouched unless Pablo includes current live tooling baseline |
| `Scripts/CaptureT66GameplayVideo.ps1` | A | unrelated tool; leave untouched unless Pablo includes current live tooling baseline |
| `Scripts/RunHero1AxeAOEVFXBindingProof.ps1` | A | unrelated tool; leave untouched unless Pablo includes current live tooling baseline |
| `Scripts/ValidateCombatVFXProductionBindings.py` | A | unrelated tool; leave untouched unless Pablo includes current live tooling baseline |
| `Scripts/AuditWorldAssetsAndExit.py` | M | unrelated world cleanup tooling; leave untouched |
| `Scripts/README.md` | M | unrelated docs; leave untouched |
| `Scripts/SetupWeaponsDataTable.py` | M | tied to weapon data changes; needs same Pablo decision as weapon data |
| `Scripts/pending_issues_Scripts.md` | M | unrelated issue tracking; leave untouched |
| `Gameplay/Movement/MASTER_MOVEMENT.md` | M | unrelated docs; leave untouched |
| `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md` | M | unrelated docs; leave untouched |
| `Gameplay/Stats/MASTER_STATS.md` | M | unrelated docs; leave untouched |
| `Gameplay/Traps/MASTER_TRAPS.md` | M | may be relevant only to W3 trap log cleanup documentation; otherwise leave untouched |
| `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` | M | may be prior placed-miniboss documentation; include only as documentation baseline |
| `Gameplay/World/T66_Tower_Multi_Agent_Implementation_Plan.md` | M | may be prior placed-miniboss documentation; include only as documentation baseline |

## Source-State Decision

Use a non-destructive isolated measurement source state before Stage 0a.

Isolated worktree:

- Path: `C:\UE\T66_B11B12_Worktree`
- Base commit: `cdd3f896bc91690a7bf2a41463caaefd67b99683`
- Baseline patch: `C:\UE\T66_B11B12_Base.patch`
- Baseline patch SHA256: `302583AEEAA5AAE9A88E7F4355CFA9C6B2EB5957A7DDBB7583CEA2C7AEEB00FC`
- Applied baseline dirty paths in the isolated worktree:
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp`
  - `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h`
  - `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp`
  - `Source/T66/Gameplay/Enemies/T66RangedEnemy.h`
  - `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`
  - `Source/T66/Gameplay/T66BossBase.cpp`
  - `Source/T66/Gameplay/T66BossProjectile.h`
  - `Source/T66/Gameplay/T66EnemyDirector.cpp`
  - `Source/T66/Gameplay/T66GameMode.cpp`
  - `Source/T66/Gameplay/T66GameMode.h`
  - `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - `Source/T66/Gameplay/T66TowerDescentHole.h`
  - `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp`
- Additional compile-required untracked source dependencies copied into the isolated worktree after the first build attempt showed HEAD referenced missing UCLASS types:
  - `Source/T66/Gameplay/T66BackroomsChaser.*`
  - `Source/T66/Gameplay/T66BackroomsDoorInteractable.*`
  - `Source/T66/Gameplay/T66CasinoInteractable.*`
  - `Source/T66/Gameplay/T66CasinoNPC.*`
  - `Source/T66/Gameplay/T66VendorInteractable.*`
  - `Source/T66/UI/T66CasinoGamblerTabWidget.*`
  - `Source/T66/UI/T66CasinoVendorTabWidget.*`
  - `Source/T66/UI/Gambler/*`
  - `Source/T66/Core/T66DeprecatedFeatureSettings.*`
  - `Source/T66/Gameplay/T66CombatDebugDraw.*`
  - `Source/T66/UI/T66LootWheelPresentationTypes.h`
- After subsequent compile attempts surfaced additional missing live source dependencies, the isolated baseline rule was widened to include all untracked `Source/T66` C++ source/header files and no untracked `Content`, `Config`, `Reports`, `Saved`, or data assets.
  - Manifest: `C:\UE\T66_B11B12_UntrackedSourceFiles.txt`
  - Manifest SHA256: `FBDF90FDBC29131D457FBAB6562704D6A6FA95C5AC671E154A00CFC989A43CA0`
  - File count: 62
- A later compile attempt showed the live source tree has cross-file dependencies among tracked dirty source files beyond the original enemy-baseline list. The isolated baseline was therefore widened to copy the live `Source/T66` and `Source/T66Editor` trees into `C:\UE\T66_B11B12_Worktree`, while still excluding unrelated `Content`, data, `Reports`, `Saved`, and other non-source artifacts.
  - Isolated source status manifest: `C:\UE\T66_B11B12_IsolatedSourceStatus.txt`
  - Isolated source status manifest SHA256: `E4134479D6EB6325C6497A12C4B7ECC12993EA15AF220881EF955175E1063F90`
  - Isolated source status entries: 596
  - This does mean Stage 0a measures the current live source code, but not unrelated dirty content/data assets. The final packet must call out this source-provenance compromise explicitly.
- Isolated-only build configuration patch:
  - Enabled `Niagara` in `C:\UE\T66_B11B12_Worktree\T66.uproject`.
  - Moved `Niagara` from private to public dependency in isolated `Source/T66/T66.Build.cs`.
  - Included `NiagaraSystem.h` in isolated `Source/T66/Data/T66DataTypes.h` because the live dirty data header exposes `TSoftObjectPtr<UNiagaraSystem>` in a public row type.
  - This patch is build-enabling for the isolated measurement source and must be called out in the final packet. It has not been applied to the main dirty worktree.

Included in the isolated source:

- the prior enemy infrastructure baseline paths listed above
- any mixed automation hook file needed for enemywaveperf/proof execution, with included/excluded rationale recorded before capture

Excluded from the isolated source:

- unrelated Combat VFX content/data/proof artifacts
- unrelated weapon data changes
- unrelated Quick Revive/world asset deletions
- unrelated UI/HUD/core/world/movement/runtime edits unless directly required by the approved B.11/B.12 packet

The current dirty worktree remains untouched. No revert, stash, reset, clean, or discard operation is authorized or needed for Stage 0a.
