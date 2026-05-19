# T66 Atmosphere Iteration 03 Report

## 1. Summary

Iteration 03 moves Dungeon ambient lighting off the non-contributing SkyLight path and onto the theme post-process volume's Ambient Cubemap settings. The Dungeon spec now references `/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap`, sets Ambient Cubemap intensity/tint from the spec, decommissions the SkyLight as an ambient source with intensity `0.0`, thins fog, neutralizes the remaining cool grading pushes, reduces torch/carry-light accent values, and adds runtime diagnostics for both ambient cubemap and SkyLight setup. A narrow cook rule was added for `/Engine/MapTemplates/Sky` after the first staged smoke showed the editor-visible cubemap was not present in the packaged runtime.

## 2. Stage 1 - Cubemap Asset Selection

Enumeration was run by a temporary commandlet script under `Scripts/`, which was deleted after writing the inventory to `Saved/Atmosphere_Iteration_03_TextureCubes.txt` and `Saved/Atmosphere_Iteration_03_TextureCubes.json`.

Full `TextureCube` inventory:

- `/Engine/EditorMaterials/AssetViewer/EpicQuadPanorama_CC+EV1.EpicQuadPanorama_CC+EV1`
- `/Engine/EditorMaterials/AssetViewer/T_ClearSky.T_ClearSky`
- `/Engine/EditorMaterials/AssetViewer/T_GreyAmbient.T_GreyAmbient`
- `/Engine/Engine_MI_Shaders/T_Cubemap_01.T_Cubemap_01`
- `/Engine/EngineMaterials/DefaultCubemap.DefaultCubemap`
- `/Engine/EngineResources/DefaultTextureCube.DefaultTextureCube`
- `/Engine/EngineResources/DefaultTextureCube_Low.DefaultTextureCube`
- `/Engine/EngineResources/GrayDarkTextureCube.GrayDarkTextureCube`
- `/Engine/EngineResources/GrayLightTextureCube.GrayLightTextureCube`
- `/Engine/EngineResources/GrayTextureCube.GrayTextureCube`
- `/Engine/MapTemplates/daylight.daylight`
- `/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap`
- `/Engine/MapTemplates/Sky/Desert_Outer_HDR.Desert_Outer_HDR`
- `/Engine/MapTemplates/Sky/SunsetAmbientCubemap.SunsetAmbientCubemap`

Chosen cubemap: `/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap`.

Rationale: it is the engine's explicit daylight ambient cubemap, a better neutral bright source for tinting than editor-only asset-viewer cubemaps, gray placeholders, desert/sunset scene-specific HDRs, or default engine fallback textures.

Cook/stage note: the first packaged launch resolved this path as `loaded=no`, so `Config/DefaultGame.ini:12-29` now adds `+DirectoriesToAlwaysCook=(Path="/Engine/MapTemplates/Sky")`. The second staged launch resolved the same path as `loaded=yes`.

## 3. Stage 2 - Spec Struct Changes

`Source/T66/Gameplay/T66ThemeAtmosphereData.h:5-8` now includes `Engine/TextureCube.h`.

`Source/T66/Gameplay/T66ThemeAtmosphereData.h:36-44` adds:

```cpp
// Ambient cubemap (primary ambient light source for indoor stylized scenes)
UPROPERTY()
TSoftObjectPtr<UTextureCube> AmbientCubemap;

UPROPERTY()
float AmbientCubemapIntensity = 10.0f;

UPROPERTY()
FLinearColor AmbientCubemapTint = FLinearColor(0.55f, 0.7f, 0.95f, 1.0f);
```

## 4. Stage 3 - Dungeon Spec Values

`Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:26-54` now sets the Dungeon spec as follows:

