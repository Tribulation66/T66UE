# T66 Visual Lock - Iteration 01

Goal: boot the staged standalone into a clean Retro FX baseline where the only active retro layer is real low-resolution framebuffer rendering at the existing 60 percent target height.

## Applied Changes

### Retro FX defaults

File: `Source/T66/Core/T66RetroFXSettings.h`

| Setting | Line | Iteration 01 value | Reason |
|---|---:|---|---|
| `bEnableRetroFXMaster` | 22 | `true` | Keep the Retro FX stack enabled so real low-res can apply. |
| `PS1BlendPercent` | 25 | `0.0f` | Disable PS1 blend. |
| `PS1DitheringPercent` | 28 | `0.0f` | Disable runtime PS1 dither. |
| `PS1BayerDitheringPercent` | 31 | `0.0f` | Disable Bayer dither variant. |
| `PS1ColorLUTPercent` | 34 | `0.0f` | Disable PS1 LUT/palette path. |
| `PS1ColorBoostPercent` | 37 | `0.0f` | Disable PS1 color boost. |
| `PS1FogPercent` | 40 | `0.0f` | Disable PS1 fog contribution and PS1 material weight from fog. |
| `PS1FogDensityPercent` | 43 | `0.0f` | Keep PS1 fog density zero. |
| `PS1FogStartDistancePercent` | 46 | `0.0f` | Keep PS1 fog distance controls at zero for a clean baseline. |
| `PS1FogFallOffDistancePercent` | 49 | `0.0f` | Keep PS1 fog falloff controls at zero for a clean baseline. |
| `PS1SceneDepthFogPercent` | 52 | `0.0f` | Disable scene-depth fog variant. |
| `bUseRealLowResolution` | 55 | `true` | The one active effect for Iteration 01. |
| `FakeResolutionSwitchSizePercent` | 58 | `0.0f` | Disable fake resolution material switch. |
| `FakeResolutionSwitchUVPercent` | 61 | `0.0f` | Disable fake UV resolution material switch. |
| `TargetResolutionHeightPercent` | 64 | `60.0f` | Resolves to 504 px target height. |
| `N64BlurBlendPercent` | 67 | `0.0f` | Disable N64 blur. |
| `N64BlurStepsPercent` | 70 | `0.0f` | Keep N64 blur internals clean while disabled. |
| `N64LowFakeResolutionPercent` | 73 | `0.0f` | Keep N64 fake low-res path off. |
| `ChromaticAberrationPercent` | 79 | `0.0f` | Disable chromatic split. |
| `ChromaticDistortionPercent` | 82 | `0.0f` | Disable chromatic distortion. |
| `T66PixelationPercent` | 88 | `0.0f` | Disable legacy/global pixelation. |
| `WorldPixelationPercent` | 91 | `0.0f` | Disable world pixelation. |
| `CharacterPixelationPercent` | 94 | `0.0f` | Disable character pixelation. |
| `bEnableCharacterOutline` | 97 | `false` | Disable stencil outline for source-outline baseline. |
| `bEnableWorldGeometry` | 190 | `false` | Disable world geometry retro effects. |
| `WorldVertexSnapPercent` | 193 | `0.0f` | Disable world vertex snap. |
| `WorldVertexSnapResolutionPercent` | 196 | `0.0f` | Zero inactive world snap resolution. |
| `WorldVertexNoisePercent` | 199 | `0.0f` | Disable world vertex noise. |
| `WorldAffineBlendPercent` | 202 | `0.0f` | Disable world affine blend. |
| `WorldAffineDistance1Percent` | 205 | `0.0f` | Zero inactive world affine distance. |
| `WorldAffineDistance2Percent` | 208 | `0.0f` | Zero inactive world affine distance. |
| `WorldAffineDistance3Percent` | 211 | `0.0f` | Zero inactive world affine distance. |
| `bEnableCharacterGeometry` | 214 | `false` | Disable character geometry retro effects. |
| `CharacterVertexSnapPercent` | 217 | `0.0f` | Disable character vertex snap. |
| `CharacterVertexSnapResolutionPercent` | 220 | `0.0f` | Zero inactive character snap resolution. |
| `CharacterVertexNoisePercent` | 223 | `0.0f` | Disable character vertex noise. |
| `CharacterAffineBlendPercent` | 226 | `0.0f` | Disable character affine blend. |
| `CharacterAffineDistance1Percent` | 229 | `0.0f` | Zero inactive character affine distance. |
| `CharacterAffineDistance2Percent` | 232 | `0.0f` | Zero inactive character affine distance. |
| `CharacterAffineDistance3Percent` | 235 | `0.0f` | Zero inactive character affine distance. |

### Save-game resolution

Files:

