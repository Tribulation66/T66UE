# T66 Atmosphere Iteration 04 Report

## 1. Summary

Iteration 04 keeps the Iteration 03 architecture intact, with PP Ambient Cubemap on the theme post-process volume as the ambient source and the SkyLight decommissioned at intensity `0.0`, but retunes Dungeon toward dim cool blue ambient plus bright warm lamp pools. The Dungeon spec now drops ambient cubemap intensity, deepens fog color/density, raises torch intensity/radius/warmth, disables the player carry-light through spec value `0.0`, and adds procedural torch placement knobs for height, spacing, separation, and per-floor cap. `T66TowerLighting` now consumes those placement fields and logs whether the new cap binds per floor.

## 2. Stage 1

New spec fields were added immediately after the torch light fields in `Source/T66/Gameplay/T66ThemeAtmosphereData.h:76-87`:

| Field | Default |
|---|---:|
| `TorchVerticalOffset` | `450.0f` |
| `TorchSpacingAlongWall` | `1800.0f` |
| `TorchMinSeparation` | `1400.0f` |
| `TorchMaxPerFloor` | `60` |

These are `UPROPERTY()` members like the rest of `FT66ThemeAtmosphereSpec`, so default-constructed non-Dungeon specs inherit the new placement defaults until Forest/Ocean/Martian/Hell atmosphere specs are authored.

## 3. Stage 2

Dungeon spec values are applied in `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:29-56`.

| Field | Iter 03 | Iter 04 | Rationale |
|---|---|---|---|
| `AmbientCubemapIntensity` | `10.0f` | `2.5f` | Drop hard so the world reads as dim cool blue, not bright |
| `AmbientCubemapTint` | `(0.55, 0.7, 0.95, 1.0)` | `(0.45, 0.6, 0.9, 1.0)` | Deeper, slightly more saturated cool blue |
| `FogDensity` | `0.008f` | `0.018f` | Thicker distance fade |
| `FogInscatteringColor` | `(0.55, 0.7, 0.95, 1.0)` | `(0.18, 0.26, 0.45, 1.0)` | Distant geometry fades toward atmospheric darkness |
| `FogStartDistance` | `400.0f` | `400.0f` | Unchanged |
| `FogCutoffDistance` | `20000.0f` | `20000.0f` | Unchanged |
| `TorchIntensity` | `700.0f` | `2000.0f` | Lamps now carry most visible-light work |
| `TorchAttenuationRadius` | `800.0f` | `900.0f` | Slightly wider warm pools |
| `TorchColor` | `(1.0, 0.55, 0.22, 1.0)` | `(1.0, 0.42, 0.10, 1.0)` | Warmer, deeper amber |
| `TorchFalloffExponent` | `2.0f` | `2.0f` | Unchanged |
| `TorchVerticalOffset` | n/a | `450.0f` | Wall-midpoint sconce height |
| `TorchSpacingAlongWall` | n/a | `1800.0f` | Tighter sampling for more lamps |
| `TorchMinSeparation` | n/a | `1400.0f` | Slightly relaxed rejection threshold |
| `TorchMaxPerFloor` | n/a | `60` | 2.5x previous cap |
| `CarryLightIntensity` | `200.0f` | `0.0f` | Removes player-centered light |
| `CarryLightAttenuationRadius` | `350.0f` | `350.0f` | Unchanged, irrelevant at zero intensity |
| `CarryLightColor` | `(1.0, 0.7, 0.4, 1.0)` | `(1.0, 0.7, 0.4, 1.0)` | Unchanged |
| `ColorGradeShadowsTint` | neutral | neutral | Unchanged |
| `ColorGradeMidtonesTint` | neutral | neutral | Unchanged |
| `ColorGradeSaturation` | `0.95` | `0.95` | Unchanged |
| `ColorGradeContrast` | `1.0` | `1.0` | Unchanged |
| `ColorGradeGain` | `1.0` | `1.0` | Unchanged |
| `SkyLightIntensity` | `0.0f` | `0.0f` | Unchanged, stays decommissioned |
| `AmbientCubemap` | `/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap` | unchanged | Iteration 03 packaged-safe cubemap path |

## 4. Stage 3

Torch placement now reads from `FT66ThemeAtmosphereSpec` in `Source/T66/Gameplay/T66TowerLighting.cpp`.

- Minimum separation: `T66IsFarEnoughFromExistingTorches()` now accepts `MinSeparation` and squares `Spec.TorchMinSeparation` via the caller path in `Source/T66/Gameplay/T66TowerLighting.cpp:98-105` and `Source/T66/Gameplay/T66TowerLighting.cpp:165`.
- Placement builder signature: `T66BuildTorchPositionsForFloor(const FFloor& Floor, const FT66ThemeAtmosphereSpec& Spec)` in `Source/T66/Gameplay/T66TowerLighting.cpp:114-116`.
- Along-wall spacing: `TorchSpacing = FMath::Max(1.0f, Spec.TorchSpacingAlongWall)` in `Source/T66/Gameplay/T66TowerLighting.cpp:122`, replacing the former `2800.0f` constant. Short-wall center fallback remains at `Source/T66/Gameplay/T66TowerLighting.cpp:143-151`.
- Per-floor cap: `MaxTorchLights = FMath::Max(0, Spec.TorchMaxPerFloor)` in `Source/T66/Gameplay/T66TowerLighting.cpp:123`, consumed by the outer and inner cap checks at `Source/T66/Gameplay/T66TowerLighting.cpp:127` and `Source/T66/Gameplay/T66TowerLighting.cpp:158`.
- Vertical offset: candidate Z now uses `Floor.SurfaceZ + Spec.TorchVerticalOffset` in `Source/T66/Gameplay/T66TowerLighting.cpp:164`, replacing the former `50.0f` floor-level offset.
- Spawn call: `SpawnFloorTorchLights()` resolves the spec and passes it into the placement builder at `Source/T66/Gameplay/T66TowerLighting.cpp:297-300`.
- Light values still come from the same spec at `Source/T66/Gameplay/T66TowerLighting.cpp:344-349`.
- Updated log line includes intensity/radius/color/falloff plus `vOffset`, `spacing`, `minSep`, `cap`, and `(CAP REACHED)` when `TorchPositions.Num() == Spec.TorchMaxPerFloor`: `Source/T66/Gameplay/T66TowerLighting.cpp:360-376`.

