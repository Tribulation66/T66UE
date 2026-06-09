Result: OK

## Independent Answer

This is a pre-edit implementation review. The user has already made every decision in this turn (boss floor included, No Weapon as explicit white single-target choice, No Idol unlocks gate, miniboss keeps combat but drops no idol, 3 equipped slots). Nothing is left for the user to decide, so Codex can proceed. Below is the repo-grounded map of what to touch and where the bugs will hide.

**Two distinct enums — do not conflate them.** The work spans two separate axes that both happen to live in `T66DataTypes.h`:
- Weapon side: `ET66AttackCategory` (line 21) = `Pierce/Bounce/AOE/DOT`. The user's "new damage type, neither aoe/pierce/dot/bounce, single target, white" is a **5th `ET66AttackCategory`** (e.g. `SingleTarget`/`None`) used by the **No Weapon** altar choice.
- Idol side: `ET66IdolElement` (line 1031) = `Fire/Ice/Electricity/Nature`. The "add Wind" item adds a **5th element** plus a `WindPower` secondary stat. These are unrelated to the No-Weapon attack category.

**Files / systems to touch:**
1. `T66DataTypes.h` — add Wind to `ET66IdolElement`; add `WindPower` to `ET66SecondaryStatType`; add Wind case to `T66GetElementPowerStatType` (line 1047, default currently returns `FirePower` — without a case Wind silently maps to Fire); add the new white single-target value to `ET66AttackCategory`.
2. `T66IdolManagerSubsystem.h` — `MaxEquippedIdolSlots` 4→3; `IdolStockSlotCount` 16→20 (5 elements × 4). Check `RestoreState`/`NormalizeEquippedArrays`: existing saves hold 4 equipped idols and must trim to 3 without losing/crashing.
3. `T66IdolAltarOverlayWidget.h` — `OfferSlotCount` 16→20, `OfferCategoryCount` 5→6 (5 element pages + No Idol page). Keep `OfferSlotsPerCategory=4`. The No-Idol page index and category-paging math (`ActiveOfferCategoryIndex`, `GetOfferStockIndexForVisibleSlot`) are the off-by-one hot spot.
4. `T66TowerMapTerrain.cpp` — floor constants currently Start=1/FirstMob=2/LastMob=4/Boss=5 → 4 floors: Start=1/FirstMob=2/LastMob=3/Boss=4. `T66TowerTotalFloorCount` derives from these. Audit anything iterating floors (descent holes, theme/lighting per floor).
5. Altar spawn / gate flow — `T66WeaponAltar.*`, `T66IdolAltar.*`, `SpawnIdolAltarForPlayer`, `T66GameMode_Tower.cpp`, `T66TowerDescentHole.cpp`: Floor 1 must spawn **both** weapon altar and the first descent-gate idol altar; gate-unlock predicate must require a weapon selection AND an idol selection, treating explicit "No Weapon"/"No Idol" as valid selections distinct from "unselected."
6. Idol-drop removals — `HandleTowerGateGuardianDefeated` (miniboss; keep the encounter, drop only the idol) and the boss reward spawn in `T66GameMode_BossFlow.cpp`.

## Evidence Checked
- `T66IdolManagerSubsystem.h` (MaxEquippedIdolSlots=4, IdolStockSlotCount=16, RestoreState/NormalizeEquippedArrays present).
- `T66IdolAltarOverlayWidget.h` (OfferSlotCount=16, OfferSlotsPerCategory=4, OfferCategoryCount=5).
- `T66TowerMapTerrain.cpp` floor constants and `T66TowerTotalFloorCount` derivation.
- `T66DataTypes.h`: `ET66AttackCategory` (Pierce/Bounce/AOE/DOT), `ET66IdolElement` (Fire/Ice/Electricity/Nature), `T66GetElementPowerStatType` switch with FirePower default.
- Presence of `T66WeaponAltar.*`, `T66IdolAltar.*`, `T66GameMode_Tower.cpp`, `T66TowerDescentHole.cpp` via grep.

## Questions Or Blockers
None requiring the user. All decisions are settled in the prompt.

## Caveats
- **Silent-default switches are the top risk.** Every `switch` over `ET66IdolElement` and `ET66AttackCategory` (power stat, VFX binding tables, colors, combat resolution) needs the new Wind / single-target case added; missing cases fall through to defaults and pass compile while behaving wrong at runtime. Grep all switches before claiming done.
- **Save migration:** dropping equipped slots 4→3 and growing stock 16→20 touches persisted arrays — verify a pre-existing save loads without crash and trims gracefully (`T66SaveMigration` may need a bump).
- **Count constants must move as a set:** `IdolStockSlotCount`, `OfferSlotCount`, `OfferCategoryCount` and the No-Idol page index are coupled; a mismatch yields blank cards or out-of-range access rather than a compile error.
- **Stage/difficulty mapping:** `GetDifficultyStartStage/EndStage` in the idol manager keys off stage numbers; confirm the 5→4 floor change doesn't shift those boundaries.
- Per scope, the white No-Weapon attack should use a **placeholder** visual only — no production Niagara authoring this task. The combat path must still handle a category with no AOE radius / pierce / bounce / DOT semantics.
- I did not open the gate-unlock predicate body or `T66GameMode_BossFlow.cpp` line-by-line; Codex should confirm the exact unlock condition and the boss/miniboss drop call sites before editing.
