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
