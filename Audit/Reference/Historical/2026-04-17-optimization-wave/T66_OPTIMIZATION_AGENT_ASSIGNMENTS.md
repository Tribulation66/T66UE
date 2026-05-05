# T66 Optimization Agent Assignments

Generated: `2026-04-17`

## Current classification (2026-05-04)

This file is no longer an active pending audit. It is an operational handoff from the April 17 optimization wave and is kept as reference/history only.

The live pending source is `Audit/Pending/T66_MASTER_OPTIMIZATION_AUDIT_V5.md`, which now contains the current verified status. Some broad themes below still apply, but the ownership split, branch name, and "current biggest remaining gaps" should not be treated as fresh instructions without rechecking the current repo.

Branch: `codex/version-2.3`

Integrator checkpoint: `fa34e41150cea444d1de95d438d0dec95b6bc583`

Primary source-of-truth audit: [T66_MASTER_OPTIMIZATION_AUDIT_V5.md](</C:/UE/T66/Audit/Pending/T66_MASTER_OPTIMIZATION_AUDIT_V5.md:1>)

This file is the operational handoff for the next implementation wave. Each agent should read this file first, then work only inside its owned slice unless a tiny compile fix is absolutely required.

## Mission

Finish the remaining high-value optimization work from `V5` in one coordinated wave, with the main-thread integrator handling merge, final package validation, and any cross-slice cleanup.

Priorities for this wave:

1. Make the game feel faster and less janky in packaged standalone.
2. Finish the remaining `P0` and `P1` runtime work before the art push.
3. Avoid merge conflicts by keeping ownership boundaries strict.

## Already Integrated

These are already in the branch. Do not redo them unless your assignment specifically requires refining them.

- frontend lifecycle improvements for warm screen activation and redundant same-screen opens
- `PowerUp` warm-open improvements and several frontend refresh reductions
- invite accept kickoff improvements, faster polling, and direct Steam-lobby join fast path
- gameplay preload improvements, including a two-phase preload path and async-first character visual warmup
- terrain material refresh path no longer reloading assets inside the spawn/refresh loop
- Mini cache and pooling improvements across several hot paths
- packaged standalone rebuild and smoke validation at the current checkpoint

## Current Biggest Remaining Gaps

- multiplayer happy-path join still has fallback/retry/search complexity in [T66SessionSubsystem.cpp](</C:/UE/T66/Source/T66/Core/T66SessionSubsystem.cpp:1>)
- startup still has sync fallback loads in gameplay readiness and combat/VFX warmup
- `HeroSelection`, `Settings`, and parts of `MainMenu` are still rebuild-heavy and lambda-heavy
- `UT66GameplayHUDWidget` still does too much work per tick and still has many live lambda-driven sections
- Mini still has sync HUD/icon loads, broad tick usage, and unfinished replication/relevancy work
- main runtime still has many `TActorIterator` / world-scan paths in gameplay and overlay systems
- packaged validation matrix is still incomplete

## Shared Rules For Every Agent

- Stay on branch `codex/version-2.3`.
- Do not revert or overwrite unrelated existing edits.
- Do not touch files owned by another agent unless it is a trivial compile fix and you call that out clearly.
- Prefer runtime feel wins over theoretical cleanup.
- If a fix requires a bigger architecture change than your owned slice can safely absorb, leave a concrete note for the integrator instead of widening scope.
- Every agent should end with:
  - files changed
  - what player-facing problem improved
  - what was validated
  - what remains in that slice

## Pre-Existing Dirty Files

These files were already dirty before this agent wave. Treat them as reserved unless your assignment explicitly includes them.

