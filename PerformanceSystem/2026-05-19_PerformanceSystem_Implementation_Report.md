# T66 PerformanceSystem Implementation Report

Date: 2026-05-19  
Audience: Claude / next planning agent  
Repo: `C:\UE\T66`

## Summary

Implemented the first additive `PerformanceSystem` pass for T66. The system now has a clear root folder, repo instructions, versioned schemas, Unreal runtime code, configurable thresholds, runtime JSON/Markdown output, and a bridge from the existing T66 lag tracker into structured diagnostics events.

This was not an optimization pass. No gameplay performance fixes were attempted. The work establishes the diagnostic framework and first cheap detectors so later optimization work has persistent evidence.

## Naming And Folder Layout

Root system folder:

- `PerformanceSystem/`

Runtime source folder:

- `Source/T66/PerformanceSystem/`

Runtime output folder:

- `Saved/PerformanceSystem/` in normal project/editor runs.
- `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/` in staged standalone runs.

The root `AGENTS.md` now includes a `PerformanceSystem Rule` telling future agents to start with:

- `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`

## Files Added

Documentation and schema:

- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`
- `PerformanceSystem/README.md`
- `PerformanceSystem/schema/performance_event.schema.v1.json`
- `PerformanceSystem/schema/performance_session_report.schema.v1.json`
- `PerformanceSystem/schema/sidecar_message.schema.v1.json`
- `PerformanceSystem/2026-05-19_PerformanceSystem_Implementation_Report.md`

Runtime source:

- `Source/T66/PerformanceSystem/T66PerformanceSubsystem.h`
- `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp`
- `Source/T66/PerformanceSystem/T66PerformanceSystemSettings.h`
- `Source/T66/PerformanceSystem/T66PerformanceSystemTypes.h`
- `Source/T66/PerformanceSystem/pending_issues_PerformanceSystem.md`

## Files Modified

- `AGENTS.md`
  - Added `PerformanceSystem Rule`.

- `Source/T66/T66.Build.cs`
  - Added `PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "PerformanceSystem"));`
  - Note: this file already had an unrelated staged `RenderCore` dependency change from previous work. This pass did not remove or rewrite it.

- `Source/T66/Core/T66LagTrackerSubsystem.cpp`
  - Included `Engine/GameInstance.h`.
  - Included `PerformanceSystem/T66PerformanceSubsystem.h`.
  - Forwards existing recorded lag operations into `UT66PerformanceSubsystem::RecordMeasuredOperation(...)`.

## Runtime Capabilities Implemented

`UT66PerformanceSubsystem` is a `UGameInstanceSubsystem` that initializes automatically when enabled.

Implemented first-pass behavior:

- Creates a unique session ID and session directory.
- Writes structured events to `events.jsonl`.
- Writes `snapshot.current.json` and rotates `snapshot.previous.json`.
- Writes `session_summary.json` and `session_summary.md` on clean subsystem deinitialize.
- Tracks rolling frame samples.
- Tracks rolling memory samples.
- Captures a bounded last-N log ring through a custom `FOutputDevice`.
- Redacts user-profile paths from captured log/report strings.
- Emits a `SessionStarted` event.
- Emits structured events with:
  - `SchemaVersion`
  - `EventId`
  - `SessionId`
  - UTC wall-clock timestamp
  - game time
  - detector name
  - event name
  - severity
  - confidence
  - world/map
  - build metadata
  - frame summary
  - memory summary
  - confidence-tagged attributions

## Detectors Implemented

Shipping-safe first wave:

- `FramePacingDetector`
  - Single-frame hitch detection.
  - Sustained low FPS detection.
  - Frame-time variance / stutter detection.
  - Rolling 1% low and 0.1% low frame-time summaries.

- `MemoryGrowthDetector`
  - Physical memory growth slope detection using `FPlatformMemory::GetStats()`.

- `GCPauseDetector`
  - GC pause spike detection using:
    - `FCoreUObjectDelegates::GetPreGarbageCollectDelegate()`
    - `FCoreUObjectDelegates::GetPostGarbageCollect()`

- `BasicHangDetector`
  - Best-effort large frame-delta detection.
  - Explicitly caveated as in-engine only, not reliable external hang detection.

- `ProjectOperationStallDetector`
  - Receives existing T66 lag tracker operation records.
  - Emits structured events when an operation exceeds `ProjectOperationWarningMs`.

- `PerformanceSystemOverhead`
  - Tracks detector/framework self-cost.
  - Emits diagnostics when detectors exceed budget repeatedly.
  - Degrades detector cadence, then disables noisy detectors with a diagnostic event.

## Settings Implemented

`UT66PerformanceSystemSettings` lives in:

- `Source/T66/PerformanceSystem/T66PerformanceSystemSettings.h`

Important defaults:

- `bEnablePerformanceSystem = true`
- `SnapshotCadenceSeconds = 5.0`
- `FrameWindowSeconds = 60.0`
- `MaxCapturedLogLines = 200`
- `HitchThresholdMs = 50.0`
- `SustainedLowFpsThreshold = 50.0`
- `SustainedLowFpsWindowSeconds = 10.0`
- `StutterStdDevThresholdMs = 8.0`
- `MemorySlopeWindowSeconds = 300.0`
- `MemoryGrowthWarningMbPerMinute = 128.0`
- `GCPauseWarningMs = 12.0`
- `BasicHangFrameDeltaSeconds = 8.0`
- `ProjectOperationWarningMs = 25.0`
- `DetectorBudgetUs = 200.0`
- `FrameworkFrameBudgetUs = 500.0`
- `bIncludeHardwareFingerprintInDevelopment = true`
- `bIncludeHardwareFingerprintInShipping = false`
- `DevelopmentDirectoryBudgetMb = 256`
- `DevelopmentSessionBudgetMb = 25`
- `ShippingDirectoryBudgetMb = 64`
- `ShippingSessionBudgetMb = 10`

## Privacy Behavior

Implemented:

- Hardware fingerprint defaults on in Development.
- Hardware fingerprint defaults off in Shipping.
- Captured paths are redacted at user-profile boundaries.
- Reports are local-only.
- No upload/submission mechanism was added.

Current hardware fingerprint fields when enabled:

- CPU brand
- primary GPU brand
- total physical memory MB

## Proton / Steam Deck Behavior

Implemented best-effort Proton status detection through environment variables:

- `SteamDeck`
- `SteamOS`
- `WINEPREFIX`
- `PROTON_LOG`
- `STEAM_COMPAT_CLIENT_INSTALL_PATH`

Reported values:

- `Detected`
- `Likely`
- `NotDetected`

This is intentionally not treated as authoritative Steam Deck telemetry.

## Output Artifacts

Runtime writes:

- `Saved/PerformanceSystem/snapshot.current.json`
- `Saved/PerformanceSystem/snapshot.previous.json`
- `Saved/PerformanceSystem/Sessions/<SessionId>/events.jsonl`
- `Saved/PerformanceSystem/Sessions/<SessionId>/session_summary.json`
- `Saved/PerformanceSystem/Sessions/<SessionId>/session_summary.md`

Standalone smoke verified output under:

- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\`

Latest clean smoke session observed:

- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260519T031450Z_hZJeU0ddPnhJRie6bVDelA`

