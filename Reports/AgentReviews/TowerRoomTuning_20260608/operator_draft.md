# Operator Draft - Tower Room Tuning 2026-06-08

## Task Contract

- Operator: Codex
- Validator: Claude
- Scope: implement tower tuning changes for floors 2 and 3: fixed 10 rooms each, 1000-unit tiles, room sizes interpreted as 5000-10000 x 5000-10000 units with independent width/height rolls, one to two traps per combat room, one interactable/NPC per combat room, and one vendor per mob floor while preserving floor 1 and floor 4 as much as the global tile-size change allows.
- Stop condition: config/code/docs updated, staged runtime proof verifies floor 2/3 layout and room content/trap rules, and caveats are documented.

## Implemented

- `Config/DefaultT66TowerTuning.ini`
  - Global tile/cell/door unit set to `1000`.
  - Mob-floor grid adjusted to `35x35` after the original 25x25 grid could not reliably place ten 5-10 tile rooms.
  - Combat dungeon room count fixed at `DungeonMinRooms=10` and `DungeonMaxRooms=10`.
  - Combat room tiles set to `DungeonMinRoomTiles=5` and `DungeonMaxRoomTiles=10`.
  - `DefaultCombat` room rule now has `TrapSlots=(1,2)` and `NonTrapContentSlots=(1,1)`.
  - Start/boss room rules remain separate with 5x5/0 slot defaults.

- `Source/T66/Core/T66TowerTuningConfig.*`
  - Runtime fallback defaults updated to match the tuning file.
  - Room rules expose active trap/content slot defaults.

- `Source/T66/Gameplay/T66TowerMapTerrain.*`
  - Room records are retained in each generated mob floor.
  - Added `TryGetRoomSurfaceLocation` for room-local placement.
  - Added `[T66Proof][TowerRoomLayoutSummary]` runtime proof logs for mob floors.

- `Source/T66/Core/T66TrapSubsystem.cpp`
  - Room-level trap spawning consumes `RoomRules.TrapSlots`.
  - Floors with room trap rules use room-placeable obstacle traps instead of the older floor-level trap request path.
  - Per-slot placement retries across all eligible obstacle trap entries and has a limited fallback for the mandatory first trap in an uncovered room.
  - Added `[T66Proof][TowerRoomTrapSummary]` runtime proof logs.

- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`
  - Room-level content spawning consumes `RoomRules.NonTrapContentSlots`.
  - Each eligible combat room gets one content/NPC placement.
  - Vendor rule remains one vendor per mob floor for floors 2 and 3, with crate/chest/totem/loot-bag filler.
  - Added `[T66Proof][TowerRoomContentSummary]` runtime proof logs.

- `Gameplay/World/T66_TUNING_SURFACE.md`
  - Documented the current tuning surface and marked trap/content room rules as active consumers.

- `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - Documented the out-of-scope full-readiness lifecycle stress failure caused by shelved mob loot.

## Verification

- Targeted diff whitespace check passed for the changed files; only CRLF warnings were emitted.
- Focused editor build:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReload`
  - Result: succeeded on rerun; a transient unrelated HeroSelection Slate compile output showed stale `.WrapAt` text, but the source already contained `.WrapTextAt` and the subsequent build reported up to date.
- Standalone stage:
  - `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -EngineRoot "C:\Program Files\Epic Games\UE_5.7" -StageRoot C:\UE\T66\Saved\StagedBuilds`
  - Result: PASS. Staged exe and project/taskbar shortcuts were refreshed.
- Focused lifecycle proof:
  - `Scripts\RunLifecycleTransitionSmokeGate.ps1 -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -OutputRoot C:\UE\T66\Saved\LifecycleTransitionSmokeGate\TowerRoomTuning_20260608_1222 -Travels 1 -NoStress -TimeoutSeconds 420`
  - Result: PASS.
  - Proof markers:
    - Floor 2 layout PASS: `Rooms=10 Expected=10 Grid=35x35 Tile=1000 RoomTiles=5-10`.
    - Floor 3 layout PASS: `Rooms=10 Expected=10 Grid=35x35 Tile=1000 RoomTiles=5-10`.
    - Content PASS: `Floors=2 Rooms=20 ContentRooms=20 Vendors=2 ExpectedVendors=2`.
    - Trap floor 2 PASS: `Rooms=10 RoomsWithTrap=10 Desired=15 Spawned=15 ExpectedRange=10-20`.
    - Trap floor 3 PASS: `Rooms=10 RoomsWithTrap=10 Desired=13 Spawned=13 ExpectedRange=10-20`.
    - Vendor summary PASS: `Spawned=2 Expected=2`.
- Full staged readiness:
  - First attempt `Saved\StagedBuildReadiness\20260608_122309`: failed preflight drain because existing Unreal build/stage processes did not clear within 120 seconds.
  - Rerun `Saved\StagedBuildReadiness\20260608_122548`: staging, shortcut verification, frontend smoke, and durable save smoke passed. Lifecycle gate completed all 6 travels and repeatedly logged PASS for the new tower room layout/content/trap/vendor markers, but the suite failed because `stress_population.mob_loot_spawned` was `0` instead of `6`.
  - Attribution: current dirty mob-loot subsystem has mob loot disabled by default and gated behind `FT66ShelvedFeatureGate::IsMobLootEnabled()`. This is outside the room-tuning scope and is documented as a pending issue.

## Caveats

- The user text said `(5000 - 10000) x (5000 - 1000)`. I treated the second range as a typo and implemented 5000-10000 by 5000-10000, with width and height rolled independently.
- The fixed count of 10 is total room records per mob floor, including the arrival/start room record. Claude independently called out this interpretation.
- No floor 4 trap/content expansion was made; the trap/content rule is scoped to floors 2 and 3 to honor the user's "no change to floor 1 and 4" constraint.
