# Runtime Ownership Inventory

This inventory maps the major runtime systems to the owner that starts them, resets them, flushes or shuts them down, and proves their health. It is a health-coverage map, not a new coordinator.

Use this document with:

- `LifecycleSystem/FOUNDATION_OWNERSHIP_INVENTORY.md` for classified call-site counts.
- `LifecycleSystem/LIFECYCLE_COORDINATOR_REGISTRY.md` for lifecycle boundary status.
- `LifecycleSystem/WORLD_RUNTIME_TEARDOWN_AUDIT.md` for world-runtime drain evidence.
- `ShutdownSystem/SHUTDOWN_REGISTRY.md` for quit/pre-exit participant ownership.
- `PerformanceSystem/RUNTIME_HEALTH_GATE.md` for reusable packaged startup/runtime diagnostics proof.

## Status Vocabulary

Lifecycle and shutdown status uses the existing registry terms:

- `implemented` - runtime boundary exists and is usable.
- `documented` - policy/owner is documented, but no new runtime code exists.
- `planned` - should be implemented in a future reviewed pass.
- `audit-first` - do not implement before the audit/proof matrix has evidence.
- `deferred` - intentionally out of scope now.
- `native-unreal` - normal Unreal `EndPlay` / `Deinitialize` is the accepted path.
- `owner-local-self-contained` - the owner drains itself; no global participant is planned unless proof shows ordering trouble.

Health-gate coverage uses:

- `covered` - current reusable gate asserts this surface directly.
- `partial` - current gate or proof covers the surface, but not every owner detail.
- `none-owner-local` - no shared gate needed yet; owner-local/native teardown is the contract.
- `routed` - proof belongs to another reusable gate.
- `n/a` - not a runtime-health proof surface.

## Inventory Matrix