The `80.0f` wall-normal offset remains hardcoded at `Source/T66/Gameplay/T66TowerLighting.cpp:24` because it is geometric wall-depth clearance rather than a theme atmosphere knob.

## 5. Stage 4

No carry-light code changed. `FT66WorldVisualSetup::ApplyAtmosphereToHeroCarryLights()` still runs from `EnsureAtmosphereForWorld()` at `Source/T66/Gameplay/T66WorldVisualSetup.cpp:522`, and still applies `Carry->SetIntensity(Spec.CarryLightIntensity)` at `Source/T66/Gameplay/T66WorldVisualSetup.cpp:545`. With the Dungeon spec now setting `CarryLightIntensity = 0.0f`, the existing path disables the player-local point light without removing the component.

## 6. Build Log Excerpt

Build command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames
```

Log: `Saved/StandaloneLogs/Atmosphere_Iteration_04_Build.log`

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
LogPakFile: Display: UnrealPak executed in 9.348835 seconds
Took 9.87s to run UnrealPak.exe, ExitCode=0
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_UFSFiles.txt, NumItems: 8903
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFiles.txt, NumItems: 1562
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFilesDebug.txt, NumItems: 6
Copying NonUFSFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Copying DebugFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Stage command time: 17.28 s
********** STAGE COMMAND COMPLETED **********
********** PACKAGE COMMAND STARTED **********
Package command time: 0.00 s
********** PACKAGE COMMAND COMPLETED **********
BuildCookRun time: 141.04 s
BUILD SUCCESSFUL
AutomationTool executed for 0h 2m 21s
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

Build result: `BUILD SUCCESSFUL`, `AutomationTool exiting with ExitCode=0 (Success)`. The only warning in the warning summary is the pre-existing `r.Upscale.Quality` priority warning; no new build errors were introduced.

Shortcut verification: `C:\UE\T66\T66 Standalone.lnk` and the taskbar pinned `T66 Standalone.lnk` both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## 7. Runtime Log Excerpt

Launch-only smoke:

- Executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Map: `/Game/Maps/GameplayLevel`
- Log: `Saved/StandaloneLogs/Atmosphere_Iteration_04_LaunchSmoke.log`
- Screenshot: `Saved/Codex/Atmosphere/Iteration04_GameplaySmoke.png`
- Exit code: `0`

```text
610: [2026.05.16-05.20.10:556][  0]LogT66WorldVisualSetup: Display: [ATMOSPHERE] SkyLight setup: intensity=0.000 color=(1.000, 1.000, 1.000) (decommissioned as ambient source)
611: [2026.05.16-05.20.10:568][  0]LogT66WorldVisualSetup: Display: [ATMOSPHERE] Ambient cubemap setup: path=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap loaded=yes intensity=2.50 tint=(0.450, 0.600, 0.900) volume=PostProcessVolume_2147482251 applied=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap
612: [2026.05.16-05.20.10:579][  0]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 0/0 hero(es) (intensity=0.0 radius=350.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
623: [2026.05.16-05.20.10:705][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 60 Dungeon torch light(s) for floor 2 (intensity=2000.0 radius=900.0 color=(R=1.000000,G=0.420000,B=0.100000,A=1.000000) falloff=2.00 vOffset=450.0 spacing=1800.0 minSep=1400.0 cap=60) (CAP REACHED)
625: [2026.05.16-05.20.10:728][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 60 Dungeon torch light(s) for floor 3 (intensity=2000.0 radius=900.0 color=(R=1.000000,G=0.420000,B=0.100000,A=1.000000) falloff=2.00 vOffset=450.0 spacing=1800.0 minSep=1400.0 cap=60) (CAP REACHED)
627: [2026.05.16-05.20.10:751][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 60 Dungeon torch light(s) for floor 4 (intensity=2000.0 radius=900.0 color=(R=1.000000,G=0.420000,B=0.100000,A=1.000000) falloff=2.00 vOffset=450.0 spacing=1800.0 minSep=1400.0 cap=60) (CAP REACHED)
971: [2026.05.16-05.20.14:805][293]LogT66WorldVisualSetup: Display: [ATMOSPHERE] SkyLight setup: intensity=0.000 color=(1.000, 1.000, 1.000) (decommissioned as ambient source)
972: [2026.05.16-05.20.14:816][296]LogT66WorldVisualSetup: Display: [ATMOSPHERE] Ambient cubemap setup: path=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap loaded=yes intensity=2.50 tint=(0.450, 0.600, 0.900) volume=PostProcessVolume_2147482251 applied=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap
973: [2026.05.16-05.20.14:828][298]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=0.0 radius=350.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
```

Torch cap status:

- Floor 2: 60 torches, `(CAP REACHED)`
- Floor 3: 60 torches, `(CAP REACHED)`
- Floor 4: 60 torches, `(CAP REACHED)`

## 8. Deferred Decisions

None. Placeholder torch mesh and light-function flicker paths were left unchanged as requested.

## 9. Open Questions for Pablo

- All three gameplay floors hit the 60-torch cap. Pablo should decide from screenshots whether the cap should remain binding or increase after the first visual check.
