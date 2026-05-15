# T66 Visual Lock - Iteration 02 Report

Date: 2026-05-14  
Working directory: `C:\UE\T66`

## Working Goal

Layer the Iteration 02 visual-lock changes onto the Iteration 01 clean Retro FX baseline, then build, stage, and verify the standalone boots with bigger uniform low-res pixels, brightness 0.8 character materials, and no stale-save override.

## Inspection Inputs

- `AGENTS.md`
- Existing `pending_issues_*.md` files under `Source/T66/Core`, `Source/T66/Gameplay`, `Source/T66/Gameplay/Enemies`, `Source/T66/Data`, and `Content/Data`
- `Audit/Reference/Visual_Systems_Audit/Report.md`
- `Audit/Reference/Visual_Lock/Iteration_01_Report.md`
- `Source/T66/Core/T66RetroFXSettings.h`
- `Source/T66/Core/T66RetroFXSubsystem.cpp` / `.h`
- `Source/T66/Core/T66PixelationSubsystem.cpp` / `.h`
- `Source/T66/Core/T66PlayerSettingsSaveGame.h`
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`
- `Source/T66/Gameplay/T66WorldVisualSetup.cpp`
- `Config/DefaultEngine.ini`
- `Config/DefaultGame.ini`

## Changes Applied

| Area | File / asset | Lines / evidence | Change |
|---|---|---:|---|
| Retro FX default | `Source/T66/Core/T66RetroFXSettings.h` | line 64 | `TargetResolutionHeightPercent = 40.0f` |
| Target height mapping | `Source/T66/Core/T66RetroFXSubsystem.cpp` | lines 111-114 | `40.0f` now resolves to `336px`; the helper maps 0-100 to 0-840 with a 120px floor. |
| Runtime upscaler CVars | `Source/T66/Core/T66RetroFXSubsystem.cpp` | lines 929-954 | Real-low-res path sets AA off, nearest upscaler, TAA upsampling off, secondary percentage 100, and low screen-percentage clamp fallback. |
| Runtime restore state | `Source/T66/Core/T66RetroFXSubsystem.h` / `.cpp` | header lines 198, 202-206; cpp lines 1773-1818 | Captures and restores the extra AA/upscaler/min-resolution CVars when real low-res is disabled. |
| Boot-reliable renderer CVars | `Config/DefaultEngine.ini` | lines 107-111 | `r.TemporalAA.Upsampling=False`, `r.AntiAliasingMethod=0`, `r.Upscale.Quality=1`, `r.SecondaryScreenPercentage.GameViewport=100`, `r.ScreenPercentage.MinResolution=0`. |
| Save schema | `Source/T66/Core/T66PlayerSettingsSaveGame.h` | line 84 | `SchemaVersion = 23`. |
| Save migration | `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` | lines 246-254 | `<23` migration resets only `RetroFXSettings` to `FT66RetroFXSettings()` and syncs `bRetroFXMasterEnabled`. |
| Runtime character brightness | `Source/T66/Core/T66CharacterVisualSubsystem.cpp` | lines 32, 476, 555 | Shared brightness constant set to `0.8f`; applied to imported skeletal fallback and QuadRetro static DMI paths. |
| Character material instance | `/Game/Materials/MI_GLB_Unlit_Character_Shared` | `Saved/VisualLock/Iteration02/asset_probe.json` | `Brightness` changed from `1.0` to `0.8`. |
| Terrain texture filtering | `/Game/World/Terrain/TowerForest/*`, `/Game/World/Terrain/TowerDungeon/*` | `Saved/VisualLock/Iteration02/asset_probe.json` | 43 targeted floor/wall base-color terrain textures changed from `TF_DEFAULT` to `TF_NEAREST`. |

## Runtime Value Resolution

Retro FX values still resolve from `UT66PlayerSettingsSubsystem` / `T66_PlayerSettings.sav`, not config. For this iteration, the schema bump to `23` invalidates stale Retro FX values and resets only the Retro FX struct on migration. Other player settings remain preserved.

The staged build command used `-ResetSavedGames`, so the first verification boot started with no staged `T66_PlayerSettings.sav`; the first boot created a schema-23 save, and the second boot confirmed the saved values stay on the Iteration 02 baseline.

## Upscaler Notes

The requested `r.Upscale.Quality=0` was tested first and failed in this UE 5.7 build:

- Crash signature: `Assertion failed: Method != EUpscaleMethod::None`
- Engine file: `Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessUpscale.cpp:250`
- Cause: `EUpscaleMethod` enum maps `0` to `None`, `1` to `Nearest`, even though the CVar help text labels `0` as nearest.

Final choice: `r.Upscale.Quality=1`, which maps to `EUpscaleMethod::Nearest` and gives the hard-edged spatial upscaler path without the assertion.

`r.ScreenPercentage.MinResolutionFraction` is not present in this UE 5.7 install. The runtime tries to set it when available, but the verified staged logs report `MinResolutionFraction=unavailable`; the fallback used is `r.ScreenPercentage.MinResolution=0`.

## Ground vs Mob Investigation

The discrepancy was contained enough to fix in this iteration. Character textures and the shared character material were already on the crisp path, but gallery/gameplay terrain base-color textures were using `TF_DEFAULT`.

Asset probe evidence:

- Shared character MI: `Brightness 1.0 -> 0.800000011920929`
- Hero sample `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro`: material brightness `0.800000011920929`
- Mob sample `/Game/Characters/Mobs/Slime/SM_Slime`: material brightness `0.800000011920929`
- Terrain textures checked: `43`
- Terrain textures changed: `43`
- Example: `/Game/World/Terrain/TowerForest/T_TowerForestGround` `TF_DEFAULT -> TF_NEAREST`
- Example: `/Game/World/Terrain/TowerDungeon/T_TowerDungeonRoof` `TF_DEFAULT -> TF_NEAREST`

No pending issue was created for the ground/mob mismatch because the root cause was a contained texture-filtering mismatch and was corrected.

## Build And Stage Status

Final build/stage command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames
```

Result: `BUILD SUCCESSFUL`

Staged executable:

- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Last write: `2026-05-14T15:17:05.6101086-03:00`
- Size: `310386176` bytes

Taskbar shortcut:

- `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
- Target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Arguments: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`
- Working directory: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64`

## Verification Readout

Verification used the staged executable with no `?listen`, verbose Retro FX and character material logs, and `-T66GameplayAutoScreenshot`.

### First boot after reset

Log: `Saved/StandaloneLogs/VisualLock_Iteration02_verify_first_boot.log`  
Screenshot: `Saved/VisualLock/Iteration02/verify_first_boot.png`

Evidence:

- Line 567: staged `T66_PlayerSettings.sav` was absent on boot.
- Line 612: `MasterEnabled=true`, all PS1/chromatic/fog values `0.00`, `RealLowRes=true`, `FakeSize=0.00`, `FakeUV=0.00`, `TargetRes=40.00`.
- Line 614: `PS1Weight=0.000`, `FogWeight=0.000`, `OutlineWeight=0.000`, `N64Weight=0.000`, `ChromaticWeight=0.000`.
- Line 615: `DitheringStrength=0.000`, `Bayer=false`, `ColorLUT=false`, `SceneDepthFog=false`, `FogEnabled=false`, `FogDensity=0.000`.
- Line 617: `TargetResolutionHeight=336.000`.
- Line 1124: `ViewportHeight=1080.00`, `TargetHeight=336.00`, `ScreenPercentage=31.11`, `AntiAliasingMethod=0`, `UpscaleQuality=1`, `TemporalAAUpsampling=0`, `MinResolutionFraction=unavailable`, `MinResolution=0.00`, `SecondaryScreenPercentage=100.00`.
- Line 641: `Hero_1_Chad` QuadRetro DMI applied with `Brightness=0.80`.
- Lines 746, 786, 918: sample mobs `Slime`, `MushroomBrute`, and `Hellhound` applied with `Brightness=0.80`.

### Second boot from staged save

Log: `Saved/StandaloneLogs/VisualLock_Iteration02_verify_second_boot.log`  
Screenshot: `Saved/VisualLock/Iteration02/verify_second_boot.png`

Evidence:

- Line 601: `MasterEnabled=true`, all PS1/chromatic/fog values `0.00`, `RealLowRes=true`, `FakeSize=0.00`, `FakeUV=0.00`, `TargetRes=40.00`.
- Line 603: `PS1Weight=0.000`, `FogWeight=0.000`, `OutlineWeight=0.000`, `N64Weight=0.000`, `ChromaticWeight=0.000`.
- Line 604: `DitheringStrength=0.000`, `Bayer=false`, `ColorLUT=false`, `SceneDepthFog=false`, `FogEnabled=false`, `FogDensity=0.000`.
- Line 606: `TargetResolutionHeight=336.000`.
- Line 1113: `ViewportHeight=1080.00`, `TargetHeight=336.00`, `ScreenPercentage=31.11`, `AntiAliasingMethod=0`, `UpscaleQuality=1`, `TemporalAAUpsampling=0`, `MinResolutionFraction=unavailable`, `MinResolution=0.00`, `SecondaryScreenPercentage=100.00`.
- Line 630: `Hero_1_Chad` QuadRetro DMI applied with `Brightness=0.80`.
- Lines 735, 775, 910: sample mobs `Slime`, `MushroomBrute`, and `Hellhound` applied with `Brightness=0.80`.

Staged save after verification:

- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\SaveGames\T66_PlayerSettings.sav`
- Size: `1732` bytes
- Last write: `2026-05-14T15:19:14.4423649-03:00`

Both final verification logs were checked for `Fatal`, `Assertion failed`, `Critical error`, and `Error:`; none were present.

## Unexpected Findings

- `r.Upscale.Quality=0` is unsafe in this UE 5.7 render path despite the CVar help text. The final config uses `1`, which maps to nearest in the engine enum and passes runtime verification.
- `r.TemporalAA.Upsampling` was previously a project setting of `True`, so runtime `SetByGameSetting` attempts could not override it. `DefaultEngine.ini` now sets it to `False`.
- `r.ScreenPercentage.MinResolutionFraction` does not exist in this UE 5.7 install, so the implementation uses the available `r.ScreenPercentage.MinResolution=0` fallback.
- The original `TargetResolutionHeightPercent` helper happened to make Iteration 01's `60.0f` resolve to `504px`, but it made Iteration 02's `40.0f` resolve to `696px`. The helper was corrected so `40.0f` resolves to the requested `336px`.

## Pending Issues Created

None.
