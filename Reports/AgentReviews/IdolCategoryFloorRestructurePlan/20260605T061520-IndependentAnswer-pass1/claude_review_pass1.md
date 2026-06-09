I have enough to produce the independent validator answer.

Result: OK

## Independent Answer

This is a well-scoped, planning-only request. The models can produce the plan internally; the structural repo facts in Codex's inspection are accurate to the live tree (verified: `MaxEquippedIdolSlots = 4`, `IdolStockSlotCount = 16` at `T66IdolManagerSubsystem.h:22,24`; `OfferSlotCount = 16 / OfferSlotsPerCategory = 4 / OfferCategoryCount = 5` at `T66IdolAltarOverlayWidget.h:36-38`; `Idols.csv` = 4 elements × {DOT,AOE,Pierce,Bounce}; tower floors hard-coded Start=1, Mob=2-4, Boss=5 at `T66TowerMapTerrain.cpp:58-62`). Recommend Codex present the plan with the math reconciliation as the lead open question.

**Suggested implementation plan (ordered by dependency):**

1. **Enum/data-type layer first** (`T66DataTypes.h`): add `Wind` to the element enum and `WindPower` to element power stats; remove `Pierce` from `ET66AttackCategory` and remove `PierceDamage/PierceSpeed/PierceScale` secondary stats. Everything downstream keys off these, so land this before touching data tables.
2. **Idol data** (`Idols.csv`): delete the 4 `*_Pierce` rows; add `Idol_Wind_AOE / Idol_Wind_DOT / Idol_Wind_Bounce`. Keeps total at 16 (4 elements × {DOT,AOE,Bounce} = 12, + Wind's 3 = 15… see math note below — confirm element/category grid).
3. **Items** (`Items.csv`): remove `Item_PierceDamage / Item_PierceSpeed / Item_PierceScale`; decide whether to add Wind-power items for parity.
4. **Heroes/Weapons** (`Heroes.csv`, `Weapons.csv`): Hero 2 and Hero 8 weapon branches reference Pierce — these must be re-pointed to a surviving category or reworked, or those heroes break.
5. **Slot/stock constants**: `MaxEquippedIdolSlots 4→3`; `IdolStockSlotCount 16→16` (stays, still 16 idols); altar widget `OfferSlotsPerCategory 4→3`, `OfferCategoryCount` per category model, `OfferSlotCount` recomputed.
6. **Save migration** (`T66SaveMigration.h`): normalize equipped saves 4→3 slots; map legacy Pierce idol IDs to a replacement (or drop). Required or old saves corrupt.
7. **Floor restructure** (`T66TowerMapTerrain.cpp` + `T66_MAP_DESIGN_REFERENCE.md`): collapse 5→4 floors; reconcile `T66GameMode_Tower.cpp` guardian/altar spawning and the stage-4-only stage-entry altar in `T66GameMode_WorldInteractables.cpp`.
8. **Traveler pool** (`T66OutgoingTravelerPoolSubsystem.cpp`): 16 visual profile slots currently include Pierce slots — remap to Wind.
9. UI/run-summary loops already key off `MaxEquippedIdolSlots`, but the hard-coded 4-slot flat summary in `T66RunSummaryScreen.cpp:~2148` needs manual fix.

## Evidence Checked

- `Content/Data/Idols.csv` — confirmed 16 rows, 4×{DOT,AOE,Pierce,Bounce}.
- `T66IdolManagerSubsystem.h:22,24`, `T66IdolAltarOverlayWidget.h:36-38` — slot/offer constants match Codex's facts.
- `T66TowerMapTerrain.cpp:58-62` — Start=1, FirstMob=2, LastMob=4, Boss=5, TotalFloorCount=5.
- `T66GameMode_Tower.cpp:178-217` — 4 local stages (`LocalStage` clamped 1..4), slot = `(LocalStage-1)*3 + GateIndex` → 3 gate guardians per stage = 12-mob roster.
- `T66GameMode_WorldInteractables.cpp:2075-2086` — stage-entry idol altar spawns **only on local stage 4, floor 1**.
- Note: several `Reports/AgentReviews/` audits reference older `=3 / =12 / OfferCategoryCount=4` values — those are **stale** prior-iteration snapshots; the live tree is at 4/16.

## Questions Or Blockers

These belong in Codex's plan as user questions, not as a NEEDS_USER stop:

1. **The floor/interaction math does not cleanly reconcile — this is the user's "am I missing something."** Repo shows 3 gate-guardian altars per mob floor set (floors 2-4) × 4 local stages = **12 gate altars per difficulty**, *plus* a stage-4-only floor-1 stage-entry altar (≈13), not 16. Cutting one mob floor (old floor 3) removes 1 altar/stage × 4 stages = **−4**, landing 8 gate altars (+ the stage-entry one). The user's "16 → 12" needs a precise definition of "interaction" (altar instances vs. per-altar `RemainingSelections` budget vs. equipped-slot count) before the arithmetic can be confirmed. **Ask the user to define "interaction," then verify against `TowerIdolSelectionsAtStageStart` / `RemainingSelections` budgets.**
2. Wind grid: 4 elements × 3 categories = 12, + Wind's 3 = 15, not 16. Confirm whether Wind also gets 4 categories, or whether the "remains 16" target is actually 15, or one existing element keeps an extra idol.
3. Hero 2 / Hero 8 Pierce weapon branches — replace with which category, or rework those weapons?
4. Legacy Pierce idols in saves — remap to a specific replacement idol, or drop and refund?

## Caveats

- I did not exhaustively trace the per-difficulty altar count to a single authoritative number (selection budgets vs. altar instances) — that's exactly what Question 1 needs resolved before the floor cut is committed.
- Cutting a floor changes terrain vertex/headroom layout and the map design reference contract; treat the boss-floor renumber (5→4) carefully so guardian/boss spawn tags don't desync.
- Idol cap and floor changes have a wide blast radius (HUD, run summary, save/snapshot, leaderboard serialization, traveler pool) per prior audits — sequence enum/data changes first so compile errors surface every dependent site.
