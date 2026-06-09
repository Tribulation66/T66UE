# Operator Draft - Tower Room Size And Hero 1 Stats

## Task

Update the current room tuning from the prior 5k-10k interpretation to 10k-20k by 10k-20k while keeping the same room count. Report Hero 1 base stats and per-level stat gains, especially speed, from live repo data.

## Changes

- `Config/DefaultT66TowerTuning.ini`
  - `GridColumns=70`
  - `GridRows=70`
  - `DungeonMinRoomTiles=10`
  - `DungeonMaxRoomTiles=20`
  - `DefaultCombat` room rule now has `WidthTiles=(Min=10,Max=20)` and `HeightTiles=(Min=10,Max=20)`.
- `Source/T66/Core/T66TowerTuningConfig.h`
  - fallback defaults now mirror the same 70x70 grid and 10-20 room tile range.
- `Gameplay/World/T66_TUNING_SURFACE.md`
  - current defaults table now documents 70/70 grid and 10/20 room tile range.
- `Source/T66/Core/pending_issues_Core.md`
  - added a pending issue for the out-of-scope staged readiness durable-save failure observed during this pass.

Note: the tuning files above are untracked in the current checkout, but they are live local files and were consumed by the staged packaged proof.

## Verification

- `rg -n "GridColumns|GridRows|DungeonMinRoomTiles|DungeonMaxRoomTiles|DefaultCombat|10 / 20|70 / 70" Config\DefaultT66TowerTuning.ini Source\T66\Core\T66TowerTuningConfig.h Gameplay\World\T66_TUNING_SURFACE.md`
  - confirmed live values in all three tuning locations.
- `git diff --check -- Config\DefaultT66TowerTuning.ini Source\T66\Core\T66TowerTuningConfig.h Gameplay\World\T66_TUNING_SURFACE.md Source\T66\Core\pending_issues_Core.md`
  - no whitespace errors; warning only that Git would convert LF to CRLF in `Source/T66/Core/pending_issues_Core.md`.
- `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReload`
  - PASS.
- `& .\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -EngineRoot 'C:\Program Files\Epic Games\UE_5.7' -StageRoot 'C:\UE\T66\Saved\StagedBuilds'`
  - PASS; staged exe ready at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`; standalone shortcuts repointed.
- Focused packaged lifecycle proof:
  - Command output root: `C:\UE\T66\Saved\LifecycleTransitionSmokeGate\TowerRoomSizeSpeed_20260608_20260608_140420`
  - Summary status: PASS.
  - `TowerRoomLayoutSummary`:
    - Floor 2 PASS, Rooms=10, Expected=10, Grid=70x70, Tile=1000, RoomTiles=10-20.
    - Floor 3 PASS, Rooms=10, Expected=10, Grid=70x70, Tile=1000, RoomTiles=10-20.
  - `TowerRoomContentSummary`: PASS, Floors=2, Rooms=20, ContentRooms=20, Vendors=2, ExpectedVendors=2.
  - `TowerRoomTrapSummary`: Floor 2 PASS and Floor 3 PASS, Rooms=10, RoomsWithTrap=10, Desired=15, Spawned=15, ExpectedRange=10-20.
  - `VendorPerFloorSummary`: PASS, Spawned=2, Expected=2.
- Broad staged readiness:
  - Output root: `C:\UE\T66\Saved\StagedBuildReadiness\20260608_140504`
  - Overall FAIL.
  - Stage PASS; shortcut verification PASS; frontend smoke PASS.
  - Durable save integrity failed before lifecycle ran: slot 8 queue phase logged `[SaveIntegrity] FAIL` because `MetaMap=T66_SaveIntegrity_DurableGate_20260608_140901` matched the marker but `LoadedMap=T66_SessionLoadedTravel_SessionLoadedTravel_20260608_031616` remained stale.
  - This was documented as an out-of-scope Core pending issue.

## Hero 1 Stats From Live Data

Source: `Content/Data/Heroes.csv`, row `Hero_1` / `Founding Chad`.

Base primary stats:

- Damage: 3
- AttackSpeed: 3
- AttackScale: 2
- AccuracyStat: 2
- Armor: 3
- Evasion: 2
- Luck: 3
- Speed: 2

Per-level primary gain ranges:

- Damage: 0.5-1.0
- AttackSpeed: 0.3-0.5
- AttackScale: 0-0
- Accuracy: 0.2-0.4
- Armor: 0.3-0.5
- Evasion: 0.2-0.4
- Luck: 0.3-0.5
- Speed: 0.2-0.4

Additional Hero 1 authored values relevant to tuning:

- MaxSpeed: 1680
- AccelerationPercentPerSecond: 20
- PrimaryCategory: AOE
- BaseFireInterval: 1.0
- BaseAttackRange: 2500
- BaseHitDamage: 22
- ProjectileSpeed: 2400
- AoeDelay: 0.2
- AoeRadius: 300
- BaseCritChance: 0.08

Live code notes:

- `Source/T66/Core/T66GameInstance.cpp:1888-1908` reads base stats and per-level gain ranges from hero data.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp:537-560` rolls/apply per-level gains for each primary stat, including Speed.
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp:21,104-106` converts speed stat to walk speed as `Max(1, SpeedStat) * 840`.
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp:244-245` says foundational Speed owns base locomotion and `MaxSpeed` is reserved for future cap semantics.

## Draft Answer Position

The room-size change is ready for tuning review: floors 2 and 3 now generate 10 rooms each at 10k-20k by 10k-20k world units, with trap/content/vendor rules still passing in packaged proof.

I did not change Hero 1 speed values yet because the user asked for current base stats and gains first. Current effective authored base Speed is 2, and the current movement conversion makes that 1680 walk-speed units before multipliers.
