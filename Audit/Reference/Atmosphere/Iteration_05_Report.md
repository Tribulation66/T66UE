# T66 Atmosphere Iteration 05 Report

## 1. Summary

Iteration 05 keeps the Iteration 03/04 ambient architecture unchanged and retunes Dungeon torches to act as smaller, deeper red-orange fire pools. Torch placement now uses one torch per eligible maze wall box plus one torch per registered outer-shell wall segment, Start floor torch coverage is enabled, and Boss floor exclusion is logged explicitly. The staged Development standalone build succeeded, the staged executable launched without crash, floor 1 spawned torches, and no floor hit the new 200-light safety cap.

## 2. Stage 1 - Dungeon Spec Value Updates

Applied at `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:45-52`.

| Field | Iter 04 | Iter 05 applied | Notes |
|---|---:|---:|---|
| `TorchIntensity` | `2000.0f` | `500.0f` | Four-times reduction for higher density. |
| `TorchAttenuationRadius` | `900.0f` | `600.0f` | Tighter local pools. |
| `TorchColor` | `(1.0, 0.42, 0.10, 1.0)` | `(1.0, 0.30, 0.05, 1.0)` | Deeper ember red-orange. |
| `TorchFalloffExponent` | `2.0f` | `2.0f` | Unchanged. |
| `TorchVerticalOffset` | `450.0f` | `450.0f` | Unchanged. |
| `TorchMaxPerFloor` | `60` | `200` | Raised for per-wall placement. |
| `TorchSpacingAlongWall` | `1800.0f` | `1800.0f` | Field remains declared, no longer read by Iteration 05 placement. |
| `TorchMinSeparation` | `1400.0f` | `1400.0f` | Field remains declared, no longer read by Iteration 05 placement. |

All non-torch Dungeon values remain at Iteration 04 values: PP Ambient Cubemap intensity `2.5`, tint `(0.45, 0.6, 0.9, 1.0)`, fog density `0.018`, SkyLight intensity `0.0`, and carry-light intensity `0.0`.

## 3. Stage 2 - OuterShellWallBoxes Registry

Added `OuterShellWallBoxes` to `T66TowerMapTerrain::FFloor` at `Source/T66/Gameplay/T66TowerMapTerrain.h:111-115`.

Implementation note: the prompt specified `UPROPERTY()`, but `FFloor` is a plain namespace struct, not a reflected `USTRUCT`; existing arrays such as `MazeWallBoxes`, `DoorwayHeaderBoxes`, and `TrapEligibleWallBoxes` are also non-UPROPERTY fields. `OuterShellWallBoxes` follows the existing struct pattern to compile cleanly.

Shell registration was added around the existing shell visual spawn path:

- Segment helper: `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4595-4640`
- Shell spawn mutation and reset: `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4642-4676`
- Shell visual spawning remains the same boxes as before: `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4678-4735`
- Registry count log: `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4743-4753`
- Spawn loop now uses a mutable local floor copy so shell boxes populated during shell spawn are visible to torch placement: `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5839-5860`

Granularity decision: shell wall boxes are registered at generated-kit segment granularity when generated split wall visuals are active, using the same segment-count math as the generated wall visual path. This produced 64 shell boxes per floor in the smoke run, matching the finest useful torch-placement granularity without changing the visual wall output.

Smoke counts:

```text
[ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 1.
[ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 2.
[ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 3.
[ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 4.
[ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 5.
```

## 4. Stage 3 - Placement Rewrite

Rewritten in `Source/T66/Gameplay/T66TowerLighting.cpp`.

