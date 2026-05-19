# T66 Atmosphere Iteration 02 Report

## 1. Summary

Atmosphere Iteration 02 moved Dungeon lighting values out of hardcoded torch and hero defaults and into `FT66ThemeAtmosphereSpec`. The Dungeon spec now targets bright cool ambient lighting with weaker warm torch pools and a subtler player carry-light: SkyLight intensity is raised to `3.0`, fog density is reduced to `0.015`, color grading is softened, torches read `1400 / 800 / warm orange` from the spec, and hero carry-lights are reapplied from the same spec during `EnsureAtmosphereForWorld()`.

## 2. Stage 1 - Spec Struct Changes

`Source/T66/Gameplay/T66ThemeAtmosphereData.h:54-78` adds these fields to `FT66ThemeAtmosphereSpec`:

| Field | Default |
|---|---:|
| `TorchIntensity` | `1400.0f` |
| `TorchAttenuationRadius` | `800.0f` |
| `TorchColor` | `FLinearColor(1.0f, 0.55f, 0.22f)` |
| `TorchFalloffExponent` | `2.0f` |
| `CarryLightIntensity` | `550.0f` |
| `CarryLightAttenuationRadius` | `450.0f` |
| `CarryLightColor` | `FLinearColor(1.0f, 0.7f, 0.4f)` |
| `CarryLightFalloffExponent` | `1.5f` |
| `CarryLightVerticalOffset` | `60.0f` |

The fields are `UPROPERTY()` members consistent with the existing sky, fog, and color-grading fields. The staged Development build succeeded, confirming the struct compiles with the new defaults.

## 3. Stage 2 - Dungeon Spec Values

`Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:26-57` now creates an explicit Dungeon spec through `T66MakeDungeonAtmosphereSpec()`.

| Field | Applied Value |
|---|---|
| `SkyLightColor` | `FLinearColor(0.65f, 0.78f, 1.0f)` |
| `SkyLightIntensity` | `3.0f` |
| `FogDensity` | `0.015f` |
| `FogHeightFalloff` | `0.2f` |
| `FogInscatteringColor` | `FLinearColor(0.35f, 0.50f, 0.75f, 1.0f)` |
| `FogStartDistance` | `400.0f` |
| `FogCutoffDistance` | `20000.0f` |
| `ColorGradeShadowsTint` | `FVector4(0.85f, 0.95f, 1.08f, 1.0f)` |
| `ColorGradeMidtonesTint` | `FVector4(0.95f, 0.97f, 1.02f, 1.0f)` |
| `ColorGradeHighlightsTint` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` |
| `ColorGradeSaturation` | `FVector4(0.9f, 0.9f, 0.9f, 1.0f)` |
| `ColorGradeContrast` | `FVector4(1.05f, 1.05f, 1.05f, 1.0f)` |
| `ColorGradeGain` | `FVector4(1.0f, 1.0f, 1.0f, 1.0f)` |
| `TorchIntensity` | `1400.0f` |
| `TorchAttenuationRadius` | `800.0f` |
| `TorchColor` | `FLinearColor(1.0f, 0.55f, 0.22f)` |
| `TorchFalloffExponent` | `2.0f` |
| `CarryLightIntensity` | `550.0f` |
| `CarryLightAttenuationRadius` | `450.0f` |
| `CarryLightColor` | `FLinearColor(1.0f, 0.7f, 0.4f)` |
| `CarryLightFalloffExponent` | `1.5f` |
| `CarryLightVerticalOffset` | `60.0f` |

Forest, Ocean, Martian, and Hell still return `NeutralSpec` at `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:64-69`. The existing pending issue for non-Dungeon theme atmosphere authoring remains valid.

## 4. Stage 3 - Torch Consumer Changes

`Source/T66/Gameplay/T66TowerLighting.cpp:294` resolves `const FT66ThemeAtmosphereSpec& Spec = T66ThemeAtmosphereData::GetSpecForTheme(Theme)` at floor torch spawn time.

The hardcoded torch block was replaced at `Source/T66/Gameplay/T66TowerLighting.cpp:337-345`:

- `SetIntensity(Spec.TorchIntensity)`
- `SetLightColor(Spec.TorchColor)`
- `SetAttenuationRadius(Spec.TorchAttenuationRadius)`
- `SetCastShadows(false)`
- `SetUseInverseSquaredFalloff(false)`
- `SetLightFalloffExponent(Spec.TorchFalloffExponent)`

Placement, flicker assignment, and placeholder mesh behavior were left unchanged. The floor log now includes the spec values at `Source/T66/Gameplay/T66TowerLighting.cpp:356-365`.

## 5. Stage 4 - Carry-Light Apply Function

`Source/T66/Gameplay/T66WorldVisualSetup.h:7-20` forward-declares `FT66ThemeAtmosphereSpec` and exposes `FT66WorldVisualSetup::ApplyAtmosphereToHeroCarryLights(UWorld* World, const FT66ThemeAtmosphereSpec& Spec)`.

`Source/T66/Gameplay/T66WorldVisualSetup.cpp:490-532` implements the function by iterating `TActorIterator<AT66HeroBase>`, finding each hero `CarryLight`, and applying:

- `Intensity = Spec.CarryLightIntensity`
- `AttenuationRadius = Spec.CarryLightAttenuationRadius`
- `LightColor = Spec.CarryLightColor`
- `LightFalloffExponent = Spec.CarryLightFalloffExponent`
- inverse squared falloff disabled
- shadows disabled
- relative Z offset = `Spec.CarryLightVerticalOffset`

`Source/T66/Gameplay/T66WorldVisualSetup.cpp:476-487` calls the function from `EnsureAtmosphereForWorld()` after SkyLight, fog, and theme post-process setup. This preserves the existing idempotent setup timing, so early calls can log `0/0` heroes and later scheduled calls apply to the spawned hero.

The constructor fallback in `Source/T66/Gameplay/T66HeroBase.cpp:111-120` remains unchanged.

## 6. Build Log Excerpt

Last 50 lines of `Saved/StandaloneLogs/Atmosphere_Iteration_02_Build.log`:

```text
LogIoStore: Display: Input:      2.39 GiB UExp
LogIoStore: Display: Input:      7.16 MiB UAsset
LogIoStore: Display: Input:      1.32 GiB UBulk
LogIoStore: Display: Input:    109.24 MiB for 1002 Global shaders
LogIoStore: Display: Input:      3.83 MiB for 430 Shared shaders
LogIoStore: Display: Input:         0   B for 0 Unique shaders
LogIoStore: Display: Input:     16.17 MiB for 463 Inline shaders
LogIoStore: Display:
LogIoStore: Display: Output:    78276 Name map entries
LogIoStore: Display: Output:     3727 Imported package entries
LogIoStore: Display: Output:     1784 Packages without imports
LogIoStore: Display: Output:        0 Public runtime script objects
LogIoStore: Display: Output:     4.97 MiB HeaderData
LogIoStore: Display: Output:     2.71 MiB InitialLoadData
LogIoStore: Display:
LogIoStore: Display: Success
LogPakFile: Display: UnrealPak executed in 9.950615 seconds
Took 10.55s to run UnrealPak.exe, ExitCode=0
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_UFSFiles.txt, NumItems: 8864
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFiles.txt, NumItems: 1562
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFilesDebug.txt, NumItems: 6
Copying NonUFSFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Copying DebugFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Stage command time: 18.80 s
********** STAGE COMMAND COMPLETED **********
********** PACKAGE COMMAND STARTED **********
Package command time: 0.00 s
********** PACKAGE COMMAND COMPLETED **********
BuildCookRun time: 157.27 s
BUILD SUCCESSFUL
AutomationTool executed for 0h 2m 38s
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

