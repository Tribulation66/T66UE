Result: OK

## Independent Answer

The request is implementable and no decision is gated on the user — the one ambiguity (`5000 - 1000`) is already resolved by a sound assumption. But this is **not a config-only change**. Grounding in the live code (`Config/DefaultT66TowerTuning.ini`, `T66TowerMapTerrain.cpp`, `T66TrapSubsystem.cpp`, `T66GameMode_WorldInteractables.cpp`) shows the actual levers differ from what the prompt implies, and two of the four asks require code, not just `.ini` edits.

What maps cleanly to config:

- **Fixed 10 rooms on floors 2 & 3 only** → set `DungeonMinRooms=10` and `DungeonMaxRooms=10`. These already apply **only to mob floors**; start (floor 1) and boss (floor 4) are single absolute-size rooms (`T66TowerMapTerrain.cpp:3845/3883`, `StartRoomSquareSize`), so floors 1 & 4 are untouched automatically. Both generation paths yield exactly 10 with Min=Max (`:2359`, `:2800`). Caveat below on whether 10 includes the arrival room.
- **1000-unit tile, globally** → set `GridCellSize=1000`, `PlacementCellSize=1000`, and `GridDoorWidth=1000` (currently all 1300). Floors 1 & 4 use absolute units (`StartRoomSquareSize=6500`), so they don't shrink — consistent with "keep those mostly the same."
- **Combat room size 5000–10000 × 5000–10000, independent rolls** → the real lever is `DungeonMinRoomTiles`/`DungeonMaxRoomTiles` (currently 2/5), **not** `RoomRules.WidthTiles/HeightTiles`. With 1000-unit tiles, set `DungeonMinRoomTiles=5`, `DungeonMaxRoomTiles=10`. Width and height are already rolled independently from the same range (`:2766-2767`), so non-square rooms happen naturally. **Editing `RoomRules.WidthTiles` would be inert for combat sizing.**

What requires **code**, because the config fields are currently inert:

- **"Every room has 1–2 traps" and "every room has one interactable/NPC"** → `RoomRules.TrapSlots` and `RoomRules.NonTrapContentSlots` are read/sanitized in `T66TowerTuningConfig.cpp` but **consumed nowhere**. Trap placement is per-floor/per-trap-type (`T66TrapSubsystem.cpp` `BuildSpawnRequestsForFloor`, driven by `TrapPools` + trap-tuning spawn counts), not per-room. Delivering a per-room guarantee means adding code to consume the per-room slots, or accepting an approximation via floor-level counts. This is the main scope expansion to call out.
- **"1 vendor per floor for 2 & 3"** is **already enforced** in code (`T66GameMode_WorldInteractables.cpp:1417-1431`, `GuaranteedPerMobFloor`). No new work; just confirm it still holds after the room changes.

## Evidence Checked

- `Config/DefaultT66TowerTuning.ini` — current values (cell sizes 1300, DungeonMinRooms 15/20, DungeonMinRoomTiles 2/5, RoomRules with TrapSlots/NonTrapContentSlots all 0, TrapPools for floors 2/3, floor 4 empty).
- `T66TowerMapTerrain.cpp` — room-count source (`:2359`, `:2800`), combat room dimensions from `DungeonMin/MaxRoomTiles` (`:2766`), start/boss as absolute single rooms (`:3845`, `:3883`), RoomRuleID stored but slots unused.
- `T66TrapSubsystem.cpp` — traps placed per-floor via `TrapPools` + spawn-count tuning, no per-room slot consumption.
- `T66GameMode_WorldInteractables.cpp` — vendor-per-mob-floor already guaranteed.
- `T66TowerTuningConfig.cpp` — `TrapSlots`/`NonTrapContentSlots` only set/sanitized, never consumed.

## Questions Or Blockers

None require the user. Items for Codex to handle in the answer/implementation:

1. Decide whether per-room trap/content is implemented as real code (consume `TrapSlots`/`NonTrapContentSlots` per room) or approximated via floor-level counts — and state which, since config alone won't satisfy "every room."
2. Edit the correct size levers (`DungeonMin/MaxRoomTiles`), not `RoomRules.WidthTiles`.

## Caveats

- **10-count semantics:** the mob-floor room set seeds an arrival/start room (`:2801-2813`) that counts toward the total, so `DungeonMaxRooms=10` likely yields 9 combat + 1 arrival. If the user means 10 *combat* rooms, set 11 — flag the interpretation rather than silently picking.
- **Grid fit:** with 1000-unit tiles, 10 rooms up to 10×10 tiles must fit in `GridColumns/Rows=25` (25000 units). Tight; a generation smoke check is warranted, and grid dims may need a bump.
- **Floor 4 traps:** `TrapPools` for floor 4 is intentionally empty. "Every floor has a trap" conflicts with keeping floor 4 (boss) unchanged — the room-level trap rule only fires on mob floors anyway, so treat floors 2 & 3 as the trap scope.
- **Verification gate:** since at minimum the inert-field work touches C++, the strongest feasible check is a compile/build plus a generation/PIE smoke confirming 10 rooms, room dimensions in 5000–10000, one vendor per floor, and per-room trap/content counts — or a clear report if the build gate can't run in this environment.
