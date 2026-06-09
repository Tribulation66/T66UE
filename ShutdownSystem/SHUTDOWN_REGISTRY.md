# Shutdown Registry

This registry tracks systems that must participate in the intentional pre-exit path.

Status values:

- `registered` - wired to `UT66ShutdownSubsystem`
- `native-unreal` - normal `EndPlay` / `Deinitialize` is the accepted path
- `owner-local-self-contained` - explicit owner `Deinitialize` drains local resources; no global shutdown participant is planned unless proof shows ordering trouble
- `defensive-fallback` - guarded last-resort used only when the shutdown subsystem is unavailable; not a normal quit path
- `planned` - should be registered in a future pass
- `out-of-scope` - intentionally not routed through player quit shutdown

| Owner | Resource Type | Phase | Timeout | Cleanup Method | Status | Notes |
|---|---|---:|---:|---|---|---|
| `UT66QuitConfirmationModal` | quit button normal route plus missing-subsystem backstop | n/a | n/a | Primary path calls `UT66ShutdownSubsystem::RequestQuitGame`; logs and falls back to `UKismetSystemLibrary::QuitGame` only if the subsystem is unavailable | defensive-fallback | Pass 6 classification. The direct fallback is intentionally retained so an abnormal null-subsystem UI state can still close the process, but any fallback warning should be treated as a shutdown initialization bug. |
| `UT66MediaViewerSubsystem` | WebView2, COM apartment balance, native overlay window | `NativeExternal` | 1.0s | Explicit WebView2 shutdown and audio restore | registered | Highest priority quit-freeze fix. COM ownership stays here because this subsystem calls `CoInitializeEx`; `RPC_E_CHANGED_MODE` is treated as usable COM initialized by another owner and is not counted, so shutdown never calls `CoUninitialize` for an apartment it did not initialize. |
| `UT66BackendSubsystem` | backend polling ticker, pending invite HTTP request, pending coop submits | `NetworkPlatform` | 1.0s | Cancel request, remove tickers, clear pending state | registered | Existing `Deinitialize` cleanup is reused. Runs first in `NetworkPlatform` so higher-level backend requests stop before session/Steam handles are torn down. |
| `UT66SessionSubsystem` | Steam/session delegates, rich presence, pending joins | `NetworkPlatform` | 1.0s | Clear rich presence, retry ticker, delegates, pending state | registered | Runs after backend and before low-level Steam helper cleanup. Smoke/proof exits are left alone. |
| `UT66SteamHelper` | Steam join bridge, auth ticket | `NetworkPlatform` | 1.0s | Unregister join bridge and cancel auth ticket | registered | Runs after higher-level backend/session cleanup; does not own Steam API global shutdown. |
| `UT66UITexturePoolSubsystem` | async UI texture loads and strong texture cache | `AsyncWork` | 1.0s | Cancel streamable handles and clear waiters/cache | registered | Prevents late Slate callbacks. Runs before web-image and audio async cleanup because UI surfaces can own texture waiters. |
| `UT66WebImageCache` | HTTP image waiters and rooted transient textures | `AsyncWork` | 1.0s | Drop waiters/pending set, remove rooted textures | registered | Existing HTTP request objects are not globally cancellable in current code. Runs after the texture pool and before audio asset cleanup. |
| `UT66AudioSubsystem` | async audio table/sound loads and audio caches | `AsyncWork` | 1.0s | Cancel streamable handles and clear caches | registered | Runs after UI/image async cleanup and before music component shutdown. Runtime audio component ownership remains local. |
| `UT66MusicSubsystem` | world delegates, settings delegates, async music loads, audio components | `MediaAudio` | 1.0s | Remove delegates, cancel loads, stop music | registered | Stops tracks before late audio teardown. |
| `UT66PerformanceSubsystem` | ticker, log output device, write worker, final diagnostics | `RuntimeTick` | 10.0s | Flush final report and stop worker | registered | Uses the existing bounded queue timeout. |
| `UT66SaveSubsystem` | run save and save-index async writes | `DurableState` | 1.0s | Track queued async run/index save sequence and perform final sync flush | registered | Owner-local participant also flushes from `Deinitialize`; no global save-policy rewrite. |
| `UT66BuffSubsystem` | buff progression async save | `DurableState` | 1.0s | Track queued async buff save sequence and perform final sync flush | registered | Owner-local participant also flushes from `Deinitialize`; normal gameplay saves remain async. |
| `UT66AchievementsSubsystem` | profile async save | n/a | n/a | Native `Deinitialize` calls forced synchronous `SaveProfileIfNeeded(true)` | native-unreal | Throttled async profile saves remain, but teardown forces a sync profile save even when the dirty flag was cleared by an async queue. |
| `UT66LeaderboardSubsystem` | local leaderboard and snapshot saves | n/a | n/a | Synchronous save at mutation/update points | native-unreal | No pending async durable state found; run-summary snapshot saves stay owner-local. |
| `UT66PlayerSettingsSubsystem` | player settings and legacy settings save path | n/a | n/a | Synchronous save at mutation/update points | native-unreal | Includes legacy temp slot path and `GameUserSettings::SaveSettings()` path; no quit participant until deferred dirty state is proven. |
| `UT66CommunityContentSubsystem` | community content save state | n/a | n/a | Synchronous `PersistSave` at draft/cache mutation points | native-unreal | HTTP request cancellation remains a separate backend/network concern, not durable-state flush. |
| `UT66CompanionUnlockSubsystem` | companion unlock save state | n/a | n/a | Synchronous save at unlock/reset mutation points | native-unreal | No queued durable work found. |
| run summary / proof-summary save callers | run-summary and automation summary save artifacts | n/a | n/a | Leave owner-local / proof-only | out-of-scope | Not a single subsystem owner; preserving proof/status-code contracts is required. |
| `UT66GameInstance` | core/presentation/gameplay streamable handles and preload timer | `AsyncWork` | TBD | Cancel/clear preload handles and timers | planned | Broad state owner; defer to focused follow-up. |
| `UT66ProjectileManagerSubsystem` | projectile arrays, Niagara body/trail components, HISM buckets, render host | `GameplayWorld` | TBD | Owner-local `Deinitialize` cleans projectile trails, HISM components, render host, and arrays; prove with `T66.WorldRuntime.ProofTravel` before registering | planned | Pass 5 kept this as an owner-local proof candidate because it owns transient projectile/render resources and static async Niagara load handles. Pass 5.1 added read-only snapshot fields. |
| `UT66MobManagerSubsystem` | mob manager weak arrays, lightweight mob pools, next-tick release delegate | `GameplayWorld` | TBD | Owner-local `Deinitialize` resets active/inactive arrays and VAT state; prove with `T66.WorldRuntime.ProofTravel` before registering | planned | Pass 5 kept this as an owner-local proof candidate because it schedules weak next-tick release work and owns active/inactive mob runtime state. Pass 5.1 added read-only snapshot fields and records the next-tick delegate count as a measurement gap. |
| `UT66OutgoingTravelerPoolSubsystem` | pooled outgoing travelers, target snapshot, Niagara component, render host | `GameplayWorld` | TBD | Owner-local `Deinitialize` writes proof manifest when requested, destroys Niagara/render host, and resets slots/uploads; prove with `T66.WorldRuntime.ProofTravel` before registering | planned | Pass 5 kept this as an owner-local proof candidate because it owns transient traveler render state. Pass 5.1 added read-only snapshot fields. |
| `UT66BossHazardSubsystem` | boss hazard HISM buckets, active hazards, render host | `GameplayWorld` | TBD | Owner-local `Deinitialize` destroys render components/host and clears hazards/buckets; prove with `T66.WorldRuntime.ProofTravel` before registering | planned | Pass 5 kept this as an owner-local proof candidate because it owns transient hazard render state. Pass 5.1 added read-only snapshot fields. |
| `UT66EnemyPoolSubsystem` | weak pooled inactive-enemy refs | n/a | n/a | Native world actor teardown plus weak pool reset in `Deinitialize` | native-unreal | Pass 5 found weak refs only; no timer, delegate, async load, or external resource ownership. |
| `UT66MobLootSubsystem` | active loot runtime, Niagara, HISM fallback, render host | `GameplayWorld` | TBD | Owner-local `Deinitialize` destroys render resources; prove with `T66.WorldRuntime.ProofTravel` before registering | planned | Pass 5 kept this as an owner-local proof candidate because it owns transient render/Niagara components. Pass 5.1 added read-only snapshot fields. |
| `UT66FloatingCombatTextPoolSubsystem` | active/inactive floating text actors and release timers | n/a | n/a | Owner-local `Deinitialize` clears release timers, destroys active/inactive text actors, and resets arrays | owner-local-self-contained | Pass 5 found explicit timer clearing and active/inactive actor destruction with weak timer keys; safe for world teardown but not a pure native/no-owned-cleanup case. |
| `UT66TrapSubsystem` | weak trap and managed pressure-plate refs | n/a | n/a | Native world actor teardown; `ClearManagedTrapActors` remains stage-spawn cleanup helper | native-unreal | Pass 5 found weak tracking and spawn/stage refresh cleanup only; no shutdown participant needed. |
| `UT66ActorRegistrySubsystem` | weak actor registries | n/a | n/a | Native weak-reference expiry/pruning | native-unreal | Pass 5 found weak refs only; no owned actors, timers, async work, or external handles. |
| `UT66PixelVFXSubsystem` | transient VFX components and async Niagara default-system load handle | `GameplayWorld` | TBD | Owner-local `Deinitialize` resets streamable handle/caches; prove with `T66.WorldRuntime.ProofTravel` async/component counts before registering | planned | Pass 5 kept this as an owner-local proof candidate because spawned Niagara components and async load state need travel-count proof. Pass 5.1 added read-only snapshot fields. |
| `UT66ToonOutlineViewSubsystem` | world visual parameter updates | n/a | n/a | Native tickable world subsystem teardown | native-unreal | Pass 5 found tick-only parameter updates and no owned actors, components, handles, or cached state. |
| gameplay smoke/proof `RequestExitWithStatus` calls | automated test exits and status-code proof lanes | n/a | n/a | Leave direct unless a test specifically needs player quit shutdown | out-of-scope | These exits intentionally preserve exact status-code behavior. |

