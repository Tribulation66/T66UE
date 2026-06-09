# T66 Lifecycle System

The Lifecycle System is the project-level home for coordination points that must happen in a deliberate order across multiple runtime systems.

It exists because some game actions are not owned by one component:

- starting a new run
- loading a saved run
- ending a run
- transitioning between frontend and gameplay worlds
- preparing durable state before travel or quit
- draining world-runtime pools and managers before a world boundary

## Relationship To Shutdown

`ShutdownSystem` remains the owner of intentional game quit and pre-exit cleanup. Player-facing Quit Game paths must still route through `UT66ShutdownSubsystem::RequestQuitGame`.

Lifecycle owns broader game-state boundaries. It can document or call into shutdown-owned work, but it must not silently replace the shutdown registry or turn shutdown into a general object garbage collector.

Use this rule of thumb:

- **Quit / close game / pre-exit resource cleanup**: start with `ShutdownSystem/`.
- **Run begin/end, loaded run, world travel, durable-state flush, or world-runtime drain**: start with `LifecycleSystem/`.

## Current Implementation

The first implemented lifecycle boundary is lightweight and lives in the existing owner:

- `UT66RunStateSubsystem::BeginNewRun`
- `UT66RunStateSubsystem::BeginLoadedRun`
- `UT66RunStateSubsystem::EndRun`
- `UT66RunStateSubsystem::ReturnRunToFrontend` (notification-only for now; frontend travel callers are not migrated yet)
- `UT66GameInstance::TransitionToFrontendLevel` (thin owner-local wrapper for behavior-equivalent frontend map travel)
- `UT66GameInstance::TransitionToGameplayLevel`
- `UT66SaveSubsystem` / `UT66BuffSubsystem` owner-local durable-state shutdown participants for pending async save flushes

The existing compatibility entry points remain:

- `UT66RunStateSubsystem::ResetForNewRun`
- `UT66RunStateSubsystem::ImportSavedRunSnapshot`
- `UT66RunStateSubsystem::MarkRunEnded`

Those compatibility methods now route through the lifecycle boundary methods so current callers get lifecycle logging and notifications without a broad call-site migration.

## Registry

Use `LIFECYCLE_COORDINATOR_REGISTRY.md` to record all lifecycle boundaries, current owners, status, and future graduation paths.

Use `FOUNDATION_OWNERSHIP_INVENTORY.md` as the Pass 1 baseline for classified ownership of travel, run reset, durable save, shutdown, proof-exit, and world-runtime cleanup surfaces. The paired generated scan files are `FOUNDATION_OWNERSHIP_SCAN.md` and `FOUNDATION_OWNERSHIP_SCAN.json`.

Use `PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md` when a task needs the subsystem-oriented map of startup owner, shutdown owner, lifecycle/reset owner, durable-state owner, and health-gate coverage. That inventory cross-links the lifecycle and shutdown registries; it does not replace either one.

For durable-state shutdown ownership, use `ShutdownSystem/SHUTDOWN_REGISTRY.md` as the source of truth. The current rule is: async durable writers that can survive past the caller register with shutdown, while synchronous mutation-time saves stay with their native owner until a dirty/deferred write is proven.

Use `WORLD_RUNTIME_TEARDOWN_AUDIT.md` before adding a world-runtime coordinator. A coordinator should be introduced only when multiple systems need ordered draining, bounded flush behavior, or shared diagnostics.

For world-runtime teardown proof, use the development-only console harness before proposing a coordinator:

- `T66.WorldRuntime.Snapshot Path=<Saved/.../manifest.json> Label=<label> [ExitOnComplete=0|1] [ExitCode=0]`
- `T66.WorldRuntime.ProofTravel Path=<Saved/.../manifest.json> Travels=<count> Delay=<seconds> [Stress=0|1] [StressCount=<count>] [StressSettle=<seconds>] [ExitOnComplete=0|1] [ExitCode=0]`

The harness lives in `Source/T66/Core/T66WorldRuntimeProofCommands.cpp` and writes JSON manifests under `Saved/WorldRuntimeProof/` by default. It is observer-only: it collects owner-local counts and global world resource counts, but it does not drain, destroy, or coordinate gameplay systems.

Use `Stress=1` when the proof needs active resources before travel. The stress path enters gameplay if needed, creates bounded live mobs, mob loot, managed projectiles, boss hazards, outgoing travelers, and pixel VFX through existing owner APIs, captures `before_travel_stress`, then runs the normal repeated travel sequence.

## Reusable Smoke Gate

For full pre-release smoke, use the suite wrapper:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunPreReleaseSmokeSuite.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

For focused lifecycle-only evidence, use the lifecycle wrapper instead of manually composing the console command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunLifecycleTransitionSmokeGate.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

The wrapper validates exit code `0`, manifest `status=complete`, requested/completed travel count parity, zero `non_current_world_proof_candidate_resource_count` in every snapshot, all six expected candidate subsystems present, and active stress resource population when stress is enabled. This gate is Development/non-shipping only because the underlying proof command is compiled out of Shipping builds.

Pair this lifecycle gate with `Scripts/RunFrontendTagClickSmokeMatrix.ps1` for pre-release frontend interaction smoke. The frontend matrix proves packaged UI navigation and modal behavior; this lifecycle gate proves structural world-runtime transition cleanup. Neither one replaces ShutdownSystem quit proof.

Pair durable-state release checks with `Scripts/RunDurableSaveIntegritySmokeGate.ps1`. That gate remains shutdown/save-owned evidence: it exercises `T66.Save.QueueIntegrityShutdown` plus `T66.Save.VerifyIntegritySlot`, validates the save integrity PASS markers, and restores protected staged save files after proof.

## Design Rules

- Prefer the existing owner first.
- Add a dedicated coordinator only when ordering across systems is proven.
- Keep GameInstance lifetime and World lifetime separate.
- Keep all cleanup idempotent.
- Keep automation/proof exits and status-code contracts intact.
- Cross-link with `ShutdownSystem/SHUTDOWN_REGISTRY.md` instead of duplicating shutdown ownership.
