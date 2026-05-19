# T66 Visual Cleanup - Iteration 01 Report

Date: 2026-05-15

## Scope

This pass applied the five cleanup areas from `Audit/Reference/Gameplay_Visual_Cleanup_Investigation/Report.md`: frontend/gameplay Retro FX backend separation, duplicate CVar write removal, disabled-feature DMI/MPC gating, asset archival, and orphaned preload removal. Combat, AI, audio, save-game behavior, and unrelated UI code were left out of scope.

## Fix 1 - Frontend / gameplay Retro FX backend separation

| File | Lines | Before | After |
|---|---:|---|---|
| `Source/T66/Gameplay/T66FrontendGameMode.cpp` | 79-88 | `HandleSettingsChanged()` invoked `UT66RetroFXSubsystem::ApplyCurrentSettings(World)` from the frontend world. | Frontend now only calls `FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World)`. Gameplay Retro FX subsystem is not invoked from frontend. |

Implementation notes:
- Removed the `Core/T66RetroFXSubsystem.h` include from `T66FrontendGameMode.cpp`.
- Kept neutral world visual setup shared for frontend, as requested.
- Added the call-site comment documenting that frontend Retro FX is driven by the UI retainer (`UT66FrontendUIRootWidget`) and gameplay Retro FX is intentionally not invoked in frontend.
- Removed the pre-gameplay frontend-world Retro FX apply in `Source/T66/Core/T66GameInstance.cpp:1727-1750`; this prevented `TransitionToGameplayLevel()` from applying gameplay Retro FX to the current frontend world immediately before map travel.

Verification:
- `Saved/StandaloneLogs/VisualCleanup_Iteration01_Frontend.log` contains `0` actual `LogT66RetroFXRuntime` lines during frontend boot.
- `Saved/StandaloneLogs/VisualCleanup_Iteration01_Stage1.log` contains `3` actual `LogT66RetroFXRuntime` lines during gameplay boot.
- Runtime gameplay line confirms the backend now activates in gameplay with the Iteration 02 low-res baseline:

```text
ApplyResolutionRuntime: enabled real low resolution ViewportHeight=1080.00 TargetHeight=336.00 ScreenPercentage=31.11 AntiAliasingMethod=0 UpscaleQuality=1 TemporalAAUpsampling=0 MinResolutionFraction=unavailable MinResolution=0.00 SecondaryScreenPercentage=100.00
```

## Fix 2 - CVar priority warning cleanup

| File | Lines | Before | After |
|---|---:|---|---|
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | 1025-1068 | `ApplyResolutionRuntime()` wrote project-owned AA/upscale CVars at runtime, producing ignored-priority warnings. | Runtime writes were removed for project-owned CVars. `r.ScreenPercentage` remains runtime-owned. |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | 1906-1920 | Restore path wrote project-owned AA/upscale/min-resolution CVars on cleanup. | Restore path only resets runtime-owned resolution overrides. |
| `Source/T66/Core/T66GameInstance.cpp` | 143-175 | `ApplyCrispRenderingDefaults()` also wrote project-owned AA/upscaler CVars. | Removed duplicate writes for `r.SecondaryScreenPercentage.GameViewport` and `r.TemporalAA.Upsampling`. |

Project-owned CVars left to `Config/DefaultEngine.ini [SystemSettings]`:

| CVar | Runtime write status |
|---|---|
| `r.AntiAliasingMethod` | Removed from Retro FX runtime path |
| `r.Upscale.Quality` | Removed from Retro FX runtime path |
| `r.TemporalAA.Upsampling` | Removed from Retro FX runtime path and game-instance crisp defaults |
| `r.SecondaryScreenPercentage.GameViewport` | Removed from Retro FX runtime path and game-instance crisp defaults |
| `r.ScreenPercentage.MinResolution` | Removed from Retro FX runtime path |
| `r.ScreenPercentage` | Kept runtime-owned because it depends on viewport height and target resolution |

Unexpected related finding:
- After removing the frontend-world apply, direct gameplay startup initially resolved an early viewport height of `1`, which caused `r.ScreenPercentage` to clamp incorrectly to `100`. `GetViewportHeight()` now ignores zero/small `GameViewport` sizes and falls back to the existing 1080p default path (`Source/T66/Core/T66RetroFXSubsystem.cpp:1923-1958`).