Clean smoke report showed:

- `SchemaVersion: 1`
- `ExitReason: SubsystemDeinitialize`
- `BuildConfig: Development`
- `World: FrontendLevel`
- `Map: FrontendLevel`
- `SessionStarted: 1`

## Verification Performed

Editor build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReload
```

Result:

- Succeeded.

Game build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReload
```

Result:

- Succeeded.

Standalone staging:

```powershell
& .\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development
```

Result:

- Build/cook/package succeeded.
- Script failed after package while copying loose runtime content at:
  - `SourceAssets\Mini\Companions\AnimationSets\Dawn\Dawn_WalkB_R.png`
- Manual copy of that exact file succeeded after creating the destination directory.
- Retried with build/cook skipped:

```powershell
& .\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild -SkipCook
```

Result:

- Succeeded.
- Staged exe ready at:
  - `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Repo shortcut refreshed.
- Taskbar shortcut refreshed.

Shortcut verification:

- `C:\UE\T66\T66 Standalone.lnk`
  - Target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Args: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`

- `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
  - Target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Args: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`

Standalone smoke:

```powershell
T66.exe -nullrhi -nosound -unattended -ExecCmds="Quit" -abslog="C:\UE\T66\Saved\StandaloneLogs\T66_PerformanceSystemCleanQuit.log" -forcelogflush
```

Result:

- Process exited `0`.
- `snapshot.current.json` existed.
- `snapshot.previous.json` existed.
- `session_summary.json` existed.
- `session_summary.md` existed.
- `events.jsonl` existed.
- First event in `events.jsonl` was verified to be one JSON object on one line.

## Important Caveats

- The smoke test used `-nullrhi`, so it did not validate GPU timing, real rendering, or real gameplay frame pacing.
- The clean smoke quit only reached `FrontendLevel` and produced one frame sample. It proves subsystem initialization/reporting, not gameplay attribution quality.
- VRAM pressure is currently reported as `Unavailable`; no Shipping-safe RHI adapter has been bound yet.
- Sidecar hardware telemetry is not implemented yet.
- Named-pipe IPC is schema-documented but not implemented yet.
- Reliable hang detection is not implemented; Phase D still needs an external launcher or persistent sidecar watchdog.
- Universal sync-load interception is not implemented.
- Per-draw-call and per-material attribution are not implemented.
- Automatic Unreal Insights trace triggering is not implemented.
- Cross-session regression comparison tooling is not implemented.

## Stale Pending Issue Resolved

There was already a stale pending issue in:

- `Source/T66/PerformanceSystem/pending_issues_PerformanceSystem.md`

It claimed the subsystem was only declared/stubbed. After implementation and successful builds, that file was updated to say no unresolved first-pass PerformanceSystem pending issues are currently recorded.

## Recommended Next Steps For Claude

1. Validate threshold defaults in real gameplay, not `-nullrhi`.
2. Add a small gameplay smoke route that loads a representative floor long enough to produce meaningful frame/memory windows.
3. Add a Dev-only detector wave for known actor-class tick costs, Niagara active counts, Slate widget counts, and aggregate draw-call counts.
4. Decide whether `UT66LagTrackerSubsystem` should remain a separate subsystem or become a thin compatibility facade over `PerformanceSystem`.
5. Implement the optional LibreHardwareMonitor named-pipe sidecar only after the in-engine reports are stable.
6. Add a report parser/checker script that validates event/report JSON against the committed schemas.
7. Revisit retention behavior after a few real sessions to confirm the 256 MB Development and 64 MB Shipping caps are appropriate.

## Git / Coordination Notes

- No commit was made.
- No push was made.
- Targeted git checks only were used to avoid broad Unreal/LFS scans.
- `Source/T66/T66.Build.cs` showed an existing staged dependency edit unrelated to this pass; this implementation only added the `PerformanceSystem` include path.

