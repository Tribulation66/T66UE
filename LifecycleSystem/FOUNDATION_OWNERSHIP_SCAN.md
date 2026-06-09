# Foundation Inventory Scan

- Generated UTC: 2026-06-07T18:13:10Z
- Source files scanned: 634
- Counts are classified rows, not raw grep totals.

## Summary

| Surface | Kind | Count |
|---|---|---:|
| `DurableState.AsyncSaveGameToSlot` | `DirectAsyncSaveCall` | 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | 13 |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | 16 |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | 34 |
| `Exit.DirectRequestExitWithStatus` | `ShutdownFinalExit` | 2 |
| `Quit.DirectQuitGameFallback` | `DirectQuitFallback` | 1 |
| `Quit.RequestQuitGame` | `PlayerQuitCall` | 1 |
| `Quit.RequestQuitGame` | `ShutdownApi` | 3 |
| `RunBoundary.ResetForNewRun` | `CallSite` | 7 |
| `RunBoundary.ResetForNewRun` | `Definition` | 10 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | 11 |
| `Shutdown.RegisterParticipant` | `ShutdownApi` | 2 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | 60 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | 30 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | 108 |
| `WorldRuntime.CleanupHooks` | `OwnerCleanupHelper` | 5 |
| `WorldTransition.RawOpenLevel` | `CallSite` | 17 |

## Rows

