# T66 Shutdown System

The Shutdown System owns the intentional pre-exit path for T66. It gives runtime systems a deterministic chance to stop external work before Unreal reaches late object destruction.

For broader run/world coordination, start with `LifecycleSystem/`. Lifecycle owns run begin/end, loaded-run, world-transition, durable-state flush planning, and world-runtime teardown audits. Shutdown remains the owner of player-facing Quit Game and pre-exit cleanup.

For the subsystem-oriented map of startup owner, shutdown owner, lifecycle/reset owner, durable-state owner, and health-gate coverage, use `PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md`. That inventory cross-links this registry; it does not replace shutdown ownership.

## Purpose

The system coordinates shutdown. It does not replace Unreal object lifetime, garbage collection, actor `EndPlay`, or subsystem `Deinitialize`.

Use it for resources that can hang, leak, keep callbacks alive, or write after teardown if they are left to late destructors:

- native windows, COM objects, DLL-backed resources, and platform handles
- worker threads and blocking queues
- HTTP requests, backend polling, Steam/session delegates, and auth tickets
- async saves and durable profile state
- async asset loads and caches
- `FTSTicker` handles and world timers
- media players, WebView panels, audio components, and UI presentation queues
- gameplay managers and object pools that keep ticking or spawning work

## Runtime Owner

Runtime code lives under:

- `Source/T66/Core/Shutdown/`

The primary entry point is `UT66ShutdownSubsystem::RequestQuitGame`. Player-facing quit UI should call this instead of calling `UKismetSystemLibrary::QuitGame` directly.

`UT66QuitConfirmationModal` keeps one logged defensive `UKismetSystemLibrary::QuitGame` backstop only for the abnormal case where `UT66ShutdownSubsystem` is unavailable. Treat that warning log as a shutdown initialization defect to investigate; it is not the normal player quit path.

The subsystem runs registered participants in ordered phases, logs timing, then requests engine exit through `FPlatformMisc::RequestExitWithStatus`.

For packaged development smoke tests, use `T66.Shutdown.RequestQuit` through `-ExecCmds` instead of raw `quit`. That console hook is excluded from shipping and routes through `UT66ShutdownSubsystem::RequestQuitGame`, so participant ordering is actually exercised.

For player-facing Quit Game proof, use the frontend tag-click automation path instead of OS mouse injection:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 `
  -Screen MainMenu `
  -Modal QuitConfirmation `
  -ClickTag "QuitConfirmation.QuitButton" `
  -ClickDelaySeconds 2.5 `
  -WaitForExit
```

The accepted log gate is a tagged Slate click followed by `UT66ShutdownSubsystem::RequestQuitGame` / shutdown participant logs and no quit-confirmation fallback warning.

For save-integrity development proof, use the save-owner harness rather than adding a separate durable coordinator:

- `T66.Save.QueueIntegrityShutdown <slot 0-8> <marker> CONFIRM [exitCode]`
- `T66.Save.VerifyIntegritySlot <slot 0-8> <marker> CONFIRM [exitCode]`

For repeatable pre-release evidence, use the script wrapper instead of manually composing both process runs:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunDurableSaveIntegritySmokeGate.ps1 `
  -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

For full pre-release smoke, use `Scripts/RunPreReleaseSmokeSuite.ps1`; it includes this durable save gate between frontend interaction proof and lifecycle transition proof.

Both hooks are excluded from shipping. The first command creates a normal `UT66RunSaveGame`, queues the usual async slot/index save, runs the shutdown participants through `ET66ShutdownReason::TestHarness`, verifies the immediate disk load and index metadata, then exits. The second command reloads the same slot/index marker in a fresh process. Always back up and restore the chosen staged `SaveGames` files around this proof command; it intentionally writes the selected slot.

`RunDurableSaveIntegritySmokeGate.ps1` performs that backup/restore automatically for the selected `T66_Slot_XX.sav` and `T66_SaveIndex.sav` files, asserts `[SaveIntegrity] PASS` plus `[SaveIntegrityReload] PASS`, and writes a summary under `Saved/DurableSaveIntegritySmokeGate/`. Slot `8` is the default reserved test slot for the wrapper; it is still written during the proof, then restored from the backup snapshot.

Current `DurableState` participants are intentionally narrow:

- `UT66SaveSubsystem` protects pending async run-slot and save-index writes.
- `UT66BuffSubsystem` protects pending async buff-progression writes.

Do not register synchronous mutation-time save owners just to make the table look complete. If a save owner persists synchronously at the mutation point, document it as `native-unreal` in `SHUTDOWN_REGISTRY.md` unless a future audit proves deferred dirty state.

## Phase Order

1. `InputLock` - block repeat quit requests and new work.
2. `NativeExternal` - WebView2, COM, native windows, DLL-backed resources.
3. `NetworkPlatform` - HTTP, backend polling, Steam/session delegates, auth tickets.
4. `DurableState` - saves, profiles, settings, run summaries.
5. `AsyncWork` - streamable handles, image and UI caches, pending async loads.
6. `RuntimeTick` - tickers, timers, worker queues, diagnostics.
7. `MediaAudio` - media players, music, audio components, UI presentation queues.
8. `GameplayWorld` - gameplay pools, managers, projectile/traveler/hazard owners.
9. `FinalReport` - shutdown summary and exit request.

## Contract

Every participant must be:

- idempotent
- safe after partial initialization
- safe if normal `Deinitialize` later runs again
- bounded; no participant may wait forever
- explicit about whether failure is required or best-effort

If a new feature owns an external resource or starts async work, add it to `SHUTDOWN_REGISTRY.md` during implementation.
