# T66 Audio Infrastructure

Last updated: 2026-04-25

## Purpose

T66 routes gameplay and UI sound through a central, data-authored audio table instead of scattering asset paths through gameplay code. Runtime systems ask for stable event IDs, while designers can replace sound assets by regenerating or editing the data table.

## Runtime Shape

- `UT66AudioSubsystem` owns lookup, random selection, cooldowns, pitch and volume variance, 2D playback, location playback, and attached playback.
- `FT66AudioEventRow` is the DataTable row schema for event metadata and soft asset references.
- `/Game/Data/DT_AudioEvents` is the runtime table loaded by the subsystem.
- `Content/Data/AudioEvents.json` is the generated source file for the DataTable.
- `Content/Audio/HeltonPixelCombat` contains the imported SoundWave assets.
- `SourceAssets/Audio/HeltonPixelCombat/Selected` contains the selected source WAVs copied from the purchased packs.

The table stores sound references as `FSoftObjectPath` values. This keeps the table authorable and cook-aware without hard-loading every sound at startup.

## Current Coverage

The current generated table has 94 event rows and references 138 selected WAV assets.

Covered groups:

- UI: button hover, button click, confirm, cancel, invalid, tab change, menu open, menu close, notification, purchase, equip.
- Player: damage, death, heal, pickup, coin pickup, dash, jump, land, level up, ultimate ready.
- Heroes: unique primary attack events for each hero archetype.
- Ultimates: specific cast events for every `ET66UltimateType`, including scoped sniper ready and scoped sniper fire.
- Enemies: spawn, melee/ranged attack, family-specific hit, family-specific death.
- Bosses: spawn, hit, death, phase change, profile-specific projectile fire, profile-specific AOE warning, AOE impact.
- Traps and interactables: wall arrows, floor flame, spike patch, pressure plate, doors, chests, vendors, shrines.
- Minigame and run state: generic minigame start/success/fail, run start, stage clear, run complete.

## Adding Or Replacing Sounds

1. Put replacement packs in `Downloads` or update the zip paths in `Scripts/SetupAudioEventsDataTable.py`.
2. Adjust or add event mappings in `build_events()` inside `Scripts/SetupAudioEventsDataTable.py`.
3. Run:

```powershell
python Scripts\SetupAudioEventsDataTable.py
```

4. To import/reload Unreal assets and the DataTable, run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\SetupAudioEventsDataTable.py' -unattended -nop4 -nosplash
```

5. Build and stage:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development 'C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE
powershell -ExecutionPolicy Bypass -File Scripts\StageStandaloneBuild.ps1
```

## Cook Notes

`Config/DefaultGame.ini` already includes:

```ini
+DirectoriesToAlwaysCook=(Path="/Game/Audio")
+DirectoriesToAlwaysCook=(Path="/Game/Data")
```

That keeps the generated SoundWaves and `DT_AudioEvents` available in cooked and staged builds.

## Validation Snapshot

2026-04-25 validation:

- C++ editor build succeeded.
- Unreal DataTable import reported 0 problems.
- `Content/Audio/HeltonPixelCombat` contains 138 imported SoundWave assets.
- `SourceAssets/Audio/HeltonPixelCombat/Selected` contains 138 selected WAV files.
- `Saved/Cooked/Windows/T66/Content/Audio/HeltonPixelCombat` contains 138 cooked audio assets.
- Staged Development build succeeded at `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`.
- `git diff --check` reported no whitespace errors.

The Unreal import command can still exit nonzero after a handled BINKA decoder ensure, even when the Python script logs successful execution and the DataTable reload has 0 problems.

## References

- Epic documentation: [Data Driven Gameplay Elements](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-driven-gameplay-elements-in-unreal-engine?application_version=5.7).
- Epic documentation: [Referencing Assets](https://dev.epicgames.com/documentation/en-us/unreal-engine/referencing-assets-in-unreal-engine?application_version=5.7).
- Epic documentation: [Asynchronous Asset Loading](https://dev.epicgames.com/documentation/en-us/unreal-engine/asynchronous-asset-loading-in-unreal-engine?application_version=5.6).