| Runtime system | Lifetime | Startup owner | Shutdown owner | Lifecycle/reset owner | Durable-state owner | Health-gate coverage | Evidence | Next action |
|---|---|---|---|---|---|---|---|---|
| Packaged executable, config, source/cooked data tables | Process / packaged build | `StageStandaloneBuild.ps1` and staged executable contract | n/a | n/a | n/a | `covered` | `Scripts/RunRuntimeHealthGate.ps1` static checks and child `RunStagedBuildReadinessGate.ps1 -SkipStage -SkipSmoke` | Keep adding only high-value startup invariants to the runtime health gate. |
| `UT66ShutdownSubsystem` and player-facing Quit Game | GameInstance / pre-exit | GameInstance subsystem initialization | `ShutdownSystem` / `UT66ShutdownSubsystem::RequestQuitGame` | n/a | registered participants by owner | `routed` | `ShutdownSystem/SHUTDOWN_REGISTRY.md`; quit proof uses tagged frontend click or `T66.Shutdown.RequestQuit` in Development | Do not move quit/pre-exit ownership into lifecycle. |
| `UT66RunStateSubsystem` run begin/end/load boundaries | GameInstance / run state | GameInstance subsystem initialization | native `Deinitialize` unless participant owner says otherwise | `BeginNewRun`, `BeginLoadedRun`, `EndRun`, `ReturnRunToFrontend` | run save snapshots are owned by `UT66SaveSubsystem` / `UT66SessionSubsystem` | `partial` | Lifecycle registry rows, Pass 2/3 scanner evidence, and `RunLifecycleTransitionSmokeGate.ps1` travel proof | External C++ callers now use canonical boundaries; keep compatibility APIs for Blueprint/legacy until a separate removal audit proves they are unused. |
| Run-boundary child resets: damage log, skill rating, idol manager, weapon manager | GameInstance / run-scoped state | GameInstance subsystem initialization | native teardown | called from `UT66RunStateSubsystem::BeginNewRun` / loaded-run restore paths | owner-local state, no queued durable state found | `none-owner-local` | `UT66RunStateSubsystem::BeginNewRun` source calls child reset surfaces | Keep as owner-local child state; do not create a coordinator. |
| World transition wrappers | GameInstance / world travel | `UT66GameInstance`, `UT66SessionSubsystem`, UI/gameplay callers | native teardown plus shutdown for registered resources | `TransitionToGameplayLevel`, `TransitionToFrontendLevel`, session travel, raw exception sites | pending save state routed separately | `partial` | Lifecycle registry; foundation inventory raw `OpenLevel` classification and loaded-save transition normalization | Local loaded-save resume now uses `TransitionToGameplayLevel` after snapshot prep. Future behavior-changing work is null-GI fallback normalization or a session-owned diagnostics wrapper; session transport must stay with `UT66SessionSubsystem`. |
| Durable run save and save index | GameInstance / durable save | `UT66SaveSubsystem` | `ShutdownSystem` `DurableState` participant | run boundaries export/import snapshots through RunState/Session | `UT66SaveSubsystem` | `routed` | `RunDurableSaveIntegritySmokeGate.ps1`; `T66.Save.QueueIntegrityShutdown` / verify harness | Keep proof in durable save gate, not runtime health gate. |
| Buff progression save | GameInstance / durable save | `UT66BuffSubsystem` | `ShutdownSystem` `DurableState` participant | n/a | `UT66BuffSubsystem` | `routed` | `ShutdownSystem/SHUTDOWN_REGISTRY.md`; durable save gate covers shutdown flush path | Keep as registered async durable participant. |
| Synchronous profile/settings/community/companion durable owners | GameInstance / durable state | `UT66AchievementsSubsystem`, `UT66LeaderboardSubsystem`, `UT66PlayerSettingsSubsystem`, `UT66CommunityContentSubsystem`, `UT66CompanionUnlockSubsystem` | `native-unreal` / mutation-time save | owner-local | each subsystem owns its save format | `none-owner-local` | `ShutdownSystem/SHUTDOWN_REGISTRY.md` classifies them as sync/native or proof-only | Add a participant only if future evidence finds deferred dirty state. |
| Backend/session/Steam network platform state | GameInstance / network | `UT66BackendSubsystem`, `UT66SessionSubsystem`, `UT66SteamHelper` | `ShutdownSystem` `NetworkPlatform` participants | session travel remains owner-local | no local durable flush except owned snapshots | `partial` | shutdown registry and runtime-health clean packaged launch | Keep network cancellation in shutdown participants; avoid adding lifecycle ownership. |
| Native media and WebView resources | GameInstance / native external | `UT66MediaViewerSubsystem` | `ShutdownSystem` `NativeExternal` participant | n/a | n/a | `partial` | shutdown registry and previous quit-freeze fix path | Keep COM/WebView ownership local to media viewer. |
| Async presentation caches and audio | GameInstance / async work and media | `UT66UITexturePoolSubsystem`, `UT66WebImageCache`, `UT66AudioSubsystem`, `UT66MusicSubsystem` | `ShutdownSystem` `AsyncWork` / `MediaAudio` participants | n/a | n/a | `partial` | shutdown registry; runtime launch currently proves no fatal startup/exit markers | If feature-specific leaks appear, add owner-local diagnostics before a coordinator. |
| Performance diagnostics and reports | GameInstance / diagnostics | `UT66PerformanceSubsystem` | `ShutdownSystem` `RuntimeTick` participant | n/a | local report files under `Saved/PerformanceSystem` | `covered` | `RunRuntimeHealthGate.ps1` requires fresh snapshot/session, schema, required fields, and clean write queue | Continue adding versioned sidecar checks here when new diagnostics become required. |
| Runtime platform, release variant, localization, RNG, hero speed, stage progression, pixelation/retro style, interaction prompt | GameInstance / lightweight service state | owning subsystem initialization | native teardown unless future resource proof says otherwise | owner-local methods or read-only/config state | owner-local or none | `none-owner-local` | current source shows no shared shutdown/lifecycle proof need in this pass | Keep owner-local; inventory row is the contract until evidence changes. |
| UI/frontend screens that drive travel or quit | Widget / frontend process | frontend root/screens | quit modal routes to `ShutdownSystem`; other screens native widget teardown | UI calls into GameInstance/RunState transition owners | no durable state except routed save screens | `partial` | frontend tag-click smoke matrix; runtime health main-menu screenshot; Pass 3 null-GI fallback classification | Normal UI gameplay-entry paths should use transition wrappers. Null-GI fallback branches remain raw defensive paths unless a reviewed helper is added. |
| World-runtime native-safe systems | World / world teardown | `UT66ActorRegistrySubsystem`, `UT66EnemyPoolSubsystem`, `UT66TrapSubsystem`, `UT66ToonOutlineViewSubsystem` | `native-unreal` | world teardown / owner-local helper only | n/a | `none-owner-local` | `WORLD_RUNTIME_TEARDOWN_AUDIT.md` classifies weak-ref/tick-only/simple cleanup systems | Do not add shutdown participants without new leak evidence. |
| `UT66FloatingCombatTextPoolSubsystem` | World / pooled actors and timers | world subsystem initialization | `owner-local-self-contained` | owner-local `Deinitialize` clears timers and destroys pool actors | n/a | `none-owner-local` | world-runtime teardown audit | Keep out of global coordinator unless repeated travel proves ordering trouble. |
| World-runtime proof candidates | World / active gameplay runtime | `UT66MobManagerSubsystem`, `UT66MobLootSubsystem`, `UT66ProjectileManagerSubsystem`, `UT66BossHazardSubsystem`, `UT66OutgoingTravelerPoolSubsystem`, `UT66PixelVFXSubsystem` | `planned` in shutdown registry, owner-local proof candidates for now | `audit-first`; proof via `T66.WorldRuntime.ProofTravel` | n/a | `routed` | `RunLifecycleTransitionSmokeGate.ps1` validates repeated active-stress travel with zero non-current-world proof candidates | Keep owner-local unless heavier proof shows multiple systems require ordered drain or shared timeout diagnostics. |
| Gameplay proof/status exits | Process / automation | owning harness or proof command | out of player quit unless test requests shutdown | proof-specific | proof-specific | `n/a` | foundation inventory and shutdown registry classify direct status exits | Preserve exact status-code contracts. |

