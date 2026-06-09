Result: OK

## Independent Answer

The revised design is coherent and resolves the main planning blockers from the earlier turn:

- **Pierce kept** — removes the churn/risk of deleting an `ET66AttackCategory` value (`Pierce` at `T66DataTypes.h:23`) and rebalancing three categories. The four categories Pierce/Bounce/AOE/DOT stay intact.
- **Idol source moved to gates** — decouples idol progression from `HandleTowerGateGuardianDefeated` (`T66GameMode_Tower.cpp`) and boss reward spawning (`T66GameMode_BossFlow.cpp`). Gate-adjacent pickup as the unlock token is a cleaner, more deterministic flow than RNG drops.
- **Wind as a 5th element** — math checks out: 5 elements × 4 categories = **20 idols + No Idol**, a clean superset of the current 4×4 grid (`ET66IdolElement` at `T66DataTypes.h:1031`). Grey color + full item/buff/stat parity mirrors the existing per-element pattern.
- **Interaction math** — 4 stages × 3 gate-idols = **12**, consistent with 4 floors per stage (3 inter-floor gates per stage). Self-consistent.

So: yes, it simplifies and unblocks. Below are the items Codex must nail down before implementation (most are internal; three are genuine user decisions).

**Implementation scope (Codex can resolve internally):**
1. Enum extension: append `Wind` to `ET66IdolElement` and `WindPower` to `ET66SecondaryStatType` (append at end to preserve serialized values — the file already follows append-for-compat convention, see comment at `:989`). Update `T66GetElementPowerStatType` (`:1047`) and the default fallthrough.
2. Constants: `IdolStockSlotCount` 16→20 (`T66IdolManagerSubsystem.h:24`); `MaxEquippedIdolSlots` stays 4 (user did not change equip count). UI: `OfferSlotCount` 16→20 and page/`OfferCategoryCount` handling so the No-Idol page survives the grid growth (`T66IdolAltarOverlayWidget.h`). Traveler pool 16→20 visual slots (`T66OutgoingTravelerPoolSubsystem.cpp`).
3. Terrain: rework hard-coded Start=1/Mob=2-4/Boss=5 in `T66TowerMapTerrain.cpp` for the 4-floor layout.
4. Remove/rewire idol spawning out of guardian-defeat and boss-reward paths; add gate-adjacent altar spawn (extend/replace `SpawnIdolAltarForPlayer`).
5. Wind data rows: items, buffs, vendor, VFX bindings, grey theming — parity sweep across all per-element tables.
6. Verify `T66MigrateLegacyIdolID` still maps legacy IDs and that new Wind IDs slot in without collision; No-Idol persistence preserved.

## Evidence Checked
- `T66DataTypes.h` — confirmed `ET66AttackCategory` (Pierce/Bounce/AOE/DOT, `:20-27`), `ET66IdolElement` Fire/Ice/Electricity/Nature (`:1031`), element-power stats + `T66GetElementPowerStatType` (`:996-1057`).
- `T66IdolManagerSubsystem.h` — confirmed `MaxEquippedIdolSlots = 4`, `IdolStockSlotCount = 16` (`:22,:24`).
- `original_prompt.md` — full user request and task contract (planning-only, stop = confirmation + concerns + verification plan).
- Other repo facts (terrain hard-codes, altar/guardian/boss spawn sites, traveler pool, altar widget counts) taken from Codex's live inspection; not independently re-read but internally consistent with the verified files.

## Questions Or Blockers
These three are genuine design decisions only the user can settle before implementation — Codex should surface them in the confirmation, not assume:
1. **Does "4 floors per stage" include the boss floor, or is the boss separate (4 traversable floors + a boss arena)?** This determines the new terrain layout (current 5 = Start+3 Mob+Boss).
2. **Do gate guardians/minibosses still exist as combat encounters,** with the idol now serving as the unlock token, or are guardians removed entirely? "No longer dropped by miniboss" describes the drop, not the guardian's fate.
3. **Is the first-floor altar one of the 3 gate idols, or an extra interaction?** The phrasing ("always on first floor" + "above floor gates") reads as the first gate being on floor 1 (giving exactly 3/stage), but confirm so the count stays 12.

## Caveats
- Planning-only turn; no code/CSV/data-table changes made or proposed for this turn.
- Verification plan to attach once scope is approved: clean build with the extended enums; save/load round-trip with Wind idols **and** a pre-Wind legacy save (migration boundary); altar UI renders 20 offers + No-Idol page; gate-unlock-on-pickup flow works and no orphaned idol spawns remain on guardian/boss paths; 4-floor terrain generates correctly across all 4 stages.
- Traveler pool, VFX bindings, and per-element item/buff tables are easy to miss in the Wind parity sweep — treat "find every place that enumerates the 4 elements" as an explicit grep checklist item.