## Fix 3 - DMI / MPC null warning cleanup via gating

| Feature | File / lines | Gate behavior |
|---|---:|---|
| PS1 stack | `Source/T66/Core/T66RetroFXSubsystem.cpp:361-392`, `586-657`, `688-792`, `827-834`, `888-938` | Skips preload, DMI ensure, parameter apply, and null warning when all PS1 controls are zero. |
| Chromatic aberration | `Source/T66/Core/T66RetroFXSubsystem.cpp:941-981` | Skips apply and null warning when `ChromaticAberrationPercent` and `ChromaticDistortionPercent` are both zero. |
| Fake-resolution MPC switches | `Source/T66/Core/T66RetroFXSubsystem.cpp:984-1022` | Skips MPC load/apply/null warnings when fake-resolution switch controls are zero. |
| N64 blur | `Source/T66/Core/T66RetroFXSubsystem.cpp:586-657`, `688-792` | Queues and applies N64 blur only when its controls are nonzero. |
| Outline | `Source/T66/Core/T66RetroFXSubsystem.cpp:688-792`, `795-823` | Confirmed still gated by `bEnableCharacterOutline`. |
| Pixelation | `Source/T66/Core/T66PixelationSubsystem.cpp:50-100`, `165` | Pixelation material preload and blendable creation are skipped when world and character pixelation are both zero. Existing DMI weight is zeroed if present. |

Header update:
- `Source/T66/Core/T66RetroFXSubsystem.h:89` now passes settings into `QueueRetroAssetPreloads(const FT66RetroFXSettings& Settings)`.

Verification:

| Warning pattern | Baseline count | New count |
|---|---:|---:|
| `ApplyPs1Parameters: PS1 post-process DMI was null` | 0 | 0 |
| `ApplyChromaticAberrationParameters: chromatic aberration DMI was null` | 0 | 0 |
| `ApplyResolutionCollection: resolution MPC was null` | 0 | 0 |
| `[Pixelation] Material at /Game/UI/M_PixelationPostProcess` | 0 | 0 |

## Fix 4 - Asset cleanup and archiving

Archive script:
- Temporary script used: `Saved/VisualCleanup/ArchiveVisualCleanupAssets.py`
- The temporary script was deleted after the archive pass per the script lifecycle rule.
- Archive manifest retained at `Saved/VisualCleanup/archive_manifest.json`.

Safety checks:
- Re-ran quick string-reference checks across `Source`, `Content/Data`, `Scripts`, and `Config` before moving the requested assets.
- No archive was blocked by runtime/source/data references.
- The legacy Hero_1 Chad `Idle` / `Walk` paths still appear in legacy inspection/import scripts only. Those scripts were left in place and documented in `Scripts/pending_issues_Scripts.md`.

| Source | Archive destination | Status |
|---|---|---|
| `/Game/World/Terrain/Common/Landscape` | `/Game/World/Terrain/_Archive/Common/Landscape` | Moved |
| `/Game/World/Terrain/Common/Rocks` | `/Game/World/Terrain/_Archive/Common/Rocks` | Moved |
| `/Game/Characters/Heroes/Hero_1/Chad/Idle` | `/Game/Characters/_Archive/Heroes/Hero_1/Chad/IdleWalk_Legacy/Idle` | Moved |
| `/Game/Characters/Heroes/Hero_1/Chad/Walk` | `/Game/Characters/_Archive/Heroes/Hero_1/Chad/IdleWalk_Legacy/Walk` | Moved |
| `/Game/Characters/Heroes/Hero_3/Chad/RigPrototype` | `/Game/Characters/_Archive/Heroes/Hero_3/Chad/RigPrototype` | Moved |
| `/Game/Characters/Heroes/Knight` | `/Game/Characters/_Archive/Heroes/Knight` | Moved |
| `/Game/Materials/MI_TestSlime_Unlit` | `/Game/Materials/_Archive/Track2/MI_TestSlime_Unlit` | Moved |
| `/Game/Materials/MI_TestSlime_ViewSpaceLit` | `/Game/Materials/_Archive/Track2/MI_TestSlime_ViewSpaceLit` | Moved |
| `/Game/Materials/MI_GLB_ViewSpaceLit_Character_Test` | `/Game/Materials/_Archive/Track2/MI_GLB_ViewSpaceLit_Character_Test` | Moved |
| `/Game/Materials/M_Track2_NeutralBackdrop` | `/Game/Materials/_Archive/Track2/M_Track2_NeutralBackdrop` | Moved |

