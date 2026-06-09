User request:

Ok so actually lets do the following, were not going to remove pierce, were keeping pierce, however we are going to remove the number of floors to 4 floors per stage instead of 5. And were changing the locations for the idol altars so idol altars will always show up in the first floor, of every stage. Then it will show up only above the floor gates so instead of it dropping from the miniboss it will be right above the floor stage and actually interacting and picking up an idol unlocks the floor gate and you can go down to the next floor, and it will no longer be dropped by the stage boss. That way we have 4 stages and 3 interactions per stage for a total of 12 total interactions. However I do want to add wind, with 4 idols, pierce, aoe, dot and bounce, that way we will have 20 total idols + no idol option. And yes Wind should get items steroids, like other elements wind should use grey color. I think this answers everything and simplifies things a lot, let me know.

Task contract:

Working task: Confirm and refine the revised implementation scope for keeping Pierce, adding Wind with four idol categories and Wind stat/item/buff parity, changing tower stages to four floors, and moving idol altar progression to gate-adjacent unlock interactions.
Operator: Codex
Validator: Claude
Scope: Planning/confirmation only. No code, CSV, asset, or data-table changes in this turn.
Stop condition: User-facing confirmation of whether the revised design resolves the earlier blockers, plus remaining implementation concerns and verification plan.

Relevant current repo facts from Codex live inspection:

- `.t66/operator-state.json` sets Operator=Codex and Validator=Claude.
- `Source/T66/Core/T66IdolManagerSubsystem.h` currently has `MaxEquippedIdolSlots = 4` and `IdolStockSlotCount = 16`.
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp` currently hard-codes Start=1, Mob=2-4, Boss=5.
- Current altar spawn sites include `SpawnIdolAltarForPlayer` in `T66GameMode_WorldInteractables.cpp`, guardian drop spawning in `HandleTowerGateGuardianDefeated` in `T66GameMode_Tower.cpp`, and boss reward idol spawning in `T66GameMode_BossFlow.cpp`.
- `Source/T66/Data/T66DataTypes.h` currently has `ET66IdolElement` Fire/Ice/Electricity/Nature and element power stats FirePower/IcePower/ElectricityPower/NaturePower.
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp` currently maps 16 traveler visual slots for 4 elements x 4 categories.
- `Source/T66/UI/T66IdolAltarOverlayWidget.h` currently has `OfferSlotCount = 16`, `OfferSlotsPerCategory = 4`, `OfferCategoryCount = 5` where the last page is No Idol.
- Current memory/live context says the current idol model is 16 idols, 4 equipped slots, elemental-power stats, and No Idol persistence; `T66MigrateLegacyIdolID` remains the compatibility boundary for legacy idol IDs.

Please independently review this revised plan and answer whether it resolves the earlier planning blockers. Surface any remaining design questions, implementation risks, and proof requirements. Keep it planning-only.
