# RetroFX Off-By-Default Fix Completion Packet

## Review Status

- Working goal: make the retro/CRT post-FX off by default, remove the duplicate master source of truth, migrate existing saves off, seal all known recreation paths, prove `r.ScreenPercentage` returns to `100`, and record a corrected full-resolution lightweight FPS sanity reference.
- Plan packet: `Reports/AgentReviews/20260529_RetroFXOffByDefaultFix/plan_packet.md`
- Claude review: `Reports/AgentReviews/20260529_RetroFXOffByDefaultFix/20260528T214339-pass1/claude_review_pass1.md`
- Verdict: `APPROVE`
- Workflow note: under the updated `AGENTS.md` rule, a valid Claude approval greenlit implementation without an extra manual Pablo go-ahead.

## Implementation Summary

- Changed `FT66RetroFXSettings` defaults so gameplay RetroFX, real low resolution, and frontend fullscreen CRT default off.
- Removed the duplicate saved `bRetroFXMasterEnabled` field from `UT66PlayerSettingsSaveGame`; `FT66RetroFXSettings::bEnableRetroFXMaster` is now the single gameplay master source.
- Added schema `24` save migration that forces all existing saves' RetroFX/CRT state off. Prior saved-on states are intentionally not preserved because the previous on state came from the bug default.
- Updated reset, safe-mode, UI reset/defaults, save-load migration, settings application, and world-startup verification so they inherit the off defaults instead of recreating on.
- Updated RetroFX runtime resolution teardown so the off path always sets `r.ScreenPercentage=100`, not the previously captured reduced value.
- Changed Windows staged device-profile default from `r.ScreenPercentage=85` to `100` so fresh staged launches are full-resolution by default.

## Files Touched

- `Config/DefaultDeviceProfiles.ini`
- `Source/T66/Core/T66RetroFXSettings.h`
- `Source/T66/Core/T66PlayerSettingsSaveGame.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`
- `Source/T66/Core/T66RetroFXSubsystem.cpp`
- `Source/T66/Gameplay/T66GameMode.cpp`

`Source/T66/Gameplay/T66GameMode.cpp` already had unrelated dirty gameplay edits in the live worktree. This pass only added the `RunRetroFXSealVerificationIfRequested(GetWorld())` call after RetroFX settings are applied on settings change.

## Verification

### Build

Command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex
```

Result: passed.

Known pre-existing warnings:

- `Source\T66Mini\T66Mini.Build.cs` references missing directory `Source\T66Mini\Public\UI\Components`.
- `Source\T66\Gameplay\T66Hero1AxeAOEVFXLabActor.cpp` uses deprecated Niagara readiness API.

### Staged Standalone

Command:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\StageStandaloneBuild.ps1 -SkipBuild
```

Result: passed. The script restored staged savegames and verified the standalone shortcut target:

`C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

Staged executable SHA256 used for recurrence and FPS verification:

`BB56594D3142EE2C35FC6740A0ECEB2F198E62137132FB052CB213A92FAEDAA2`

After the seal run exercised safe mode, the generated staged `GameUserSettings.ini` was reset to full-resolution/high scalability for the FPS sanity capture. That was a generated staged config reset, not a source edit.

### Recurrence-Seal Run

Command shape:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -T66Entry=Tower -T66RetroFXSealVerify -T66AutomationResX=1920 -T66AutomationResY=1080 -windowed -forcelogflush -abslog=C:\UE\T66\Saved\StandaloneLogs\T66_RetroFXSealVerify.log
```

The process was stopped after the final `MapWorldLoad` proof line was emitted. This made the process exit code `-1`; the proof itself completed.

| Path | EnabledAfter | RealLowResAfter | UIFullScreenCRTAfter | ScreenPercentage | WorldPixelationLevel | CharacterPixelationLevel |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| FreshLaunch | 0 | 0 | 0 | 100.00 | 0 | 0 |
| SettingsReset | 0 | 0 | 0 | 100.00 | 0 | 0 |
| SafeMode | 0 | 0 | 0 | 100.00 | 0 | 0 |
| UIReset | 0 | 0 | 0 | 100.00 | 0 | 0 |
| LegacySaveLoadMigration | 0 | 0 | 0 | 100.00 | 0 | 0 |
| GameplaySettingsApply | 0 | 0 | 0 | 100.00 | 0 | 0 |
| MapWorldLoad | 0 | 0 | 0 | 100.00 | 0 | 0 |

Evidence log:

`C:\UE\T66\Saved\StandaloneLogs\T66_RetroFXSealVerify.log`

### Full-Resolution Lightweight FPS Sanity Read

Command shape:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -T66Entry=Run:Tower -T66GameplayAutoCapture=enemywaveperf -T66GameplayAutoScreenshotDelay=1 -T66GameplayAutoPostCaptureScreenshotDelay=125 -T66AutomationResX=1920 -T66AutomationResY=1080 -T66AutomationWindowed -windowed -ResX=1920 -ResY=1080 -T66AutoCaptureHeroHPOverride=20000 -T66RangedDiagnosticLogging=1 -T66MobUseLightweight=1 -T66MobRouteRushLightweight=1 -T66MobRouteFlyingLightweight=1 -T66MobRouteRangedLightweight=1 -T66MobUseTouchDamageOverlap=1 -T66MobManagerTickProfileEnabled=0 -T66PerfSubstepAttribution=0 -forcelogflush
```

Result: passed.

| Metric | Value |
| --- | ---: |
| Exit code | 0 |
| Staged SHA stable | yes |
| Average saturated FPS | 146.30 |
| 1% low FPS | 72.33 |
| 0.1% low FPS | 50.12 |
| Saturated samples | 16,397 |
| Peak live regular enemies | 90 |
| Peak lightweight mobs | 87 |
| Peak active enemy projectiles | 2 |
| PerformanceSystem overhead max | 878.3 us |
| Hero HP at end | 18,860 |
| Hero projectile hits | 109 |
| Projectile damage | 2,180 HP |
| Projectiles fired | 111 |
| Projectiles hit hero | 109 |
| Terminal reason | `GameplayAutomationQuit` |

Evidence:

- Result JSON: `C:\UE\T66\Saved\Codex\Performance\RetroFXOffByDefaultFix\full_res_enemywaveperf_result.json`
- Capture log: `C:\UE\T66\Saved\StandaloneLogs\T66_RetroFXOffByDefault_FullResEnemyWavePerf.log`
- Performance session: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260529T005943Z_pcDN4kb7jC2K-f-qMRSHKQ`
- Screenshot: `C:\UE\T66\Saved\Codex\Performance\RetroFXOffByDefaultFix\screenshots\T66_RetroFXOffByDefault_FullResEnemyWavePerf.png`

This 146.30 FPS value is the corrected single-capture full-resolution sanity reference. It should not be compared directly to the earlier 192/200 captures, which likely benefited from the real-low-resolution downscale. The architecture conclusion remains intact; this changes the honest full-resolution absolute reference before B.13.

## Pending Issue Updates

- `Source/T66/Core/pending_issues_Core.md`: added resolved entry for the default-on recurrence root cause.
- `Reports/AgentReviews/20260528_RetroFXPixelationRootCause/root_cause_report.md`: linked this fix packet as the resolution.
- `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`: added full-resolution baseline note for B.13.

## Outcome

RetroFX/CRT now defaults off at source, migrated saves are forced off, recurring recreation paths are sealed, and the off path restores full-resolution rendering with `r.ScreenPercentage=100`.
