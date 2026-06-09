# Runtime Health Gate

`Scripts/RunRuntimeHealthGate.ps1` is the reusable startup/runtime health coordinator for the staged packaged game. It is an infrastructure gate, not a gameplay feature and not an optimizer pass.

Use it when a change could affect packaged startup, core runtime diagnostics, required data/config availability, or PerformanceSystem report emission:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunRuntimeHealthGate.ps1
```

The default gate is intentionally broader than a one-off quit smoke but cheaper than a full staging/smoke release pass. It:

1. Checks required project files, reusable gate scripts, PerformanceSystem docs, schema files, and high-value source DataTable/cooked DataTable pairs.
2. Checks that `PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md` exists and is linked from the owning routers.
3. Runs `RunStagedBuildReadinessGate.ps1 -SkipStage -SkipSmoke` so the target staged executable and shortcut contract are still checked without rebuilding or running the full pre-release suite.
4. Launches the staged packaged executable into the MainMenu with a delayed Unreal screenshot request. The delay keeps the process alive long enough for startup diagnostics to write useful PerformanceSystem evidence before the frontend automation exits.
5. Confirms the executable hash is unchanged during the gate so diagnostics are tied to the same staged binary.
6. Requires exit code `0`, `LogExit: Exiting`, no fatal log markers, a fresh `snapshot.current.json`, a fresh `session_summary.json`, current `SchemaVersion`, required session fields, a clean PerformanceSystem write queue, `events.jsonl`, and `session_summary.md`.

On some Windows GUI-process launches, PowerShell may not expose the process exit code even after the game exits cleanly. In that case, the gate accepts `FPlatformMisc::RequestExitWithStatus(0, 0, UGameEngine::HandleExitCommand)` in the Unreal log as fallback exit-status evidence, but only alongside the normal clean-exit log marker, fatal-marker scan, fresh session summary, and clean PerformanceSystem write queue.

## Output

Each run writes:

- `Saved/RuntimeHealthGate/<timestamp>/summary.json`
- `Saved/RuntimeHealthGate/<timestamp>/summary.md`
- `Saved/RuntimeHealthGate/<timestamp>/staged_readiness/summary.json`
- `Saved/RuntimeHealthGate/<timestamp>/runtime_launch/runtime.log`
- `Saved/RuntimeHealthGate/<timestamp>/runtime_launch/health_mainmenu.png`

The summary status is `PASS`, `WARN`, `FAIL`, or `PRINT_ONLY`. Any failed check makes the script exit nonzero.

## Extension Rules

- Add checks here when they represent shared runtime-health contracts: required startup files, diagnostics schemas, staged executable provenance, packaged launch/exit health, or final-report completeness.
- Keep `PerformanceSystem/RUNTIME_OWNERSHIP_INVENTORY.md` as the map for deciding whether a new check belongs here or should be routed to lifecycle, shutdown, durable-save, or frontend smoke gates.
- Do not add optimizer fixes to this gate. If the gate finds a performance or memory issue, record the failure evidence and fix it in the owning runtime system.
- Do not replace the full staged readiness or pre-release smoke suites with this gate. This gate composes the cheap staged-readiness mode and adds runtime diagnostics proof; the release gate remains authoritative before shipping.
- Prefer owner-local checks first. Add a new coordinator only when ordering, durable flushing, runtime draining, or shared diagnostics need cross-system sequencing.
- If a future system writes required runtime sidecars, give it a versioned schema and make this gate assert freshness, schema version, and clean writer state rather than only checking that a file exists.

## Common Commands

Dry-run command and static plan:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunRuntimeHealthGate.ps1 -PrintOnly
```

Run only static and staged executable readiness checks:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunRuntimeHealthGate.ps1 -SkipRuntimeLaunch
```

Run only packaged runtime launch and PerformanceSystem artifact checks:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\RunRuntimeHealthGate.ps1 -SkipStaticChecks -SkipStagedReadiness
```