| Field | New Value | Rationale |
|---|---|---|
| `SkyLightColor` | `FLinearColor::White` | Neutral because SkyLight is no longer the ambient source |
| `SkyLightIntensity` | `0.0f` | Decommission SkyLight as ambient source |
| `AmbientCubemap` | `/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap` | Primary ambient source |
| `AmbientCubemapIntensity` | `10.0f` | Starting point for bright ambient base |
| `AmbientCubemapTint` | `FLinearColor(0.55f, 0.7f, 0.95f, 1.0f)` | Cool blue Dungeon ambient identity |
| `FogDensity` | `0.008f` | Thinner fog now that ambient handles visibility |
| `FogHeightFalloff` | `0.2f` | Unchanged |
| `FogInscatteringColor` | `FLinearColor(0.55f, 0.7f, 0.95f, 1.0f)` | Matches ambient tint |
| `FogStartDistance` | `400.0f` | Unchanged |
| `FogCutoffDistance` | `20000.0f` | Unchanged |
| `ColorGradeShadowsTint` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` | Neutralized |
| `ColorGradeMidtonesTint` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` | Neutralized |
| `ColorGradeHighlightsTint` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` | Unchanged neutral |
| `ColorGradeSaturation` | `FVector4(0.95f, 0.95f, 0.95f, 1.0f)` | Gentle relax from Iteration 02 |
| `ColorGradeContrast` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` | Neutralized |
| `ColorGradeGain` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` | Unchanged |
| `TorchIntensity` | `700.0f` | Halved from Iteration 02 |
| `TorchAttenuationRadius` | `800.0f` | Unchanged |
| `TorchColor` | `FLinearColor(1.0f, 0.55f, 0.22f)` | Unchanged warm amber accent |
| `TorchFalloffExponent` | `2.0f` | Unchanged |
| `CarryLightIntensity` | `200.0f` | Reduced from Iteration 02 |
| `CarryLightAttenuationRadius` | `350.0f` | Tighter player-local pool |
| `CarryLightColor` | `FLinearColor(1.0f, 0.7f, 0.4f)` | Unchanged warm amber |
| `CarryLightFalloffExponent` | `1.5f` | Unchanged |
| `CarryLightVerticalOffset` | `60.0f` | Unchanged |

## 5. Stage 4 - Ambient Cubemap Apply Block

The ambient cubemap write lives on the existing theme PP volume path in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:403-453`, inside `T66ApplyThemePostProcess()`.

Exact applied block:

```cpp
// Ambient cubemap: primary ambient source for stylized indoor scenes.
UTextureCube* LoadedCubemap = Spec.AmbientCubemap.LoadSynchronous();
PPS.AmbientCubemap = LoadedCubemap;
PPS.AmbientCubemapIntensity = Spec.AmbientCubemapIntensity;
PPS.bOverride_AmbientCubemapIntensity = true;
PPS.AmbientCubemapTint = Spec.AmbientCubemapTint;
PPS.bOverride_AmbientCubemapTint = true;

const FString AppliedCubemapPath = PPS.AmbientCubemap
	? PPS.AmbientCubemap->GetPathName()
	: FString(TEXT("None"));
UE_LOG(
	LogT66WorldVisualSetup,
	Display,
	TEXT("[ATMOSPHERE] Ambient cubemap setup: path=%s loaded=%s intensity=%.2f tint=(%.3f, %.3f, %.3f) volume=%s applied=%s"),
	*Spec.AmbientCubemap.ToSoftObjectPath().ToString(),
	LoadedCubemap ? TEXT("yes") : TEXT("no"),
	Spec.AmbientCubemapIntensity,
	Spec.AmbientCubemapTint.R,
	Spec.AmbientCubemapTint.G,
	Spec.AmbientCubemapTint.B,
	*Volume->GetName(),
	*AppliedCubemapPath);
```

Engine field-name confirmation: `C:\Program Files\Epic Games\UE_5.7\Engine\Source\Runtime\Engine\Classes\Engine\Scene.h:787-790` defines `bOverride_AmbientCubemapTint` and `bOverride_AmbientCubemapIntensity`. `Scene.h:1820-1833` defines `AmbientCubemapTint`, `AmbientCubemapIntensity`, and `AmbientCubemap`, but no `bOverride_AmbientCubemap`; the code assigns `PPS.AmbientCubemap` directly and logs the applied path.

## 6. Stage 5 - SkyLight Decommission Status

