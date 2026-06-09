# Lifecycle System Agents

## Owns

Project-level lifecycle coordination policy, run lifecycle boundaries, world-transition planning, durable-state flush planning, and world-runtime drain/teardown audits.

Runtime code remains with the system that owns the state or resource. Lifecycle docs coordinate ownership; they do not automatically centralize all implementation.

## Trigger Words

Lifecycle, coordinator, overseer, run lifecycle, start run, new run, loaded run, end run, return to frontend, world transition, level travel, durable state, save flush, teardown audit, runtime drain, memory leak prevention, ordered shutdown, coordinating systems.

## Read First

- `LifecycleSystem/README.md`
- `LifecycleSystem/LIFECYCLE_COORDINATOR_REGISTRY.md`
- `LifecycleSystem/WORLD_RUNTIME_TEARDOWN_AUDIT.md`
- `PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md` when the task needs subsystem ownership mapped to health-gate coverage
- `ShutdownSystem/README.md`
- `ShutdownSystem/SHUTDOWN_REGISTRY.md`
- Root `AGENTS.md`
- The folder router for the owner being changed, such as `Gameplay/GAMEPLAY_AGENTS.md`, `UI/UI_AGENTS.md`, `Backend/BACKEND_AGENTS.md`, `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`, or `ShutdownSystem/SHUTDOWN_SYSTEM_AGENTS.md`.

## Hard Rules

- Do not add a global lifecycle base class until repeated concrete implementations prove it removes real duplication.
- Do not build a coordinator when one owner-local method or delegate is enough.
- Do not move shutdown ownership into lifecycle. Quit/pre-exit behavior stays under `ShutdownSystem/`.
- Do not route smoke/proof `RequestExitWithStatus` calls through player quit shutdown unless a test contract explicitly requires it.
- Separate GameInstance-scoped persistent state from World-scoped runtime state in every plan.
- New lifecycle boundaries must be idempotent and safe after partial initialization.
- Before proposing a `UT66WorldRuntimeCoordinatorSubsystem`, use or reference the observer-only `T66.WorldRuntime.Snapshot` / `T66.WorldRuntime.ProofTravel` harness and `WORLD_RUNTIME_TEARDOWN_AUDIT.md` evidence.
- New runtime lifecycle code requires focused compile verification. If a playable standalone path changes, refresh staged standalone and verify the shortcut target.
- For pre-release smoke, prefer `Scripts/RunPreReleaseSmokeSuite.ps1`. For focused lifecycle-only proof, use `Scripts/RunLifecycleTransitionSmokeGate.ps1` against a Development/non-shipping executable and pair it with `Scripts/RunFrontendTagClickSmokeMatrix.ps1` for frontend interaction proof.
- If a pass discovers an out-of-scope leak or ordering problem, record it in the nearest `pending_issues_<folder>.md`.

## Verification

Minimum evidence depends on scope:

- Documentation or audit only: file references plus Codex/Claude review.
- Runtime C++ boundary changes: focused compile.
- Playable flow changes: focused compile plus staged standalone validation.
- Quit path changes: staged standalone refresh, shortcut target verification, and quit smoke/log evidence.
- Pre-release smoke: `Scripts/RunPreReleaseSmokeSuite.ps1`; focused proof remains frontend tag-click matrix plus lifecycle transition smoke gate when the target build is Development/non-shipping.
