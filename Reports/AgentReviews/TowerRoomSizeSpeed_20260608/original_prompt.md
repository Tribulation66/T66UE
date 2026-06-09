Working task:
Operator: Codex
Validator: Claude
Scope: update floors 2 and 3 to keep the same 10-room count but double combat room size range to 10k-20k by 10k-20k, then report live Hero 1 base stats and per-level stat gains.
Stop condition: config/code/docs are updated as needed, the changed tower generation is verified as far as feasible, Hero 1 stats are answered from live data/code, and Claude validation is incorporated.

Original user request:
Ok lets keep the same number of rooms but double the size range, so 10k - 20k x 10k - 20k, also the speed need to be adjusted, tell me the base stats for hero 1 and how many stats he gains per level up for each stat.

Relevant repo rules:
- Do not use native goal tools.
- Codex is Operator and Claude is Validator per `.t66/operator-state.json`.
- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Gameplay runtime changes prefer data-authored tuning and require compile/build/staged verification when they affect playable standalone.
- World tuning lives in `Config/DefaultT66TowerTuning.ini`, `Source/T66/Core/T66TowerTuningConfig.*`, and `Gameplay/World/T66_TUNING_SURFACE.md`.
- Room-size tuning flows through `DungeonMinRoomTiles` / `DungeonMaxRoomTiles` and should stay in sync with `RoomRules.WidthTiles` / `HeightTiles`.
