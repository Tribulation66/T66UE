# Section 7 - Profiling Artifacts

## Existing Profiling Artifacts

No files were found for:

- `.utrace`
- `.ue4stats`
- `.uestats`
- `Saved/Profiling`
- `Saved/UnrealInsights`
- `Saved/Trace`
- `Saved/Traces`
- MemReport files

Existing artifacts are mostly:

- Screenshots under `Saved/Screenshots`
- Standalone logs under `Saved/StandaloneLogs`
- Visual audit screenshots under `Audit/Reference`
- Material stats under `Saved/MaterialStats`
- Cooked shader metadata under `Saved/Cooked/Windows/T66/Metadata`

## Material Stats

Material stats CSVs exist under:

- `Saved/MaterialStats`

Latest sampled file:

- `Stats-2026.05.18-17.56.02.csv`

These provide material compile/cooked counts, not live GPU pass timings.

## Trace Flags in Launch Scripts

No script hardcodes:

- `-trace=cpu,gpu,frame,bookmark`
- `stat startfile`
- `stat unit`
- `ProfileGPU`

Capture scripts expose `ExtraArgs`, so trace flags can be passed manually.

## In-Code Profiling

Unreal macro usage is sparse.

Found:

- `RETURN_QUICK_DECLARE_CYCLE_STAT` in `UT66ToonOutlineViewSubsystem`.

Not found in source:

- `SCOPE_CYCLE_COUNTER`
- `TRACE_CPUPROFILER_EVENT_SCOPE`
- `QUICK_SCOPE_CYCLE_COUNTER`

Custom lag tracker:

- `UT66LagTrackerSubsystem`
- `FLagScopedScope`
- console command `T66.Perf.Dump`
- hitch/slow-scope logging controls via CVars

Major `FLagScopedScope` locations include:

- UI manager screen/modal show paths.
- Frontend top bar actions.
- HUD refresh and map refresh.
- Backend/session party/join calls.
- Enemy tick.
- Hero tick.
- Companion tick ground trace.
- Combat fire.
- Enemy runtime wave spawn.
- Miasma manager update.

## Required Baseline Capture Plan

Before remediation planning, capture:

- `stat unit`
- `stat game`
- `stat gpu`
- `stat rhi`
- `stat scenerendering`
- `stat slate`
- `stat memory`
- `stat gc`
- Unreal Insights with `-trace=cpu,gpu,frame,bookmark`
- `ProfileGPU`
- `T66.Perf.Dump`

Recommended scenarios:

- Frontend idle.
- Settings/hero selection UI.
- Gameplay level just loaded.
- Normal combat with representative enemy count.
- Heavy VFX/boss/trap encounter.
- Stage/floor transition.
- Minigame battle screen.