- [T66BuffSaveGame.h](/C:/UE/T66/Source/T66/Core/T66BuffSaveGame.h:1)
- [T66BuffSubsystem.cpp](/C:/UE/T66/Source/T66/Core/T66BuffSubsystem.cpp:1)
- [T66BuffSubsystem.h](/C:/UE/T66/Source/T66/Core/T66BuffSubsystem.h:1)
- [T66HeroMovementComponent.cpp](/C:/UE/T66/Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp:1)
- [T66HeroMovementTypes.h](/C:/UE/T66/Source/T66/Gameplay/Movement/T66HeroMovementTypes.h:1)
- [T66HeroBase.h](/C:/UE/T66/Source/T66/Gameplay/T66HeroBase.h:1)
- [T66TowerMapTerrain.cpp](/C:/UE/T66/Source/T66/Gameplay/T66TowerMapTerrain.cpp:1)
- [T66ShopScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66ShopScreen.cpp:1)
- [T66TemporaryBuffSelectionScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:1)
- [T66TemporaryBuffShopScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66TemporaryBuffShopScreen.cpp:1)
- [T66StatsPanelSlate.cpp](/C:/UE/T66/Source/T66/UI/T66StatsPanelSlate.cpp:1)
- `SourceAssets/UI/WebLab/...`
- `Tools/ChatGPTBridge/*.log`

## Shared Validation Commands

