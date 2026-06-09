# Foundation Ownership Inventory

Pass 1 baseline for lifecycle, shutdown, travel, durable-state, and teardown ownership.

This document is the contract for later cleanup passes. It does not approve runtime behavior changes by itself.

## Evidence

Reusable scanner:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\Invoke-T66FoundationInventoryScan.ps1 `
  -JsonPath C:\UE\T66\LifecycleSystem\FOUNDATION_OWNERSHIP_SCAN.json `
  -MarkdownPath C:\UE\T66\LifecycleSystem\FOUNDATION_OWNERSHIP_SCAN.md
```

Current generated evidence:

- `LifecycleSystem/FOUNDATION_OWNERSHIP_SCAN.md`
- `LifecycleSystem/FOUNDATION_OWNERSHIP_SCAN.json`
- Pass 2/3 before snapshot: `Saved/FoundationPass2_3/before/FOUNDATION_OWNERSHIP_SCAN.md`
- Pass 2/3 after snapshot: `Saved/FoundationPass2_3/after/FOUNDATION_OWNERSHIP_SCAN.md`
- Pass 3 world-transition classification snapshot: `Saved/FoundationPass3/before/FOUNDATION_OWNERSHIP_SCAN.md`

The scan is classified. Counts below are not raw grep totals; declarations, definitions, API surfaces, proof exits, and call sites are separated.

## Current Classified Snapshot

| Surface | Classified Count | Current Shape | Intended Owner | Migration Pass |
|---|---:|---|---|---|
| Raw world travel through `UGameplayStatics::OpenLevel` | 15 call sites | Pass 3 classified every remaining call as a wrapper internal, null-GI safety fallback, loaded-run fast-resume bypass, session/current-map dynamic travel, or proof fallback. The loaded-run fast-resume bypasses were resolved in the loaded-save transition normalization pass. | Lifecycle world-transition policy, with `UT66GameInstance` / `UT66SessionSubsystem` as runtime owners; local save resume now uses the GameInstance gameplay transition wrapper, while session/current-map travel remains owner-specific. | Pass 3 complete + loaded-save normalization |
| Run reset through `ResetForNewRun` | 7 call sites, 10 definitions | External RunState compatibility callers are migrated. Remaining reset-named call sites are owner-local child resets inside RunState or an explicit DamageLog proof reset. | `UT66RunStateSubsystem` run boundaries: `BeginNewRun`, `BeginLoadedRun`, `EndRun`, `ReturnRunToFrontend`. | Pass 2 |
| Sync durable saves through `UGameplayStatics::SaveGameToSlot` | 13 direct calls | Achievements, community content, companion unlocks, leaderboard, player settings, run summary, save subsystem, and proof/summary paths save independently. | Owner-local save methods plus durable-state flush classification. | Pass 4 |
| Async durable saves through `UGameplayStatics::AsyncSaveGameToSlot` | 4 direct async calls | Save subsystem, buff subsystem, and achievements queue async saves without a shared final flush contract. | Owner-local save methods plus a future durable-state flush coordinator. | Pass 4 |
| Shutdown participant registration | 11 registrations, 2 shutdown API rows | Existing shutdown participants are registered owner-locally. | `UT66ShutdownSubsystem` registry, with cleanup staying in each resource owner. | Pass 6 |
| Player quit request | 1 player-facing call, 2 shutdown API rows | Quit modal calls `RequestQuitGame` when the subsystem exists. | `ShutdownSystem` / `UT66ShutdownSubsystem`. | Pass 6 |
| Direct quit fallback | 1 direct fallback | Quit modal falls back to `UKismetSystemLibrary::QuitGame` if shutdown subsystem lookup fails. | Shutdown-owned behavior unless Pass 6 proves this must remain an emergency fallback. | Pass 6 |
| Direct status-code exits | 2 shutdown final exits, 34 proof/automation exits, 16 direct status exits | Proof and fatal/error lanes use `FPlatformMisc::RequestExitWithStatus` directly. | Shutdown final exit or proof/fatal owner by classification; proof lanes stay out of player quit unless a proof explicitly requests full shutdown. | Pass 6 / proof exception |
| World runtime cleanup hooks | 30 `Deinitialize` definitions, 108 `EndPlay` references, 5 owner cleanup-helper rows, 60 other cleanup references | World and actor owners already contain cleanup surfaces, but pre-travel drain need is not proven by this count alone. | World subsystem or actor owner-local cleanup, coordinated only after the teardown audit proves need. | Pass 5 |

## Ownership Contract By Surface

### World Transition

Current raw-travel exception files:

- `Source/T66/Core/T66GameInstance.cpp`
- `Source/T66/Core/T66SessionSubsystem.cpp`
- `Source/T66/Core/T66WorldRuntimeProofCommands.cpp`
- `Source/T66/Gameplay/T66CowardiceGate.cpp`
- `Source/T66/Gameplay/T66PlayerController_Input.cpp`
- `Source/T66/Gameplay/T66StageGate.cpp`
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66PauseMenuScreen.cpp`
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`

Contract: Pass 3 classified these sites as intentional exceptions, not automatic migrations. Future callers should express transition intent; the transition owner should decide flush, drain, diagnostics, and the actual travel call only when behavior matches or a behavior change is approved.

Pass 3 retained 17 raw travel call sites because no remaining site was behavior-equivalent to an existing wrapper. The later loaded-save transition normalization pass deliberately changed local SaveSlots load/preview behavior so valid saved-run snapshots now use `UT66GameInstance::TransitionToGameplayLevel` after snapshot state is prepared.

| Category | Count | Files / Sites | Decision |
|---|---:|---|---|
| `UT66GameInstance` wrapper internals | 2 | `Source/T66/Core/T66GameInstance.cpp` frontend and gameplay wrapper bodies | Keep. These are the owner-local wrapper implementations that perform the actual engine travel. |
| Null-`GameInstance` gameplay fallbacks | 5 | `T66CompanionSelectionScreen.cpp`, `T66HeroSelectionScreen.cpp`, `T66PauseMenuScreen.cpp`, `T66RunSummaryScreen.cpp`, `T66PlayerController_Input.cpp` | Keep. Normal paths already call `TransitionToGameplayLevel`; the raw branch only exists when the `UT66GameInstance` instance needed for the gameplay wrapper is unavailable. |
| Loaded-run fast-resume bypass | 0 | Formerly `T66SaveSlotsScreen.cpp` load and preview-entry paths | Resolved. Local loaded-save resume now preserves snapshot preparation and uses the gameplay wrapper's curtain, preload, and timer delay. Party/session loaded-save resume remains in `UT66SessionSubsystem` and still uses `StartLoadedGameplayTravel` / `ServerTravel`. |
| Session/current-map dynamic travel | 6 | `T66SessionSubsystem.cpp`, `T66StageGate.cpp`, `T66CowardiceGate.cpp` | Keep. Listen-server travel, current-map reload, and session cleanup routes need behavior that the fixed frontend/gameplay wrappers do not represent. |
| Proof fallback travel | 2 | `T66WorldRuntimeProofCommands.cpp` | Keep. Proof travel preserves automation contracts and uses wrappers where available, with raw gameplay fallback only when the wrapper owner is unavailable. |

Pass 3 also records a minor safety-fallback inconsistency: the Companion and Hero null-GI fallback branches use `GetTribulationEntryLevelName()`, while Pause Menu, Run Summary, and player input use `GetGameplayLevelName()`. In the current source, `GetTribulationEntryLevelName()` returns `GetGameplayLevelName()`, but unifying fallback intent would be a separate defensive cleanup if those entry names ever diverge.

### Run Boundary

Current remaining reset-named call-site files:

- `Source/T66/Core/RunState/T66RunStateSubsystem.cpp`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`