## Fix 5 - Orphaned preload removal

| File | Lines | Before | After |
|---|---:|---|---|
| `Source/T66/Core/T66GameInstance.cpp` | 437-556 | Startup hero-selection preload included `/Game/Characters/Heroes/Knight/KnightClip.KnightClip`. | Knight preview preload and helper path were removed. |

Additional preload cleanup:
- Removed the orphaned `KnightPreviewMovieName` and `ResolveKnightPreviewMoviePath()` helper.
- Checked the same preload section for other paths pointing to archived assets; no other archived-asset preload remained.

## Warning count diff

Baseline log: `Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log`

New log: `Saved/StandaloneLogs/VisualCleanup_Iteration01_Stage1.log`

| Warning group | Baseline | New | Result |
|---|---:|---:|---|
| Total warnings | 55 | 38 | Reduced by 17 |
| Tracked visual CVar priority warnings | 21 | 4 | Reduced by 17 |
| DMI/MPC/pixelation disabled-feature warnings | 0 | 0 | Clean in both logs |

| Warning pattern | Baseline | New |
|---|---:|---:|
| `r.AntiAliasingMethod` priority warning | 3 | 0 |
| `r.Upscale.Quality` priority warning | 5 | 2 |
| `r.TemporalAA.Upsampling` priority warning | 4 | 0 |
| `r.SecondaryScreenPercentage.GameViewport` priority warning | 4 | 0 |
| `r.ScreenPercentage.MinResolution` priority warning | 3 | 0 |
| `r.HeterogeneousVolumes` priority warning | 2 | 2 |

Remaining visual CVar warnings:
- `r.Upscale.Quality`: remaining warnings are scalability-vs-project-setting ownership, not the removed Retro FX runtime SetByGameSetting path.
- `r.HeterogeneousVolumes`: still a project-vs-scalability conflict and documented as out of scope in `Source/T66/Core/pending_issues_Core.md`.

## Build and stage status

Final stage command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames
```

Result:
- Final build and stage completed successfully.
- Staged executable verified at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Taskbar shortcut verified at `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`.
- Shortcut target verified as `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Unexpected staging note:
- One stage attempt hit a transient `UnrealBuildTool_Mutex` / `ConflictingInstance` error while a previous UBT process was still exiting. The process cleared, the same stage command was retried, and the final stage succeeded.

## Runtime verification artifacts

| Artifact | Path |
|---|---|
| Frontend log | `Saved/StandaloneLogs/VisualCleanup_Iteration01_Frontend.log` |
| Gameplay log | `Saved/StandaloneLogs/VisualCleanup_Iteration01_Stage1.log` |
| Verification summary | `Saved/VisualCleanup/Iteration_01/verification_summary.txt` |
| Frontend screenshot | `Saved/VisualCleanup/Iteration_01/frontend_menu.png` |
| Stage 1 sanity screenshot | `Saved/VisualCleanup/Iteration_01/stage1_sanity.png` |

Screenshots:

![Frontend menu](../../../Saved/VisualCleanup/Iteration_01/frontend_menu.png)

![Stage 1 sanity](../../../Saved/VisualCleanup/Iteration_01/stage1_sanity.png)

## Pending issues created

| File | Topic |
|---|---|
| `Source/T66/Core/pending_issues_Core.md` | `r.HeterogeneousVolumes` still has a project-vs-scalability CVar ownership conflict. |
| `Content/Materials/pending_issues_Materials.md` | `M_GLB_ViewSpaceLit_Character` master remains parked pending visual lock A/B decision. |
| `Content/Characters/pending_issues_Characters.md` | Remaining companion and hero variant ownership needs a dedicated audit before broader archival. |
| `Scripts/pending_issues_Scripts.md` | Legacy hero inspection/import scripts still reference the archived Hero_1 Chad Idle/Walk paths. |

## Deferred items

- Did not archive `M_GLB_ViewSpaceLit_Character`; it is the Track 2 master kept for future A/B.
- Did not remove one-off migration/import scripts; they remain possibly legacy and are documented for a future cleanup pass.
- Did not attempt to solve `r.HeterogeneousVolumes` because it is outside the gameplay visual code path changed here.