Build summary:

- `Saved/StandaloneLogs/Atmosphere_Iteration_02_Build.log:99` reports `Result: Succeeded`.
- `Saved/StandaloneLogs/Atmosphere_Iteration_02_Build.log:633` reports `Success - 0 error(s), 2 warning(s)`.
- The two warnings are `r.Upscale.Quality` scalability override warnings at `Saved/StandaloneLogs/Atmosphere_Iteration_02_Build.log:152` and `Saved/StandaloneLogs/Atmosphere_Iteration_02_Build.log:154`; no new compile or stage errors were introduced.
- `C:\UE\T66\T66 Standalone.lnk` and the taskbar `T66 Standalone.lnk` both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## 7. Runtime Log Excerpt

Launch-only staged smoke:

- Executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Map: `/Game/Maps/GameplayLevel`
- Log: `Saved/StandaloneLogs/Atmosphere_Iteration_02_LaunchSmoke.log`
- Screenshot: `Saved/Codex/Atmosphere/Iteration02_GameplaySmoke.png`
- Exit code: `0`

```text
611: [2026.05.16-04.23.25:270][  0]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 0/0 hero(es) (intensity=550.0 radius=450.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
614: [2026.05.16-04.23.25:305][  0]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 0/0 hero(es) (intensity=550.0 radius=450.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
620: [2026.05.16-04.23.25:373][  0]LogT66TowerLighting: [ATMOSPHERE] Spawned 24 Dungeon torch light(s) for floor 2 (intensity=1400.0 radius=800.0 color=(R=1.000000,G=0.550000,B=0.220000,A=1.000000) falloff=2.00).
622: [2026.05.16-04.23.25:396][  0]LogT66TowerLighting: [ATMOSPHERE] Spawned 24 Dungeon torch light(s) for floor 3 (intensity=1400.0 radius=800.0 color=(R=1.000000,G=0.550000,B=0.220000,A=1.000000) falloff=2.00).
624: [2026.05.16-04.23.25:419][  0]LogT66TowerLighting: [ATMOSPHERE] Spawned 24 Dungeon torch light(s) for floor 4 (intensity=1400.0 radius=800.0 color=(R=1.000000,G=0.550000,B=0.220000,A=1.000000) falloff=2.00).
966: [2026.05.16-04.23.29:476][289]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=550.0 radius=450.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
1004: [2026.05.16-04.23.29:925][382]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=550.0 radius=450.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
1005: [2026.05.16-04.23.29:938][385]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=550.0 radius=450.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
1006: [2026.05.16-04.23.32:501][873]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=550.0 radius=450.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
1008: [2026.05.16-04.23.34:507][217]LogWindows: FPlatformMisc::RequestExit(0, UGameEngine::HandleExitCommand)
1048: [2026.05.16-04.23.35:804][218]LogExit: Exiting.
```

The initial `0/0` hero carry-light logs are expected because the atmosphere setup runs before pawn spawn on early setup passes. Later setup passes apply to `1/1` hero after `HeroBase BeginPlay`.

## 8. Deferred Decisions

No requested implementation scope was deferred. Visual tuning remains Pablo's eye check from staged screenshots.

No new pending issue was added; the existing `Source/T66/Gameplay/pending_issues_Gameplay.md:52-57` entry for non-Dungeon theme atmosphere specs still covers Forest, Ocean, Martian, and Hell.

## 9. Open Questions for Pablo

- Confirm from staged dungeon screenshots whether `SkyLightIntensity=3.0`, `TorchIntensity=1400`, and `CarryLightIntensity=550` land the intended bright-cool ambient / subtle-warm-pool balance.