`Source/T66/Gameplay/T66WorldVisualSetup.cpp:272-301` still find/configures the tagged SkyLight, preserves the strip allowlist architecture, and now logs the applied spec intensity/color. The Dungeon spec sets `SkyLightIntensity = 0.0f`, and staged runtime logs confirm `intensity=0.000 (decommissioned as ambient source)`.

No SkyLight code path was orphaned. `FT66WorldVisualSetup::EnsureAtmosphereForWorld()` still calls `T66ApplyAtmosphereSkyLight()` before fog/theme PP setup at `Source/T66/Gameplay/T66WorldVisualSetup.cpp:515-522`.

## 7. Stage 6 - Diagnostic Logging

Added SkyLight diagnostic log at `Source/T66/Gameplay/T66WorldVisualSetup.cpp:292-300`:

```cpp
[ATMOSPHERE] SkyLight setup: intensity=%.3f color=(%.3f, %.3f, %.3f) %s
```

Added ambient cubemap diagnostic log at `Source/T66/Gameplay/T66WorldVisualSetup.cpp:441-452`:

```cpp
[ATMOSPHERE] Ambient cubemap setup: path=%s loaded=%s intensity=%.2f tint=(%.3f, %.3f, %.3f) volume=%s applied=%s
```

The ambient log includes the extra `applied=%s` field to prove the theme PP volume's `Settings.AmbientCubemap` pointer was set after the write.

## 8. Build Log Excerpt

