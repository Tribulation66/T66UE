# Lifecycle Coordinator Registry

This registry tracks lifecycle boundaries that need explicit ownership. It is not a replacement for `ShutdownSystem/SHUTDOWN_REGISTRY.md`.

Status values:

- `implemented` - runtime boundary exists and is usable
- `documented` - owner/policy is documented, but no new runtime code exists
- `planned` - should be implemented in a future pass
- `audit-first` - do not implement before the audit matrix has evidence
- `deferred` - intentionally out of scope for the current lifecycle slice

| Boundary | Current Owner | Runtime Surface | Status | Notes |
|---|---|---|---|---|
| Player quit / pre-exit shutdown | `ShutdownSystem` / `UT66ShutdownSubsystem` | `Source/T66/Core/Shutdown/` | implemented | Quit Game paths route through shutdown. Lifecycle cross-links this owner but does not replace it. |
| New run begin | `UT66RunStateSubsystem` | `BeginNewRun`, compatibility `ResetForNewRun` | implemented | Lightweight owner-local boundary. External C++ fresh-run callers were re-pointed in Pass 2/3; remaining reset-named call sites are child-owner resets inside RunState or proof-specific DamageLog reset. |
| Loaded run begin | `UT66RunStateSubsystem` | `BeginLoadedRun`, compatibility `ImportSavedRunSnapshot` | implemented | Restores saved run state through the existing snapshot import path with lifecycle events/logging. External C++ loaded-run caller is now canonical. |
| Run end | `UT66RunStateSubsystem` | `EndRun`, compatibility `MarkRunEnded` | implemented | Keeps existing final-time and victory/death semantics. External C++ run-end callers were re-pointed to `EndRun`; duplicate end requests remain idempotent. |
| Return run to frontend | `UT66RunStateSubsystem` | `ReturnRunToFrontend` | implemented | Notification-only boundary for future UI/session callers. Current frontend travel remains unchanged. |
| World transition | `UT66GameInstance` / `UT66SessionSubsystem` and callers | `TransitionToGameplayLevel`, `TransitionToFrontendLevel`, session travel, raw `OpenLevel` exception sites | implemented | Owner-local gameplay/frontend wrappers exist. The loaded-run fast-resume bypasses were resolved by routing local SaveSlots load/preview entries through `TransitionToGameplayLevel` after snapshot prep. Remaining raw travel exceptions are wrapper internals, null-GI gameplay fallbacks, session/current-map dynamic travel, and proof fallbacks. Session travel remains owned by `UT66SessionSubsystem` to preserve `ServerTravel`, `ClientTravel`, and listen-server semantics. Session-owned loaded-save travel-plan proof is available through non-shipping `T66.Session.QueueLoadedTravelSeed` / `T66.Session.VerifyLoadedTravelPlan` and `Scripts/RunSessionLoadedTravelSmoke.ps1`; it intentionally skips live `ServerTravel` and remote-client join. |
| Durable state flush | `ShutdownSystem` plus owner-local durable writers | `UT66SaveSubsystem`, `UT66BuffSubsystem`, achievements, leaderboard, settings, community content, companion unlocks | implemented | Async run/index saves and buff saves are registered in shutdown `DurableState`; sync durable owners are documented as native-safe; no separate durable coordinator exists yet. |
| Durable save integrity proof | `UT66SaveSubsystem` / `ShutdownSystem` docs | non-shipping `T66.Save.QueueIntegrityShutdown`, `T66.Save.VerifyIntegritySlot`; wrapper `Scripts/RunDurableSaveIntegritySmokeGate.ps1` | implemented | Proof harness only. Verifies queued run-slot/index save flush through shutdown without broadening runtime save policy or adding a durable coordinator. The wrapper is the pre-release gate and restores protected staged save files after proof. |
| World-runtime drain | World subsystem owners | projectile, mob, hazard, traveler, loot, trap, registry, visual systems; non-shipping `T66.WorldRuntime.Snapshot` / `T66.WorldRuntime.ProofTravel` proof commands | audit-first | Pass 5 classified weak-ref/tick-only/simple world systems as native-safe, `UT66FloatingCombatTextPoolSubsystem` as owner-local self-contained, and render/Niagara/pool systems as owner-local proof candidates. Pass 5.1 added observer-only proof tooling and owner-local snapshots; no coordinator yet. Use `WORLD_RUNTIME_TEARDOWN_AUDIT.md` evidence and the reusable `Scripts/RunLifecycleTransitionSmokeGate.ps1` manifest gate before adding runtime drain code. |

## Pass 1 Foundation Ownership Baseline

`FOUNDATION_OWNERSHIP_INVENTORY.md` is the current ownership contract for later infrastructure passes. It is backed by the reusable scanner `Scripts/Invoke-T66FoundationInventoryScan.ps1` and generated evidence in `FOUNDATION_OWNERSHIP_SCAN.md` / `FOUNDATION_OWNERSHIP_SCAN.json`.

`PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md` is the subsystem-oriented health-coverage map. Use it to decide whether a missing proof belongs in the runtime health gate, lifecycle transition smoke, durable save smoke, shutdown proof, frontend smoke, or owner-local docs.

That inventory reuses this registry's lifecycle status vocabulary and adds a separate health-gate coverage vocabulary (`covered`, `partial`, `none-owner-local`, `routed`, `n/a`). Treat those as coverage labels only, not lifecycle coordinator status values.

Current classified surface counts:

| Surface | Classified Count | Owning Follow-Up |
|---|---:|---|
| Raw `OpenLevel` travel call sites | 15 | Pass 3 classified complete; loaded-save local resume behavior resolved; remaining future work is null-GI fallback policy or real two-peer session travel automation, not mechanical cleanup |
| `ResetForNewRun` call sites | 7 | Owner-local child resets / proof-specific exceptions; no external RunState compatibility caller remains |
| Direct sync durable saves | 13 | Pass 4 durable-state flush / owner-local classification |
| Direct async durable saves | 4 | Pass 4 durable-state flush |
| Shutdown participant registrations | 11 | Pass 6 shutdown completion |
| Direct quit fallback | 1 | Pass 6 quit hardening |
| Direct status-code exits | 52 total, classified as shutdown final, proof/automation, or direct status | Pass 6 / proof exceptions |
| World cleanup hooks | Broad candidate set; not leak proof by itself | Pass 5 teardown audit |

## Graduation Path

Start with owner-local lifecycle methods and delegates. Graduate to a dedicated coordinator only when at least one of these is true:

- multiple systems require a deterministic order
- a boundary needs bounded timeout/flush diagnostics
- multiple callers are duplicating the same lifecycle sequence
- GameInstance and World lifetimes must be reconciled before travel

Do not create shared lifecycle base classes until at least two concrete coordinators have repeated enough code to justify that abstraction.