Editor build:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development 'C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild
```

Standalone restage:

```powershell
powershell -ExecutionPolicy Bypass -File Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipCook
```

Packaged executable:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Packaged log:

- [T66.log](</C:/UE/T66/Saved/StagedBuilds/Windows/T66/Saved/Logs/T66.log:1>)

## Agent 1: Multiplayer / Session / Backend

Goal:

- Finish the healthy invite-accept -> joined-lobby path so it is as flat and deterministic as possible.

Own these files:

- [T66SessionSubsystem.cpp](/C:/UE/T66/Source/T66/Core/T66SessionSubsystem.cpp:1)
- [T66SessionSubsystem.h](/C:/UE/T66/Source/T66/Core/T66SessionSubsystem.h:1)
- [T66PartyInviteModal.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66PartyInviteModal.cpp:1)
- [T66BackendSubsystem.cpp](/C:/UE/T66/Source/T66/Core/T66BackendSubsystem.cpp:1)
- backend invite/join routes under `C:\UE\Backend\src\app\api\party-invite\`

Primary outcomes:

- reduce or remove healthy-path dependence on `FindFriendSession` retries
- prefer direct lobby/connect data whenever the invite or OSS result already contains enough information
- keep destroy-before-join only where it is truly required
- keep diagnostics granular so the integrator can time:
  - accept click
  - backend response
  - join kickoff
  - lobby lookup
  - actual travel

Do not touch:

- preload/readiness systems
- frontend hot screens outside invite UI
- Mini slice

Validation:

- editor build
- backend build if backend files changed
- at minimum one packaged smoke run

Stretch goals:

- tighten packaged Steam invite timing diagnostics
- improve backend happy-path route latency where the client is still waiting on avoidable work

## Agent 2: Frontend Hot Screens

Goal:

- remove the remaining high-cost rebuild and live-polling behavior from the hottest frontend screens.

Own these files:

- [T66HeroSelectionScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66HeroSelectionScreen.cpp:1)
- [T66HeroSelectionScreen.h](/C:/UE/T66/Source/T66/UI/Screens/T66HeroSelectionScreen.h:1)
- [T66SettingsScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66SettingsScreen.cpp:1)
- [T66SettingsScreen.h](/C:/UE/T66/Source/T66/UI/Screens/T66SettingsScreen.h:1)
- [T66MainMenuScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66MainMenuScreen.cpp:1)
- [T66MainMenuScreen.h](/C:/UE/T66/Source/T66/UI/Screens/T66MainMenuScreen.h:1)
- [T66AccountStatusScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66AccountStatusScreen.cpp:1)
- [T66CompanionSelectionScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp:1)
- [T66UIManager.cpp](/C:/UE/T66/Source/T66/UI/T66UIManager.cpp:1) only if needed

Primary outcomes:

- stop full `ForceRebuildSlate()` calls for small state changes
- reduce hot `Text_Lambda` / `Visibility_Lambda` usage where the data is event-driven or selection-driven
- keep warm re-entry cheap for `HeroSelection`, `Settings`, and `MainMenu`
- avoid introducing stale-state bugs while reducing rebuilds

Do not touch:

- [T66ShopScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66ShopScreen.cpp:1)
- [T66TemporaryBuffSelectionScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp:1)
- [T66TemporaryBuffShopScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66TemporaryBuffShopScreen.cpp:1)

Validation:

- editor build
- manually smoke the screen open/close paths you touched
- if practical, restage and check the packaged frontend open path

Stretch goals:

- reduce viewport-responsive rebuild churn
- formalize cached state updates instead of per-widget lambda polling

## Agent 3: Startup / Preload / Gameplay Readiness

Goal:

- remove the remaining sync-fallback pain from match entry and first-use gameplay presentation.

Own these files:

- [T66GameInstance.cpp](/C:/UE/T66/Source/T66/Core/T66GameInstance.cpp:1)
- [T66GameInstance.h](/C:/UE/T66/Source/T66/Core/T66GameInstance.h:1)
- [T66CharacterVisualSubsystem.cpp](/C:/UE/T66/Source/T66/Core/T66CharacterVisualSubsystem.cpp:1)
- [T66CharacterVisualSubsystem.h](/C:/UE/T66/Source/T66/Core/T66CharacterVisualSubsystem.h:1)
- [T66PlayerController.cpp](/C:/UE/T66/Source/T66/Gameplay/T66PlayerController.cpp:1)
- [T66CombatComponent.cpp](/C:/UE/T66/Source/T66/Gameplay/T66CombatComponent.cpp:1)
- [T66CombatVFX.cpp](/C:/UE/T66/Source/T66/Gameplay/T66CombatVFX.cpp:1)
- [T66MainMapTerrain.cpp](/C:/UE/T66/Source/T66/Gameplay/T66MainMapTerrain.cpp:1)
- [T66TerrainThemeAssets.cpp](/C:/UE/T66/Source/T66/Gameplay/T66TerrainThemeAssets.cpp:1)
- [T66TowerThemeVisuals.cpp](/C:/UE/T66/Source/T66/Gameplay/T66TowerThemeVisuals.cpp:1)

Primary outcomes:

- reduce or eliminate remaining sync fallback at gameplay startup
- keep preload truly async-first where possible
- make first-use VFX/SFX/visual setup less likely to hitch on entry
- keep terrain/theme setup from doing avoidable runtime asset churn

Do not touch:

- `T66TowerMapTerrain.cpp` unless absolutely necessary and only after reading the existing local edits carefully
- frontend screen logic outside readiness-related compile fixes

Validation:

- editor build
- restaged standalone build
- packaged smoke run that reaches gameplay if your slice changes startup/travel behavior substantially

Stretch goals:

- reduce sync DataTable fallbacks in getters if a safer cached path is available
- make combat VFX warmup less dependent on on-demand sync loads

## Agent 4: Main Runtime / HUD / World Scan Cleanup

Goal:

- remove the biggest remaining world-scan and runtime-UI churn from the main game.

Own these files:

- [T66GameMode.cpp](/C:/UE/T66/Source/T66/Gameplay/T66GameMode.cpp:1)
- [T66GameMode.h](/C:/UE/T66/Source/T66/Gameplay/T66GameMode.h:1)
- [T66GameplayHUDWidget.cpp](/C:/UE/T66/Source/T66/UI/T66GameplayHUDWidget.cpp:1)
- [T66GameplayHUDWidget.h](/C:/UE/T66/Source/T66/UI/T66GameplayHUDWidget.h:1)
- [T66VendorOverlayWidget.cpp](/C:/UE/T66/Source/T66/UI/T66VendorOverlayWidget.cpp:1)
- [T66GamblerOverlayWidget.cpp](/C:/UE/T66/Source/T66/UI/T66GamblerOverlayWidget.cpp:1)
- [T66PlayerController_Overlays.cpp](/C:/UE/T66/Source/T66/Gameplay/T66PlayerController_Overlays.cpp:1)
- [T66FrontendGameMode.cpp](/C:/UE/T66/Source/T66/Gameplay/T66FrontendGameMode.cpp:1)
- [T66BossGate.cpp](/C:/UE/T66/Source/T66/Gameplay/T66BossGate.cpp:1)
- [T66PixelationSubsystem.cpp](/C:/UE/T66/Source/T66/Core/T66PixelationSubsystem.cpp:1) if needed

Primary outcomes:

- replace remaining hot `TActorIterator` / `GetAllActorsOfClass` usage in gameplay and overlay paths
- reduce `UT66GameplayHUDWidget` live lambda and per-tick churn where state is event-driven
- make frontend preview-stage lookups and runtime overlay lookups use cached refs or registry patterns

Do not touch:

- Agent 2 screen files unless it is a tiny compile fix
- Mini files

Validation:

- editor build
- if HUD changed materially, packaged smoke run to ensure no gameplay-HUD crash/regression

Stretch goals:

- reduce unnecessary recurring timers
- make overlay systems use one cached lookup path instead of repeated ad-hoc searches

## Agent 5: Mini Runtime / Mini UI / Mini Networking

Goal:

- finish the Mini cleanup so density, UI open cost, and runtime churn are all materially better.

Own these files:

- everything under `Source/T66Mini/`

Primary outcomes:

- remove remaining sync HUD/icon/UI loads where practical
- reduce unnecessary actor tick usage
- improve Mini projectile / enemy / pickup runtime efficiency
- apply sensible replication / relevancy / quantization improvements where clearly beneficial

Priority files to inspect first:

- [T66MiniBattleHUD.cpp](/C:/UE/T66/Source/T66Mini/Private/UI/T66MiniBattleHUD.cpp:1)
- [T66MiniIdolSelectScreen.cpp](/C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniIdolSelectScreen.cpp:1)
- [T66MiniShopScreen.cpp](/C:/UE/T66/Source/T66Mini/Private/UI/Screens/T66MiniShopScreen.cpp:1)
- [T66MiniEnemyBase.cpp](/C:/UE/T66/Source/T66Mini/Private/Gameplay/T66MiniEnemyBase.cpp:1)
- [T66MiniProjectile.cpp](/C:/UE/T66/Source/T66Mini/Private/Gameplay/T66MiniProjectile.cpp:1)
- [T66MiniEnemyProjectile.cpp](/C:/UE/T66/Source/T66Mini/Private/Gameplay/T66MiniEnemyProjectile.cpp:1)
- [T66MiniPickup.cpp](/C:/UE/T66/Source/T66Mini/Private/Gameplay/T66MiniPickup.cpp:1)
- [T66MiniGameMode.cpp](/C:/UE/T66/Source/T66Mini/Private/Gameplay/T66MiniGameMode.cpp:1)

Do not touch:

- mainline `Source/T66/` files except tiny compile fixes

Validation:

- editor build
- Mini flow smoke test if possible
- packaged smoke run if Mini startup/UI behavior changed

Stretch goals:

- if replication changes are made, keep them narrow and measurable
- prefer cheap and safe wins over speculative network architecture

## Integrator Responsibilities

The main-thread integrator will:

- merge and reconcile cross-slice conflicts
- restage and package the final standalone build
- run the final packaged smoke pass
- track what remains after the agent wave

## Out Of Scope For This Exact Agent Wave Unless Time Remains

- broad `P2` module/build/code-hygiene cleanup
- GC tuning after all churn reduction work is finished
- broad cook/always-cook cleanup and content-pruning sweep
- full scenario-matrix benchmarking and long soak validation

If an agent finishes early, it should report that first and then take a stretch goal inside its own slice rather than jumping into another owner’s files.
