You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\IdolCategoryFloorRestructurePlan\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\IdolCategoryFloorRestructurePlan\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\IdolCategoryFloorRestructurePlan\20260605T061520-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok great, now were going to move on to the idols but in doing so were going to make some big changes. So were going to change the Idol categories from 4 categories, AOE, Pierce, Bounce and DOT, to 3 categories, AOE, Bounce and Dot. So were getting rid of Pierce as a category, that means a lot of changes first, the secondary stats for pierce are done away with second, which affects steroids and items, just remove the pierce items, this will also remove in the idol selection screen how many idols are displayed it will only be 3 now per element. But we are adding a new element called wind, which will have 3 idols, an AOE, a Dot and a bounce, that way the total number of idols does not decraese and remains as 16. then were also going to reduce the amount of idols a hero can equip to 3 total instead of 4, for a total of 12 interactions with the idol altar per difficulty. Now this affects basically when idol altars show up so these are the changes we are making actually has to do with the floors, so instead of 5 floors total per stage we will change that to 4 floors total per stage. So imagine were cutting the previous floor 3, that way, we should be cutting exactly, 4 idol interacations and getting to 12. So let me know if im missing something, or anyquestions come up that I need to answer regarding making this sort of change, it is a big change which will include data changes, but I believe its for the best, after these changes are done we will move to making the placeholder vfx for all these idols but first give me an implementation plan and let me know of any questions or outstanding issues

Task contract:

Working task: Provide a repo-grounded implementation plan, questions, and outstanding issues for the idol category, Wind element, Pierce stat/item removal, idol slot count, idol altar cadence, and tower floor-count restructure.
Operator: Codex
Validator: Claude
Scope: Planning only. No code, CSV, asset, or data-table changes in this turn.
Stop condition: User-facing implementation plan plus concrete questions/outstanding issues, with verification/reporting of the planning evidence gathered.

Relevant current repo facts from Codex live inspection:

- `.t66/operator-state.json` sets Operator=Codex and Validator=Claude.
- `Source/T66/Core/T66IdolManagerSubsystem.h` currently has `MaxEquippedIdolSlots = 4` and `IdolStockSlotCount = 16`.
- `Content/Data/Idols.csv` currently has 16 idols as 4 elements x 4 categories: Fire/Ice/Electricity/Nature by DOT/AOE/Pierce/Bounce.
- `Source/T66/Data/T66DataTypes.h` currently has shared `ET66AttackCategory` values Pierce/Bounce/AOE/DOT, secondary stats PierceDamage/PierceSpeed/PierceScale, element enum Fire/Ice/Electricity/Nature, and element power stats FirePower/IcePower/ElectricityPower/NaturePower.
- `Content/Data/Items.csv` includes `Item_PierceDamage`, `Item_PierceSpeed`, and `Item_PierceScale`.
- `Content/Data/Heroes.csv` and `Content/Data/Weapons.csv` still use Pierce for Hero 2 and Hero 8 weapon branches.
- `Source/T66/Core/T66SaveMigration.h` currently normalizes equipped idol saves to 4 slots and maps several legacy IDs to current Pierce idols.
- `Source/T66/UI/T66IdolAltarOverlayWidget.h` has `OfferSlotCount = 16`, `OfferSlotsPerCategory = 4`, and `OfferCategoryCount = 5`.
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp` currently hard-codes normal tower floors as Floor 1 start, Floor 2-4 mob floors, Floor 5 boss.
- `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` documents the same five-floor live tower contract and says Floor 2-4 are gameplay floors.
- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` spawns placed miniboss guardians on every mob floor and spawns an idol altar when a tower gate guardian is defeated.
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp` still has a stage-entry idol altar path for local stage 4.
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp` maps 16 traveler visual profile slots with Pierce slots for each existing element.

Please produce an independent repo-grounded answer to the user: what implementation plan should be followed, what questions are blocking or important, and what outstanding issues/risk areas should be surfaced before implementation. Keep it planning-only.

</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
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

</codex_draft>