## Pass 1 Foundation Ownership Baseline

`LifecycleSystem/FOUNDATION_OWNERSHIP_INVENTORY.md` is the current classified ownership contract. Shutdown follow-up work must compare against `LifecycleSystem/FOUNDATION_OWNERSHIP_SCAN.md` / `.json` before moving or removing quit, durable-state, proof-exit, or gameplay-world cleanup paths.

Pass 1 found:

- 9 existing participant registrations, plus the shutdown API rows.
- 1 player-facing `RequestQuitGame` call in the quit modal.
- 1 direct `UKismetSystemLibrary::QuitGame` fallback in the quit modal; resolved in Pass 6 as a logged `defensive-fallback` for missing-subsystem states only.
- 14 direct durable save calls total: 10 sync and 4 async.
- 43 direct `RequestExitWithStatus` rows total: 2 shutdown final exits, 31 proof/automation exits, and 10 other direct status exits. Proof and fatal exits are not player quit paths by default.

Current scanner counts may be higher than the Pass 1 historical rows as new proof/status harnesses are added. Foundation Pass 2/3 intentionally left proof/status exits untouched, and Pass 3 classifies raw proof travel as a world-transition exception rather than a player-quit path. Preserving exact automation status-code and proof-travel contracts remains the shutdown rule unless a proof-specific task requests player-quit shutdown.