- Floor role string helper: `Source/T66/Gameplay/T66TowerLighting.cpp:98-110`
- Placement result with maze/shell counters and cap state: `Source/T66/Gameplay/T66TowerLighting.cpp:113-119`
- Outer-shell normal resolves from shell segment toward floor center: `Source/T66/Gameplay/T66TowerLighting.cpp:121-133`
- One-per-wall candidate append path: `Source/T66/Gameplay/T66TowerLighting.cpp:135-173`
- Placement source loops: `Source/T66/Gameplay/T66TowerLighting.cpp:175-202`
- Light values still read from `FT66ThemeAtmosphereSpec`: `Source/T66/Gameplay/T66TowerLighting.cpp:413-418`
- Updated diagnostic log with maze/shell breakdown: `Source/T66/Gameplay/T66TowerLighting.cpp:427-445`

Removed from the active placement algorithm:

- Along-wall stride sampling via `Spec.TorchSpacingAlongWall`
- Euclidean rejection via `Spec.TorchMinSeparation`
- Multi-sample loops per long wall

New behavior:

```text
Candidate walls = (TrapEligibleWallBoxes if non-empty else MazeWallBoxes) + OuterShellWallBoxes.
Each non-doorway wall box gets one torch at box center + wall-normal * 80 UU + Z * TorchVerticalOffset.
Placement stops at Spec.TorchMaxPerFloor and reports CAP REACHED if the cap binds.
```

## 5. Stage 4 - Floor Coverage Investigation

Floor role enum is defined at `Source/T66/Gameplay/T66TowerMapTerrain.h:14-19`.

Floor roles are assigned in `BuildLayout()` at `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5165-5171`:

- Floor 1 is `Start`.
- Floors 2-4 are `Gameplay`.
- Floor 5 is `Boss`.

Maze build still intentionally skips non-gameplay floors at `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3543-3551`, so Start floor does not get maze wall boxes from that path. However, shell wall spawning runs for every floor in `Spawn()` and now registers `OuterShellWallBoxes`, so floor 1 has shell wall data available for torches.

Root cause of missing floor 1: `SpawnFloorTorchLights()` previously skipped every floor whose role was not `Gameplay`. That filtered out Start floor even though the shell walls existed visually.

Fix applied in `Source/T66/Gameplay/T66TowerLighting.cpp:309-370`:

- Start and Gameplay floors are eligible.
- Boss floor remains skipped with reason `boss-floor`.
- Non-Dungeon themes and unsupported roles log explicit skip reasons.
- Floors with no eligible wall data log `no-eligible-wall-boxes`.

Smoke result:

- Floor 1 now receives torches: `100` total, `36` maze-source boxes and `64` shell boxes.
- Boss floor remains skipped explicitly.
- No floor hit `(CAP REACHED)`.

## 6. Build Log Excerpt

Build log: `Saved/StandaloneLogs/Atmosphere_Iteration_05_Build.log`.

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
LogPakFile: Display: UnrealPak executed in 6.174655 seconds
Took 6.68s to run UnrealPak.exe, ExitCode=0
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_UFSFiles.txt, NumItems: 8903
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFiles.txt, NumItems: 1562
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFilesDebug.txt, NumItems: 6
Copying NonUFSFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Copying DebugFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Stage command time: 15.44 s
********** STAGE COMMAND COMPLETED **********
********** PACKAGE COMMAND STARTED **********
Package command time: 0.00 s
********** PACKAGE COMMAND COMPLETED **********
BuildCookRun time: 175.87 s
BUILD SUCCESSFUL
AutomationTool executed for 0h 2m 56s
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

Build warnings observed:

- Pre-existing `Source/T66Mini/T66Mini.Build.cs` referenced missing `Source/T66Mini/Public/UI/Components`.
- Pre-existing cook warning: `r.Upscale.Quality` scalability write ignored because project setting priority already sets value `1`.

## 7. Runtime Log Excerpt

Launch smoke log: `Saved/StandaloneLogs/Atmosphere_Iteration_05_LaunchSmoke.log`.

Launch command exited with code `0`. Screenshot artifact was written to `Saved/Codex/Atmosphere/Iteration05_GameplaySmoke.png`.

