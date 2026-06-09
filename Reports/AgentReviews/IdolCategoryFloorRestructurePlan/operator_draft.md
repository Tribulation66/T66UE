Operator draft plan for cross-review:

This is planning-only. No code/data edits should happen until the user answers the design decisions below.

Lead findings:

1. The active idol-count math is inconsistent as stated. Current live data is 4 elements x 4 categories = 16. Removing Pierce from all 4 existing elements leaves 12. Adding Wind with AOE/DOT/Bounce adds 3, which totals 15, not 16. If the target is "only 3 idol cards per element" and Wind is a fifth element, the active grid is 5 x 3 = 15. If 16 is mandatory, there needs to be one special/non-grid idol, a fourth Wind idol, or one element with an extra idol.
2. "Remove Pierce" has two possible scopes. Pierce is currently shared by idol categories, item stats, temporary buff stats, Hero 2 and Hero 8 weapon branches, combat code, VFX validation, and run summary serialization. If the change is idol/stat economy only, `ET66AttackCategory::Pierce` must remain for hero weapons while Pierce idols/items/buffs are removed from player-facing idol/stat progression. If the change is global, Hero 2 and Hero 8 weapons need redesign or reassignment first.
3. The live altar interaction math needs a proof pass before implementation. The current tower code has three placed-miniboss floors per stage (`Floor 2-4`), each guardian defeat spawns an idol altar, and the stage-entry altar only spawns on local stage 4. Cutting old floor 3 removes one guardian altar per local stage, but the live code does not obviously equal 16 interactions before the cut.

Implementation plan:

1. Lock the design decisions:
   - Resolve 15 vs 16 active idols.
   - Decide whether Pierce is removed globally or only from idols/items/buffs.
   - Define the exact replacement mapping for current Pierce idols and legacy alias IDs.
   - Decide whether Wind gets `WindPower` items/buffs/stat displays and what color Wind uses.
   - Define "idol interaction" for acceptance: altar actor count, selection budget, or actual selections taken.

2. Data model and save compatibility:
   - Add `Wind` to `ET66IdolElement`.
   - Add `WindPower` to `ET66SecondaryStatType` only if Wind gets element-power parity.
   - Remove or deprecate Pierce secondary stats from live pools; preserve enum values if needed for serialized compatibility.
   - Change idol save normalization from 4 slots to 3 slots, with explicit truncation/migration behavior for slot 4.
   - Update `T66MigrateLegacyIdolID` so every old alias and every soon-to-be-removed Pierce idol maps to a surviving canonical idol or is intentionally dropped.

3. Source data edits:
   - Update `Content/Data/Idols.csv`: remove current `*_Pierce` rows and add Wind rows.
   - Update `Content/Data/Items.csv`: remove `Item_PierceDamage`, `Item_PierceSpeed`, `Item_PierceScale`; add Wind-power items only if approved.
   - Update `Content/Data/CombatVFXBindings.csv` and traveler visual profile mappings for the new idol set.
   - If global Pierce removal is chosen, update `Content/Data/Heroes.csv` and `Content/Data/Weapons.csv` for Hero 2 and Hero 8.

4. Runtime stat/item/buff systems:
   - Remove Pierce from live secondary-stat offer pools, temporary buff/steroid selection pools, item bonus calculations, power-up naming, hero-selection stats, and smoke checks.
   - If `ET66AttackCategory::Pierce` remains for weapons, keep weapon-specific runtime accessors intact but stop offering Pierce as an item/buff/idol economy family.
   - Add Wind power handling wherever Fire/Ice/Electricity/Nature power currently exists.

5. Idol stock and UI:
   - Change equipped idol slots from 4 to 3.
   - Change idol altar offer layout from 4 cards per visible category to 3 cards per element/category grouping, after the count model is decided.
   - Update HUD idol slot widgets, run summary screens, leaderboard snapshots, backend parser/serializer defaults, tutorial single-offer behavior, and localization labels.

6. Tower floor and altar cadence:
   - Change tower constants from Start=1, Mob=2-4, Boss=5 to Start=1, Mob=2-3, Boss=4 if "cut old floor 3" means two gameplay floors.
   - Update descent-hole creation, guardian gating, guardian mob roster slot mapping, floor tags, minimap/current-floor logic, miasma floor anchors, chest/crate/totem floor rules, and map design docs.
   - Update idol altar spawning so the live number of accepted interactions per difficulty equals the user-approved target of 12. Do not assume the floor-count change alone proves that.

7. Reload/import after data edits:
   - Run the owning data-table reload scripts after source data changes: `SetupIdolsDataTable.py`, `SetupItemsDataTable.py`, `SetupPlayerExperienceDataTable.py`, `SetupCombatVFXBindingsDataTable.py`, and `SetupWeaponsDataTable.py`/`ImportHeroDataTable.py` if hero/weapon rows change.

8. Verification:
   - Focused compile for editor/game targets.
   - Data validation: no active `Pierce` idol rows/items in source CSVs; all new Wind rows load into runtime data; no stale missing DataTable rows.
   - Save/backend proof: old 4-slot saves normalize to 3, legacy aliases map safely, backend run summaries parse/serialize new elements/categories without falling back to Pierce defaults.
   - UI proof: idol altar screen shows the approved 3-card grouping and HUD/run summary show 3 equipped slots.
   - Gameplay proof: tower stage has 4 floors, old floor 3 is gone or intentionally collapsed, boss floor is 4, and measured idol interaction count per difficulty is 12 by the agreed definition.
   - Staged standalone refresh and shortcut target verification because this affects playable runtime.

Questions to ask the user:

1. Should active idols become 15 (5 elements x 3 categories), or is 16 mandatory? If 16 is mandatory, what is the 16th idol?
2. Is Pierce being removed globally, including Hero 2/Hero 8 weapon branches, or only from idol/item/buff/stat progression?
3. What should removed Pierce idols and legacy aliases map to in saves?
4. Should Wind have `WindPower` stat/items/buffs like Fire/Ice/Electricity/Nature, and what color should Wind use?
5. For altar cadence, should acceptance be based on actual selectable idol interactions, altar actors spawned, or selection budget?
