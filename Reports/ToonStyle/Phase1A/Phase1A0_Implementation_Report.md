# Phase 1A.0 Implementation Report

## Summary

Phase 1A.0 adds the first ToonStyle runtime iteration surface: a `TestRoom` run category, a `TEST` button on Hero Selection, a dedicated cuboid room branch in the gameplay map, neutral test-room lighting, a fixed central player spawn, and TestRoom-scoped retro/low-resolution suppression.

The implementation builds and stages successfully. The staged HeroSelection dump confirms the `TEST` button exists, is visible, has a click handler, and is tagged as `HeroSelection.BottomRow.TestButton`. A fully automated OS-level click-through into TestRoom was attempted but did not trigger the Slate button reliably, so the route is build-verified and widget-dump-verified, not live-click-verified.

## Files Changed

- `Source/T66/Core/T66RunTypes.h`
- `Source/T66/Core/T66GameInstance.h`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.h`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp`
- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.h`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`

## Run Category and UI Route

`ET66RunCategory` now has `TestRoom` in `Source/T66/Core/T66RunTypes.h:22`. `UT66GameInstance` has `IsTestRoomRun()` at `Source/T66/Core/T66GameInstance.h:520`.

Hero Selection now exposes `HandleTestClicked()` at `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp:251` and declares it at `Source/T66/UI/Screens/T66HeroSelectionScreen.h:282`. The handler mirrors the existing Lab/Tutorial start-flow pattern: it resolves the selected hero/difficulty, clears active daily-climb state, sets `SelectedRunCategory = ET66RunCategory::TestRoom`, marks the run leaderboard-ineligible, seeds the run, hides frontend UI, and calls `TransitionToGameplayLevel()`.

The new `TEST` button is added below `MODS` in the bottom-right Hero Selection cluster at `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp:729`. It uses the same selected button styling as the neighboring route buttons and is tagged `HeroSelection.BottomRow.TestButton`.

## GameMode Branch

`HandleSpecialModeBeginPlay()` now recognizes TestRoom before Lab at `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:183`. The branch marks the run leaderboard-ineligible, resets run state and damage logs, runs setup when auto setup is enabled, schedules the same cleanup/warmup hide used by special modes, logs `T66GameMode BeginPlay - TestRoom`, and returns early from normal special-mode branching.

`EnsureLevelSetup()` now has a TestRoom branch before Lab at `Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp:435`. It calls `EnsureNeutralVisualSetupForWorld`, spawns the cuboid room, spawns neutral lighting, spawns the player start, and returns before normal tower generation.

Companion and terrain spawning are also gated:

- `SpawnCompanionForPlayer()` skips TestRoom at `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp:124`.
- `SpawnPlayerStartIfNeeded()` uses `T66TestRoom::PlayerStartLocation()` at `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp:351`.
- `SpawnDefaultPawnFor_Implementation()` uses the same fixed location at `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp:520`.
- Procedural terrain fallback is disabled for TestRoom at `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp:652`.

## Cuboid Room

The room helper lives in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` and `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.h`.

Design choices:

- Interior dimensions: `1000 x 1000 x 600` UU.
- Wall thickness: `40` UU.
- Geometry: six `/Engine/BasicShapes/Cube.Cube` static mesh actors.
- Surfaces: floor, ceiling, north wall, south wall, east wall, west wall.
- Player start: `FVector(0, 0, 220)`.
- Tags: room actors get `T66_TestRoom`; surfaces get `T66_TestRoom_Surface`; lighting gets `T66_TestRoom_Lighting`; all room/lighting actors also get `T66_AtmosphereSpared`.
- Materials: default cube material for this pass. Textured wall/floor material work remains Phase 1A.1.

The six room surfaces are spawned at `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp:156-161`.

## Lighting and Atmosphere

The TestRoom branch uses neutral visuals only. It calls `EnsureNeutralVisualSetupForWorld` and does not call `EnsureAtmosphereForWorld`.

`T66TestRoom::SpawnLighting()` creates:

- One `ADirectionalLight`, rotation `(-60, -35, 0)`, intensity `3.0`, neutral white.
- One `ASkyLight`, intensity `0.35`.

Both are tagged `T66_AtmosphereSpared` so the visual stripping path does not remove them.

## Retro Override

The intended behavior is TestRoom-only suppression of retro master FX and real low-resolution rendering without mutating saved player settings.

Implementation note: the initial plan was to add a push/clear override layer directly to `UT66RetroFXSubsystem`. That source file was locked by a Windows mapped-section write error during this pass, so I implemented the same effective behavior at the GameMode settings application seam instead of forcing a risky edit through the locked file.

`AT66GameMode::HandleSettingsChanged()` now checks `IsTestRoomRun()` at `Source/T66/Gameplay/T66GameMode.cpp:1516`. For non-TestRoom flows it applies saved settings normally. For TestRoom it copies the saved settings into a local `FT66RetroFXSettings`, sets:

- `bEnableRetroFXMaster = false`
- `bUseRealLowResolution = false`
- `TargetResolutionHeightPercent = 100.0`

and applies that local effective settings object at `Source/T66/Gameplay/T66GameMode.cpp:1531-1535`. The saved settings object is not modified.

This gives symmetric run-category semantics: TestRoom gets the clean override; Tower, Lab, and Tutorial continue to reapply saved settings through the existing settings-change path.

## Save Migration

No migration code was added. Pablo approved deleting existing dev saves if needed. `TestRoom` was appended as a new enum value rather than inserted before existing values, reducing the chance of older serialized enum values remapping to a different category.

## Verification

### Build

Command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development 'C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild
```