## Pass 4 Durable-State Boundary

Pass 4 wires only the async durable writers that can lose queued work during player quit:

- `UT66SaveSubsystem` registers `Save.RunSlotAndIndex` in `DurableState`, tracks the latest async run-slot and save-index writes, and performs a final synchronous flush during shutdown or `Deinitialize` when a queued write is still pending.
- `UT66BuffSubsystem` registers `Buff.ProgressionSave` in `DurableState`, tracks the latest async buff-progression write, and performs a final synchronous flush during shutdown or `Deinitialize` when a queued write is still pending.

The synchronous durable writers are classified as `native-unreal` because they persist at the mutation/update point and do not leave background save work for quit. Keep run-summary and proof-summary save callers out of player-quit shutdown unless a future proof-specific task explicitly changes that contract.

The async/sync overlap during quit is intentional: an async write may still complete after the final synchronous flush, but both writes serialize the same owner object to the same slot, so ordering remains consistent. Pass 4 inspection found no owner-local dirty flag or delayed local-save ticker in the sync-save owners; leaderboard `Pending*` fields are backend/UI request state that resolves through existing sync save call sites, not an unflushed local durable buffer.

## Pass 4.1 Save-Integrity Proof Harness

`UT66SaveSubsystem` owns a non-shipping integrity harness for the run-slot/index durable shutdown path:

- `T66.Save.QueueIntegrityShutdown <slot 0-8> <marker> CONFIRM [exitCode]`
- `T66.Save.VerifyIntegritySlot <slot 0-8> <marker> CONFIRM [exitCode]`

The queue command writes a real run save through `SaveToSlot`, which also queues the save-index write, then runs shutdown participants with `ET66ShutdownReason::TestHarness`. The harness checks that `Save.RunSlotAndIndex` cleared the pending run/index state, that the slot can be loaded immediately, and that the save index reports the same marker metadata. The reload command is a separate-process check that the flushed slot and index can be read after process exit.

This is proof tooling only. It does not add a new shutdown participant, does not change runtime save policy, and does not make synchronous mutation-time save owners part of `DurableState`.

Use `Scripts/RunDurableSaveIntegritySmokeGate.ps1` for repeatable pre-release proof. The wrapper backs up and restores the selected staged `T66_Slot_XX.sav` and `T66_SaveIndex.sav` files, runs both harness commands in separate processes, and treats missing PASS markers as a build/configuration or runtime proof failure.