- `Source/T66/Core/T66PlayerSettingsSaveGame.h:84`
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp:238`

Retro FX values resolve from `UT66PlayerSettingsSubsystem::GetRetroFXSettings()`, which reads `T66_PlayerSettings.sav`. `FT66RetroFXSettings` is a `SaveGame` struct, not a config-backed settings object, so `DefaultGame.ini` / `DefaultEngine.ini` cannot reliably drive these values at boot.

The first staged verification pass exposed a stale-save conflict: the fresh first boot applied the new struct defaults, but the next boot loaded `bRetroFXMasterEnabled=false` from the existing staged player settings save, disabling the real low-resolution runtime path.

To make the staged build boot correctly without user save manipulation, player settings schema was bumped from `21` to `22`, and the `<22` migration resets only `RetroFXSettings` to `FT66RetroFXSettings()` and sets `bRetroFXMasterEnabled` from that struct. That preserves the existing settings system while guaranteeing stale saves migrate into the Iteration 01 baseline.

## Runtime Resolution Path

1. `UT66PlayerSettingsSubsystem::LoadOrCreate()` loads or creates `T66_PlayerSettings.sav`.
2. `UT66PlayerSettingsSubsystem::GetRetroFXSettings()` returns the saved `RetroFXSettings` and overwrites `bEnableRetroFXMaster` from `bRetroFXMasterEnabled`.
3. `AT66GameMode::HandleSettingsChanged()` and `AT66FrontendGameMode::HandleSettingsChanged()` apply current settings through `UT66RetroFXSubsystem`.
4. `UT66RetroFXSubsystem::ApplySettings()` builds effective settings, applies post-process weights, applies resolution runtime, and forwards pixelation levels to `UT66PixelationSubsystem`.
5. `UT66RetroFXSubsystem::ApplyResolutionRuntime()` maps `TargetResolutionHeightPercent=60` to `TargetHeight=504`, then applies `r.ScreenPercentage = TargetHeight / ViewportHeight * 100`.

For a 1080p staged window this resolves to:

`504 / 1080 * 100 = 46.67`

## Build And Stage

Command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames
```

Final run status:

- UnrealHeaderTool: succeeded for `T66Editor` and `T66`.
- C++ build: succeeded for `T66Editor Win64 Development` and `T66 Win64 Development`.
- Cook: succeeded for Windows.
- Stage/package: succeeded.
- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Staged executable timestamp: `2026-05-14 14:08:10`
- Staged executable size: `310,383,616` bytes.
- Pinned taskbar shortcut target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Pinned taskbar shortcut args: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`

## Verification Readout

Verification used the staged executable with no `?listen`, verbose Retro FX logs, and `-T66GameplayAutoScreenshot`.

### First boot after reset

Log: `Saved/StandaloneLogs/VisualLock_Iteration01_verify_first_boot.log`

Screenshot: `Saved/VisualLock/Iteration01/verify_first_boot.png`

Evidence:

- `T66_PlayerSettings.sav` was absent on boot: log line 571 reported failed read.
- Runtime settings line 914: `MasterEnabled=true`, all PS1/chromatic/fog values `0.00`, `RealLowRes=true`, `FakeSize=0.00`, `FakeUV=0.00`, `TargetRes=60.00`.
- Blend weights line 916: `PS1Weight=0.000`, `FogWeight=0.000`, `OutlineWeight=0.000`, `N64Weight=0.000`, `ChromaticWeight=0.000`.
- PS1 parameters line 917: `DitheringStrength=0.000`, `Bayer=false`, `ColorLUT=false`, `SceneDepthFog=false`, `FogEnabled=false`, `FogDensity=0.000`.
- Resolution line 919: `TargetResolutionHeight=504.000`.
- Runtime framebuffer line 921: `ViewportHeight=1080.00`, `TargetHeight=504.00`, `ScreenPercentage=46.67`.

### Second boot from staged save

Log: `Saved/StandaloneLogs/VisualLock_Iteration01_verify_second_boot.log`

Screenshot: `Saved/VisualLock/Iteration01/verify_second_boot.png`

Evidence:

- Runtime settings line 909: `MasterEnabled=true`, all PS1/chromatic/fog values `0.00`, `RealLowRes=true`, `FakeSize=0.00`, `FakeUV=0.00`, `TargetRes=60.00`.
- Blend weights line 911: `PS1Weight=0.000`, `FogWeight=0.000`, `OutlineWeight=0.000`, `N64Weight=0.000`, `ChromaticWeight=0.000`.
- PS1 parameters line 912: `DitheringStrength=0.000`, `Bayer=false`, `ColorLUT=false`, `SceneDepthFog=false`, `FogEnabled=false`, `FogDensity=0.000`.
- Resolution line 914: `TargetResolutionHeight=504.000`.
- Runtime framebuffer line 916: `ViewportHeight=1080.00`, `TargetHeight=504.00`, `ScreenPercentage=46.67`.

Staged save state after verification:

- `Saved/StagedBuilds/Windows/T66/Saved/SaveGames/T66_PlayerSettings.sav`
- size: `1732` bytes
- created: `2026-05-14 14:10:44`
- last written: `2026-05-14 14:10:44`

The second boot proves Pablo's next taskbar launch will read a staged save that keeps `MasterEnabled=true` and the Iteration 01 real-low-res-only baseline.

## Unexpected Findings

- `FT66RetroFXSettings` is not config-backed, so config overrides were not the correct implementation surface.
- Reset staging alone was not a sufficient proof because a staged save can reappear or persist into the next boot. The schema 22 migration was required to guarantee stale saves cannot disable the master toggle.
- `UT66RetroFXSubsystem::ApplyResolutionRuntime()` logs `ScreenPercentage=100.00` on the earliest apply before viewport height is valid (`ViewportHeight=1.00`). It reapplies after the real 1080p viewport exists and correctly resolves to `46.67`.
- On process shutdown the subsystem restores `r.ScreenPercentage` to `100`; this is expected cleanup and does not affect the next boot because settings are applied again during gameplay startup.

## Pending Issues Created

None.

No new `pending_issues_<foldername>.md` file was created. The stale-save behavior was in scope for this iteration and was fixed through the settings schema migration.
