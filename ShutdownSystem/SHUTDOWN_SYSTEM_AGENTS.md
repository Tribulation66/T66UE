# Shutdown System Agents

## Owns

T66 game quit, pre-exit cleanup, shutdown participant registration, shutdown phase ordering, shutdown timeout diagnostics, and resource-lifetime policy for external or async runtime work.

Runtime source lives under `Source/T66/Core/Shutdown/`. Human-readable contracts and inventories live under `ShutdownSystem/`.

## Trigger Words

Shutdown, quit, QuitGame, close game, exit, freeze on quit, teardown, cleanup, memory leak, external resource, WebView2 shutdown, COM shutdown, async save flush, pending HTTP, Steam ticket, worker thread shutdown, ticker cleanup, timer cleanup.

## Read First

- `ShutdownSystem/README.md`
- `ShutdownSystem/SHUTDOWN_REGISTRY.md`
- `ShutdownSystem/SHUTDOWN_IMPLEMENTATION_INSTRUCTIONS.md`
- `LifecycleSystem/README.md` when the request is about run/world lifecycle, durable-state flush, or general teardown architecture rather than only player-facing quit
- Root `AGENTS.md`
- The folder router for the participant owner being changed, such as `UI/UI_AGENTS.md`, `Backend/BACKEND_AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, or `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.

## Hard Rules

- Do not turn the shutdown subsystem into a manual garbage collector. Unreal still owns UObject lifetimes.
- Participant cleanup must stay with the system that owns the resource.
- Player-facing quit must route through `UT66ShutdownSubsystem::RequestQuitGame`.
- Broader run/world lifecycle coordination belongs in `LifecycleSystem/`; do not move quit/pre-exit ownership out of `ShutdownSystem`.
- Smoke/proof harness `RequestExitWithStatus` calls are not automatically player quit paths. Route them only when a specific test contract requires full shutdown participation.
- New native handles, worker threads, HTTP/network requests, Steam/session delegates, async saves, async asset loads, tickers, timers, media players, audio components, or persistent external state must either register with the shutdown system or document why normal Unreal teardown is enough.
- No shutdown participant may perform an unbounded wait.

## Verification

For runtime changes, run a focused compile. If the change affects the playable standalone quit path, refresh the staged standalone build and verify the shortcut target.

When practical, validate shutdown by opening the relevant feature surface, pressing Quit, and checking logs for:

- shutdown begin
- participant start/finish
- participant duration
- timeout or failure markers
- final exit request