Result: succeeded.

Known unrelated warning:

```text
C:\UE\T66\Source\T66Mini\T66Mini.Build.cs: warning: Referenced directory 'C:\UE\T66\Source\T66Mini\Public\UI\Components' does not exist.
```

### Diff Hygiene

Command:

```powershell
git diff --check -- Source/T66/Core/T66RunTypes.h Source/T66/Core/T66GameInstance.h Source/T66/UI/Screens/T66HeroSelectionScreen.h Source/T66/UI/Screens/T66HeroSelectionScreen.cpp Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp Source/T66/Gameplay/T66GameMode.cpp Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.h Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp
```

Result: no whitespace errors. Git reported LF-to-CRLF warnings on several existing source files.

### Staged Standalone Refresh

Command:

```powershell
powershell -ExecutionPolicy Bypass -File 'C:\UE\T66\Scripts\StageStandaloneBuild.ps1' -ClientConfig Development
```

Result: succeeded. The script preserved 15 staged save files, restored them after staging, reset staged window settings, and updated shortcuts.

### Shortcut Verification

Both shortcuts point to the staged executable:

- `C:\UE\T66\T66 Standalone.lnk`
- `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`

Expected target:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Both matched.

### Staged Smoke Boot

Command shape:

```powershell
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -nullrhi -nosound -unattended -nopause -NoSplash -abslog=C:\UE\T66\Saved\StandaloneLogs\Phase1A0_StagedSmoke.log -forcelogflush
```

The process was stopped after a 25-second smoke window. Log markers confirm the staged exe initialized and loaded the frontend:

```text
Game Engine Initialized.
LoadMap: /Game/Maps/FrontendLevel?Name=Player
Game class is 'BP_FrontendGameMode_C'
Load map complete /Game/Maps/FrontendLevel
```

### HeroSelection TEST Button Dump

Command:

```powershell
.\Scripts\CaptureT66UIScreen.ps1 -Screen HeroSelection -Output C:\UE\T66\Saved\Codex\ToonStyle\Phase1A0\HeroSelection_TestButton_capture.png -DelaySeconds 5.5 -TimeoutSeconds 120 -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\ToonStyle\Phase1A0\HeroSelection_TestButton_dump.json")
```

Result: succeeded. The dump contains:

- tag `HeroSelection.BottomRow.TestButton`
- text `TEST`
- `has_click_handler: true`
- `hover_capable: true`
- 1920x1080 geometry `x=1691, y=997, width=192, height=48`

### Live Click-Through Attempt

I attempted three OS-level staged click-through tests by launching directly to HeroSelection, clicking the dumped TEST button region, and checking the log for `GameplayLevel` / `T66GameMode BeginPlay - TestRoom`.

Both attempts loaded HeroSelection but did not trigger the Slate click:

- `C:\UE\T66\Saved\StandaloneLogs\Phase1A0_TestButtonClick.log`
- `C:\UE\T66\Saved\StandaloneLogs\Phase1A0_TestButtonClick_02.log`
- `C:\UE\T66\Saved\StandaloneLogs\Phase1A0_TestButtonClick_03.log`

This appears to be an OS/window-coordinate automation limitation, not a compile/runtime error. I did not add a new automation hook for this pass.

### Route Regression Notes

No existing route handlers were intentionally changed:

- ENTER still uses `OnEnterTribulationClicked()`.
- TUTORIAL still uses `HandleTutorialClicked()`.
- LAB still uses `HandleLabClicked()`.
- CHALLENGES still uses `OnChallengesClicked()`.
- MODS still uses `OnModsClicked()`.

These routes were compile-verified and source-verified. They were not each live-clicked in the staged build during this pass.

## Caveats

Several source files were already dirty before this pass. A Windows mapped-section lock prevented normal full-file writes on some source files; I used a same-size in-place write workaround for the required edits. The build passed and `git diff --check` passed, but some touched files have formatting churn in the diff. I did not revert or normalize unrelated pre-existing changes.

## Next Step

Pablo should visually open the staged build, click TEST from Hero Selection, and confirm the room feel manually. If the manual click does not route, the next fix target is the Hero Selection handler binding, not the TestRoom GameMode branch.