## Gate Coverage Contract

`RunRuntimeHealthGate.ps1` owns only packaged startup/runtime diagnostics checks:

- required project and schema files
- router/doc cross-links for this inventory
- staged executable/shortcut readiness in cheap mode
- short packaged MainMenu launch
- executable hash stability
- clean Unreal exit markers
- fresh PerformanceSystem snapshot/session artifacts
- current PerformanceSystem schema and clean write queue

It must not absorb:

- durable save integrity proof (`RunDurableSaveIntegritySmokeGate.ps1`)
- repeated world-travel teardown proof (`RunLifecycleTransitionSmokeGate.ps1`)
- full frontend interaction matrix (`RunFrontendTagClickSmokeMatrix.ps1`)
- release-candidate build/stage/smoke authority (`RunStagedBuildReadinessGate.ps1`)
- player-facing quit proof through the tagged Quit button

## Future Feature Rule

When adding a feature that starts long-lived work, document it in the nearest owner registry during implementation:

1. Does it own native handles, worker threads, HTTP requests, platform delegates, async loads, timers, tickers, media/audio components, world actors/components, or durable dirty state?
2. If yes, does existing Unreal teardown handle it, does owner-local `Deinitialize` drain it, or must it register with `ShutdownSystem`?
3. If it participates in run begin/end, loaded-run restore, world travel, durable flush, or runtime health proof, add or update the corresponding row in this inventory.
4. If it writes required sidecar artifacts, add a versioned schema and make `RunRuntimeHealthGate.ps1` assert freshness, schema, and clean writer state.

## Current Decision

This pass does not justify a new lifecycle coordinator or a global runtime owner. The current foundation is owner-local boundaries plus three proof lanes:

- `RunRuntimeHealthGate.ps1` for packaged startup/runtime diagnostics.
- `RunDurableSaveIntegritySmokeGate.ps1` for save-owned durable shutdown proof.
- `RunLifecycleTransitionSmokeGate.ps1` for world-runtime travel/teardown proof.

World-transition classification is complete, and the loaded-save resume curtain/preload decision has been resolved for local SaveSlots load/preview entries. The next implementation work should either address null-GI fallback normalization, add a session-owned travel diagnostics boundary without changing session transport, or move to the next unclosed owner-local risk surface before adding any new coordinator.