Contract: external C++ callers use canonical run boundaries. Compatibility wrappers may remain until Blueprint/legacy usage is separately audited. Owner-local child resets are intentionally still named `ResetForNewRun` because they are not the RunState boundary API.

### Durable State

Current direct save owners/callers:

- `UT66SaveSubsystem`
- `UT66BuffSubsystem`
- `UT66AchievementsSubsystem`
- `UT66LeaderboardSubsystem`
- `UT66PlayerSettingsSubsystem`
- `UT66CommunityContentSubsystem`
- `UT66CompanionUnlockSubsystem`
- Run summary / proof summary save paths

Contract: Pass 4 classifies each direct save as owner-local, coordinator-routed, proof-only, or intentionally retained. The future durable coordinator calls owners; it does not absorb their save formats or data ownership.

### Shutdown And Quit

Current registered participants:

- `UT66MediaViewerSubsystem`
- `UT66BackendSubsystem`
- `UT66SessionSubsystem`
- `UT66SteamHelper`
- `UT66UITexturePoolSubsystem`
- `UT66WebImageCache`
- `UT66AudioSubsystem`
- `UT66MusicSubsystem`
- `UT66PerformanceSubsystem`

Contract: Pass 6 can register additional participants only after owner-local cleanup exists from Pass 4 or Pass 5. Proof/fatal exits remain separate unless their owning test contract explicitly requests player quit shutdown.