Build command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames
```

Log: `Saved/StandaloneLogs/Atmosphere_Iteration_03_Build.log`

Last 50 lines:

```text
LogIoStore: Display: Input:      2.42 GiB UExp
LogIoStore: Display: Input:      7.18 MiB UAsset
LogIoStore: Display: Input:      1.33 GiB UBulk
LogIoStore: Display: Input:    109.24 MiB for 1002 Global shaders
LogIoStore: Display: Input:      3.88 MiB for 438 Shared shaders
LogIoStore: Display: Input:         0   B for 0 Unique shaders
LogIoStore: Display: Input:     16.19 MiB for 465 Inline shaders
LogIoStore: Display:
LogIoStore: Display: Output:    78408 Name map entries
LogIoStore: Display: Output:     3743 Imported package entries
LogIoStore: Display: Output:     1791 Packages without imports
LogIoStore: Display: Output:        0 Public runtime script objects
LogIoStore: Display: Output:     4.98 MiB HeaderData
LogIoStore: Display: Output:     2.71 MiB InitialLoadData
LogIoStore: Display:
LogIoStore: Display: Success
LogPakFile: Display: UnrealPak executed in 5.914278 seconds
Took 6.41s to run UnrealPak.exe, ExitCode=0
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_UFSFiles.txt, NumItems: 8903
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFiles.txt, NumItems: 1562
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFilesDebug.txt, NumItems: 6
Copying NonUFSFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Copying DebugFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Stage command time: 13.39 s
********** STAGE COMMAND COMPLETED **********
********** PACKAGE COMMAND STARTED **********
Package command time: 0.00 s
********** PACKAGE COMMAND COMPLETED **********
BuildCookRun time: 138.90 s
BUILD SUCCESSFUL
AutomationTool executed for 0h 2m 19s
AutomationTool exiting with ExitCode=0 (Success)
Refreshed loose runtime root 'RuntimeDependencies/T66/Fonts/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\Fonts' (2 files).
Refreshed loose runtime root 'RuntimeDependencies/T66/Arcade/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\Arcade' (61 files).
Refreshed loose runtime root 'RuntimeDependencies/T66/UI/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI' (653 files).
Refreshed loose runtime root 'RuntimeDependencies/T66/Video/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\Video' (97 files).
Refreshed loose runtime root 'Content/Movies/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Movies' (99 files).
Refreshed loose runtime root 'Content/Mini/Data/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Mini\Data' (13 files).
Refreshed loose runtime root 'SourceAssets/Mini/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\Mini' (470 files).
Refreshed loose runtime root 'SourceAssets/ItemSprites/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\ItemSprites' (1 files).
Refreshed loose runtime root 'Content/TD/Data/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\TD\Data' (10 files).
Refreshed loose runtime root 'SourceAssets/TD/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\TD' (46 files).
Refreshed loose runtime root 'Content/Deck/Data/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Deck\Data' (11 files).
Refreshed loose runtime root 'SourceAssets/Deck/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\Deck' (26 files).
Refreshed loose runtime root 'Content/Idle/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Idle' (10 files).
Refreshed loose runtime root 'SourceAssets/Idle/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\Idle' (18 files).
Standalone build ready at 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe'.
Reset standalone GameUserSettings: C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\Config\Windows\GameUserSettings.ini (1920 x 1080, windowed).
Updated standalone shortcut 'C:\UE\T66\T66 Standalone.lnk' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe'.
Updated standalone shortcut 'C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe'.
```

Warnings observed: pre-existing `T66Mini.Build.cs` missing include-directory warning and pre-existing `r.Upscale.Quality` priority warning. No new build errors.

Shortcut verification: both `C:\UE\T66\T66 Standalone.lnk` and the taskbar pinned shortcut target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## 9. Runtime Log Excerpt

Launch-only smoke command targeted the staged `GameplayLevel` and wrote `Saved/StandaloneLogs/Atmosphere_Iteration_03_LaunchSmoke.log`. Screenshot artifact: `Saved/Codex/Atmosphere/Iteration03_GameplaySmoke.png`.

```text
[2026.05.16-04.55.25:114][  0]LogT66WorldVisualSetup: Display: [ATMOSPHERE] SkyLight setup: intensity=0.000 color=(1.000, 1.000, 1.000) (decommissioned as ambient source)
[2026.05.16-04.55.25:126][  0]LogT66WorldVisualSetup: Display: [ATMOSPHERE] Ambient cubemap setup: path=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap loaded=yes intensity=10.00 tint=(0.550, 0.700, 0.950) volume=PostProcessVolume_2147482251 applied=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap
[2026.05.16-04.55.25:138][  0]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 0/0 hero(es) (intensity=200.0 radius=350.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
[2026.05.16-04.55.25:264][  0]LogT66TowerLighting: [ATMOSPHERE] Spawned 24 Dungeon torch light(s) for floor 2 (intensity=700.0 radius=800.0 color=(R=1.000000,G=0.550000,B=0.220000,A=1.000000) falloff=2.00).
[2026.05.16-04.55.25:288][  0]LogT66TowerLighting: [ATMOSPHERE] Spawned 24 Dungeon torch light(s) for floor 3 (intensity=700.0 radius=800.0 color=(R=1.000000,G=0.550000,B=0.220000,A=1.000000) falloff=2.00).
[2026.05.16-04.55.25:311][  0]LogT66TowerLighting: [ATMOSPHERE] Spawned 24 Dungeon torch light(s) for floor 4 (intensity=700.0 radius=800.0 color=(R=1.000000,G=0.550000,B=0.220000,A=1.000000) falloff=2.00).
[2026.05.16-04.55.29:313][369]LogT66WorldVisualSetup: Display: [ATMOSPHERE] SkyLight setup: intensity=0.000 color=(1.000, 1.000, 1.000) (decommissioned as ambient source)
[2026.05.16-04.55.29:324][371]LogT66WorldVisualSetup: Display: [ATMOSPHERE] Ambient cubemap setup: path=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap loaded=yes intensity=10.00 tint=(0.550, 0.700, 0.950) volume=PostProcessVolume_2147482251 applied=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap
[2026.05.16-04.55.29:336][374]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=200.0 radius=350.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
```

## 10. Deferred Decisions

None. The only deviation from the prompt was engine-field naming: UE 5.7 has no `bOverride_AmbientCubemap`, so the cubemap pointer is assigned directly while intensity and tint use their local override booleans.

## 11. Open Questions for Pablo

None from implementation. Pablo's screenshot pass should decide whether `AmbientCubemapIntensity = 10.0f` and the cool tint are the right visual starting point.