| Surface | Kind | Path | Line | Intended Owner | Migration Pass |
|---|---|---|---:|---|---|
| `DurableState.AsyncSaveGameToSlot` | `DirectAsyncSaveCall` | `Source/T66/Core/T66AchievementsSubsystem.cpp` | 1222 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.AsyncSaveGameToSlot` | `DirectAsyncSaveCall` | `Source/T66/Core/T66BuffSubsystem.cpp` | 604 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.AsyncSaveGameToSlot` | `DirectAsyncSaveCall` | `Source/T66/Core/T66SaveSubsystem.cpp` | 268 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.AsyncSaveGameToSlot` | `DirectAsyncSaveCall` | `Source/T66/Core/T66SaveSubsystem.cpp` | 489 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66AchievementsSubsystem.cpp` | 1205 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66BuffSubsystem.cpp` | 635 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66CommunityContentSubsystem.cpp` | 885 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66CompanionUnlockSubsystem.cpp` | 37 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66LeaderboardSubsystem.cpp` | 534 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66LeaderboardSubsystem.cpp` | 1056 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66LeaderboardSubsystem.cpp` | 1087 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` | 370 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` | 1093 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66SaveSubsystem.cpp` | 562 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Core/T66SaveSubsystem.cpp` | 578 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 5282 | Durable-state owner plus future flush coordinator | Pass 4 |
| `DurableState.SyncSaveGameToSlot` | `DirectSaveCall` | `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` | 4240 | Durable-state owner plus future flush coordinator | Pass 4 |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Core/T66GameInstance.cpp` | 216 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 875 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 941 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 1182 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 1196 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 1209 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 1220 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 1235 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 5898 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 5974 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6070 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6117 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6351 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6422 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6619 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `DirectStatusExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6677 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Core/pending_issues_Core.md` | 40 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Core/T66WorldRuntimeProofCommands.cpp` | 729 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Core/T66WorldRuntimeProofCommands.cpp` | 787 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Backrooms.cpp` | 308 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 1876 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 1883 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 1902 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 1921 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 1940 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 1958 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 2169 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 2263 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` | 2346 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66BossHazardSubsystem.cpp` | 943 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66MobManagerSubsystem.cpp` | 2709 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` | 930 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 432 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 864 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 1273 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 1338 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 2648 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 2861 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 3182 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 5389 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 5883 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6056 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6192 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6212 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6262 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6337 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6575 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 6733 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 8438 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ProofOrAutomationExit` | `Source/T66/PerformanceSystem/T66MobLootStressHarnessActor.cpp` | 255 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ShutdownFinalExit` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 194 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Exit.DirectRequestExitWithStatus` | `ShutdownFinalExit` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 318 | Shutdown final exit or proof/fatal owner by classification | Pass 6 / proof exception |
| `Quit.DirectQuitGameFallback` | `DirectQuitFallback` | `Source/T66/UI/Screens/T66QuitConfirmationModal.cpp` | 168 | ShutdownSystem, unless documented emergency fallback | Pass 6 |
| `Quit.RequestQuitGame` | `PlayerQuitCall` | `Source/T66/UI/Screens/T66QuitConfirmationModal.cpp` | 162 | ShutdownSystem / UT66ShutdownSubsystem | Pass 6 |
| `Quit.RequestQuitGame` | `ShutdownApi` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 38 | ShutdownSystem / UT66ShutdownSubsystem | Pass 6 |
| `Quit.RequestQuitGame` | `ShutdownApi` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 168 | ShutdownSystem / UT66ShutdownSubsystem | Pass 6 |
| `Quit.RequestQuitGame` | `ShutdownApi` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.h` | 71 | ShutdownSystem / UT66ShutdownSubsystem | Pass 6 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 593 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 602 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 634 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 638 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 643 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp` | 367 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `CallSite` | `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | 4063 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 483 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66DamageLogSubsystem.cpp` | 90 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66DamageLogSubsystem.h` | 56 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66IdolManagerSubsystem.cpp` | 502 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66IdolManagerSubsystem.h` | 57 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66RunStateSubsystem.h` | 1559 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66SkillRatingSubsystem.cpp` | 65 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66SkillRatingSubsystem.h` | 36 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66WeaponManagerSubsystem.cpp` | 17 | UT66RunStateSubsystem run boundary | Pass 2 |
| `RunBoundary.ResetForNewRun` | `Definition` | `Source/T66/Core/T66WeaponManagerSubsystem.h` | 25 | UT66RunStateSubsystem run boundary | Pass 2 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/Backend/T66BackendSubsystem.cpp` | 302 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66AudioSubsystem.cpp` | 34 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66BuffSubsystem.cpp` | 129 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66MediaViewerSubsystem.cpp` | 75 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66MusicSubsystem.cpp` | 114 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66SaveSubsystem.cpp` | 151 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66SessionSubsystem.cpp` | 145 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66SteamHelper.cpp` | 261 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66UITexturePoolSubsystem.cpp` | 39 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/Core/T66WebImageCache.cpp` | 36 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ParticipantRegistration` | `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp` | 795 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ShutdownApi` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 102 | ShutdownSystem participant registry | Pass 6 |
| `Shutdown.RegisterParticipant` | `ShutdownApi` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.h` | 59 | ShutdownSystem participant registry | Pass 6 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/Backend/T66BackendSubsystem.cpp` | 325 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 57 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 99 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.h` | 57 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66AchievementsSubsystem.cpp` | 279 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66AchievementsSubsystem.h` | 59 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66AudioSubsystem.cpp` | 58 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66AudioSubsystem.h` | 29 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66BackendSubsystem.h` | 157 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66BuffSubsystem.cpp` | 153 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66BuffSubsystem.h` | 74 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66CompanionUnlockSubsystem.cpp` | 20 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66CompanionUnlockSubsystem.h` | 25 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66EnemyPoolSubsystem.cpp` | 82 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66EnemyPoolSubsystem.h` | 51 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.cpp` | 144 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.h` | 33 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66LagTrackerSubsystem.cpp` | 151 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66LagTrackerSubsystem.h` | 47 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66LocalizationSubsystem.cpp` | 79 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66LocalizationSubsystem.h` | 59 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66MediaViewerSubsystem.cpp` | 98 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66MediaViewerSubsystem.h` | 37 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66MusicSubsystem.cpp` | 162 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66MusicSubsystem.h` | 30 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66PartySubsystem.cpp` | 125 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66PartySubsystem.h` | 66 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66PixelVFXSubsystem.cpp` | 113 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66PixelVFXSubsystem.h` | 32 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` | 184 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66PlayerSettingsSubsystem.h` | 33 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66RetroFXSubsystem.cpp` | 583 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66RetroFXSubsystem.h` | 78 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66RngSubsystem.cpp` | 29 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66RngSubsystem.h` | 27 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66RunStateSubsystem.h` | 137 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66SaveSubsystem.cpp` | 174 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66SaveSubsystem.h` | 24 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66SessionSubsystem.cpp` | 168 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66SessionSubsystem.h` | 28 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66StageProgressionSubsystem.cpp` | 70 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66StageProgressionSubsystem.h` | 84 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66SteamHelper.cpp` | 359 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66SteamHelper.h` | 64 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66UITexturePoolSubsystem.cpp` | 62 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66UITexturePoolSubsystem.h` | 35 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66WebImageCache.cpp` | 59 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Core/T66WebImageCache.h` | 23 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66BossHazardSubsystem.cpp` | 147 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66BossHazardSubsystem.h` | 61 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66MobLootSubsystem.cpp` | 136 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66MobLootSubsystem.h` | 121 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66MobManagerSubsystem.cpp` | 1350 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66MobManagerSubsystem.h` | 198 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp` | 305 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.h` | 279 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp` | 633 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h` | 166 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp` | 818 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `CleanupReference` | `Source/T66/PerformanceSystem/T66PerformanceSubsystem.h` | 23 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/Backend/T66BackendSubsystem.cpp` | 314 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/RunState/T66RunStateSubsystem.cpp` | 50 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/Shutdown/T66ShutdownSubsystem.cpp` | 96 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66AchievementsSubsystem.cpp` | 270 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66AudioSubsystem.cpp` | 47 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66BuffSubsystem.cpp` | 142 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66CompanionUnlockSubsystem.cpp` | 18 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66EnemyPoolSubsystem.cpp` | 76 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66FloatingCombatTextPoolSubsystem.cpp` | 113 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66LagTrackerSubsystem.cpp` | 135 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66LocalizationSubsystem.cpp` | 77 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66MediaViewerSubsystem.cpp` | 87 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66MusicSubsystem.cpp` | 151 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66PartySubsystem.cpp` | 114 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66PixelVFXSubsystem.cpp` | 108 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` | 172 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66RetroFXSubsystem.cpp` | 552 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66RngSubsystem.cpp` | 27 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66SaveSubsystem.cpp` | 163 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66SessionSubsystem.cpp` | 157 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66StageProgressionSubsystem.cpp` | 62 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66SteamHelper.cpp` | 348 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66UITexturePoolSubsystem.cpp` | 51 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Core/T66WebImageCache.cpp` | 48 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Gameplay/T66BossHazardSubsystem.cpp` | 125 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Gameplay/T66MobLootSubsystem.cpp` | 107 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Gameplay/T66MobManagerSubsystem.cpp` | 1341 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp` | 276 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/Gameplay/T66ProjectileManagerSubsystem.cpp` | 602 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `DeinitializeDefinition` | `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp` | 807 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.cpp` | 84 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.cpp` | 91 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h` | 50 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp` | 167 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp` | 170 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h` | 127 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66BossBase.cpp` | 2284 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66BossBase.cpp` | 2296 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66BossBase.h` | 181 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66ChestInteractable.cpp` | 24 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66ChestInteractable.cpp` | 27 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66ChestInteractable.h` | 29 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66CombatComponent.cpp` | 638 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66CombatComponent.cpp` | 669 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66CombatComponent.h` | 88 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66CompanionBase.cpp` | 198 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66CompanionBase.cpp` | 205 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66CompanionBase.h` | 87 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66EnemyBase.cpp` | 851 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66EnemyBase.cpp` | 861 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66EnemyBase.h` | 221 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66EnemyDirector.cpp` | 300 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66EnemyDirector.cpp` | 321 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66EnemyDirector.h` | 129 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66FrontendGameMode.cpp` | 61 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66FrontendGameMode.cpp` | 71 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66FrontendGameMode.h` | 23 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66GalleryDisplayActor.cpp` | 103 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66GalleryDisplayActor.cpp` | 113 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66GalleryDisplayActor.h` | 33 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66GameMode.cpp` | 1124 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66GameMode.cpp` | 1160 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66GameMode.h` | 190 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66HeroBase.cpp` | 889 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66HeroBase.cpp` | 898 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66HeroBase.h` | 214 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66HeroPlagueCloud.cpp` | 146 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66HeroPlagueCloud.cpp` | 154 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66HeroPlagueCloud.h` | 42 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66KnockbackComponent.cpp` | 90 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66KnockbackComponent.cpp` | 103 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66KnockbackComponent.h` | 209 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LavaPatch.cpp` | 159 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LavaPatch.cpp` | 167 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LavaPatch.h` | 113 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LethalZoneComponent.cpp` | 30 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LethalZoneComponent.cpp` | 33 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LethalZoneComponent.h` | 25 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LootBagPickup.cpp` | 127 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LootBagPickup.cpp` | 143 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LootBagPickup.h` | 100 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LootWheelInteractable.cpp` | 133 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LootWheelInteractable.cpp` | 136 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66LootWheelInteractable.h` | 21 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66MiasmaBoundary.cpp` | 176 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66MiasmaBoundary.cpp` | 188 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66MiasmaBoundary.h` | 47 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66MobBase.cpp` | 361 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66MobBase.cpp` | 393 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66MobBase.h` | 46 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66NPCBase.cpp` | 269 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66NPCBase.cpp` | 293 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66NPCBase.h` | 85 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PetActor.cpp` | 407 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PetActor.cpp` | 414 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PetActor.h` | 85 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PilotableTractor.cpp` | 137 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PilotableTractor.cpp` | 156 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PilotableTractor.h` | 30 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PlayerController.cpp` | 632 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PlayerController.cpp` | 671 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66PlayerController.h` | 224 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66RecruitableCompanion.cpp` | 135 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66RecruitableCompanion.cpp` | 144 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66RecruitableCompanion.h` | 94 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66SafeZoneComponent.cpp` | 48 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66SafeZoneComponent.cpp` | 73 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66SafeZoneComponent.h` | 33 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66StageGate.cpp` | 102 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66StageGate.cpp` | 112 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66StageGate.h` | 41 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66TutorialManager.cpp` | 116 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66TutorialManager.cpp` | 129 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66TutorialManager.h` | 33 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66WorldInteractableBase.cpp` | 137 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66WorldInteractableBase.cpp` | 155 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/T66WorldInteractableBase.h` | 66 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp` | 138 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66FloorFlameTrap.cpp` | 149 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66FloorFlameTrap.h` | 60 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.cpp` | 98 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.cpp` | 108 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66FloorSpikePatchTrap.h` | 70 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp` | 151 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.cpp` | 159 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h` | 34 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66TrapBase.cpp` | 30 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66TrapBase.cpp` | 37 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66TrapBase.h` | 76 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp` | 142 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66WallArrowTrap.cpp` | 150 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/Gameplay/Traps/T66WallArrowTrap.h` | 56 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/PerformanceSystem/T66MobLootStressHarnessActor.cpp` | 259 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/PerformanceSystem/T66MobLootStressHarnessActor.cpp` | 266 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/PerformanceSystem/T66MobLootStressHarnessActor.h` | 36 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/PerformanceSystem/T66OutgoingTravelerStressHarnessActor.cpp` | 249 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/PerformanceSystem/T66OutgoingTravelerStressHarnessActor.cpp` | 269 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `EndPlayReference` | `Source/T66/PerformanceSystem/T66OutgoingTravelerStressHarnessActor.h` | 38 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `OwnerCleanupHelper` | `Source/T66/Core/T66TrapSubsystem.cpp` | 556 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `OwnerCleanupHelper` | `Source/T66/Core/T66TrapSubsystem.cpp` | 641 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `OwnerCleanupHelper` | `Source/T66/Core/T66TrapSubsystem.cpp` | 651 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `OwnerCleanupHelper` | `Source/T66/Core/T66TrapSubsystem.h` | 36 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldRuntime.CleanupHooks` | `OwnerCleanupHelper` | `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp` | 974 | World subsystem or actor owner-local cleanup | Pass 5 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66GameInstance.cpp` | 2068 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66GameInstance.cpp` | 2095 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66SessionSubsystem.cpp` | 286 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66SessionSubsystem.cpp` | 321 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66SessionSubsystem.cpp` | 1928 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66WorldRuntimeProofCommands.cpp` | 813 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Core/T66WorldRuntimeProofCommands.cpp` | 844 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Gameplay/T66CowardiceGate.cpp` | 116 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Gameplay/T66PlayerController_Input.cpp` | 521 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Gameplay/T66StageGate.cpp` | 146 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/Gameplay/T66StageGate.cpp` | 179 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp` | 1654 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` | 822 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/UI/Screens/T66PauseMenuScreen.cpp` | 291 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/UI/Screens/T66RunSummaryScreen.cpp` | 4480 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp` | 1127 | LifecycleSystem / future world-transition coordinator | Pass 3 |
| `WorldTransition.RawOpenLevel` | `CallSite` | `Source/T66/UI/Screens/T66SaveSlotsScreen.cpp` | 1167 | LifecycleSystem / future world-transition coordinator | Pass 3 |