### World Runtime Cleanup

Current scan includes all `Deinitialize`, `EndPlay`, and selected cleanup helper references. This intentionally over-includes actors and subsystems so Pass 5 starts from a broad candidate list.

Contract: a row in the cleanup scan is not proof of a leak. Pass 5 must classify each relevant world system as native Unreal teardown, owner-local pre-travel drain, shutdown participant, or documented no-op.

## Pass 1 Decisions

- The scanner is promoted as reusable project tooling because later passes need stable before/after classified counts.
- Raw grep counts are not accepted as pass gates; later passes compare classified call-site counts from the scanner.
- Runtime source behavior is unchanged in Pass 1.
- Suspect legacy/fallback paths remain suspect until live references prove removal is safe.

## Pass 2/3 Caller-Reduction Update

Pass 2/3 reduced external run-boundary compatibility callers without adding a new coordinator or static travel helper.

Scanner evidence:

| Surface | Before | After | Decision |
|---|---:|---:|---|
| `RunBoundary.ResetForNewRun` call sites | 32 | 7 | External `UT66RunStateSubsystem` callers were re-pointed to `BeginNewRun`, `BeginLoadedRun`, and `EndRun`. Remaining rows are owner-local child reset calls inside RunState or an explicit `DamageLog` proof reset. |
| `WorldTransition.RawOpenLevel` call sites | 17 | 17 | Retained as classified exceptions because the current gameplay wrapper adds loading-screen, preload, and curtain behavior, so the raw sites are not behavior-equivalent. |

Retained raw-travel exception categories:

- `UT66GameInstance` wrapper internals, where the actual engine travel call lives.
- `UT66SessionSubsystem` listen-server, server travel, client travel, and current-map session routes.
- Proof/status travel in `T66WorldRuntimeProofCommands`, which must preserve automation contracts.
- Dynamic current-map restarts in stage/cowardice gates.
- UI null-`GameInstance` gameplay fallbacks. The former loaded-run resume bypasses were intentionally migrated in the loaded-save transition normalization pass after behavior review.

No dedicated lifecycle coordinator was added. The current contract remains owner-local run boundaries plus documented world-transition exceptions.

## Pass 3 World-Transition Classification Update

Pass 3 made no source migration. The raw `OpenLevel` count remains `17`, and that is the intended classified state after review. The pass outcome is that remaining raw travel is no longer treated as low-confidence cleanup debt; it is a documented exception set.

Scanner evidence:

| Surface | Before | After | Decision |
|---|---:|---:|---|
| `WorldTransition.RawOpenLevel` call sites | 17 | 17 | Retained. All remaining rows are wrapper internals, null-GI safety fallback, loaded-run fast-resume bypass, session/current-map dynamic travel, or proof fallback. |

Future behavior-changing travel work should start from one of these explicit questions:

- Should the null-GI fallback branches be normalized behind a static best-effort helper, accepting that this only helps abnormal states?
- Should session/current-map travel get its own session-owned wrapper for diagnostics without changing listen/current-map semantics?

Do not treat any of those as behavior-equivalent migrations without a reviewed source plan and staged standalone proof.

## Later Pass Gates

Every implementation pass should report:

- scanner before/after counts for its surface
- files changed
- exceptions intentionally retained
- compile/smoke/staged proof required by the pass
- Claude review result