```text
[2026.05.16-05.50.00:236][  0]LogT66WorldVisualSetup: Display: [ATMOSPHERE] SkyLight setup: intensity=0.000 color=(1.000, 1.000, 1.000) (decommissioned as ambient source)
[2026.05.16-05.50.00:249][  0]LogT66WorldVisualSetup: Display: [ATMOSPHERE] Ambient cubemap setup: path=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap loaded=yes intensity=2.50 tint=(0.450, 0.600, 0.900) volume=PostProcessVolume_2147482251 applied=/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap
[2026.05.16-05.50.00:261][  0]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 0/0 hero(es) (intensity=0.0 radius=350.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
[2026.05.16-05.50.00:379][  0]LogT66TowerMapTerrain: Display: [ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 1.
[2026.05.16-05.50.00:391][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 100 Dungeon torch light(s) for floor 1 (maze=36 shell=64, intensity=500.0 radius=600.0 color=(R=1.000000,G=0.300000,B=0.050000,A=1.000000) falloff=2.00 vOffset=450.0 cap=200)
[2026.05.16-05.50.00:414][  0]LogT66TowerMapTerrain: Display: [ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 2.
[2026.05.16-05.50.00:425][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 114 Dungeon torch light(s) for floor 2 (maze=50 shell=64, intensity=500.0 radius=600.0 color=(R=1.000000,G=0.300000,B=0.050000,A=1.000000) falloff=2.00 vOffset=450.0 cap=200)
[2026.05.16-05.50.00:449][  0]LogT66TowerMapTerrain: Display: [ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 3.
[2026.05.16-05.50.00:461][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 119 Dungeon torch light(s) for floor 3 (maze=55 shell=64, intensity=500.0 radius=600.0 color=(R=1.000000,G=0.300000,B=0.050000,A=1.000000) falloff=2.00 vOffset=450.0 cap=200)
[2026.05.16-05.50.00:484][  0]LogT66TowerMapTerrain: Display: [ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 4.
[2026.05.16-05.50.00:496][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Spawned 114 Dungeon torch light(s) for floor 4 (maze=50 shell=64, intensity=500.0 radius=600.0 color=(R=1.000000,G=0.300000,B=0.050000,A=1.000000) falloff=2.00 vOffset=450.0 cap=200)
[2026.05.16-05.50.00:520][  0]LogT66TowerMapTerrain: Display: [ATMOSPHERE] Registered 64 outer shell wall box(es) for floor 5.
[2026.05.16-05.50.00:530][  0]LogT66TowerLighting: Display: [ATMOSPHERE] Skipped torch placement for floor 5 (role=Boss, reason=boss-floor)
[2026.05.16-05.50.04:656][272]LogT66WorldVisualSetup: [ATMOSPHERE] Applied carry-light spec to 1/1 hero(es) (intensity=0.0 radius=350.0 color=(R=1.000000,G=0.700000,B=0.400000,A=1.000000) falloff=1.50 z=60.0).
```

Cap status: no runtime torch line included `(CAP REACHED)`.

Shortcut verification:

```text
Shortcut=C:\UE\T66\T66 Standalone.lnk
Target=C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
Matches=True
Shortcut=C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk
Target=C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
Matches=True
```

## 8. Deferred Decisions

- `TorchSpacingAlongWall` and `TorchMinSeparation` remain in `FT66ThemeAtmosphereSpec` but are intentionally unused by the Iteration 05 one-per-wall placement path.
- Boss floor torch placement remains excluded. The current code logs the skip rather than silently omitting floor 5.
- `OuterShellWallBoxes` is populated on the mutable floor copy used during terrain spawn and torch placement. This satisfies the current runtime placement path; if later systems need shell-wall queries outside spawn-time lighting, the registry should be moved into the persisted layout build data.
- No new pending issue entries were added.

## 9. Open Questions for Pablo

None from this pass. The next decision is visual: whether `500 / 600 / (1.0, 0.30, 0.05)` with the new floor counts lands the intended fire-pool density.
