User request:

Ok regarding the remaining things yes it includes boss floor. 2. Yes its the first Descent gate altar. So basically the first floor will have the weapon altar and the idol altar. You need to select both and actually I want to add another option for the weapon altar. That says no Weapon, and that is always an option, but you have to select it or the weapon to unlock the floor gate, you cant just not select a weapon thats not the same thing as no weapon, we will make a VFX for this too, this is a new damage type where its neither, aoe, or pierce, or dot or bounce, its just single target. And will use white for its color. 3. Yes miniboss should still exist just not drop idol. 4. Yes no idol unlcoks gate. 5. Yes we do need to reduce idol slots to 3 now. Go ahead and make the changes and we will move on to making the idols vfx.

Task contract:

Working task: Implement the clarified idol/tower/gate flow: keep Pierce, add Wind with 4 idol categories and WindPower parity, set equipped idol slots to 3, expand idol stock/UI to 20 plus No Idol, change normal tower to 4 floors including boss floor, make the first floor contain both weapon and idol altars, make selecting a weapon or explicit No Weapon plus selecting an idol/No Idol unlock the first descent gate, make subsequent gate-adjacent idol altars unlock gates, keep minibosses but remove idol drops from minibosses, remove normal boss idol drops, add a No Weapon weapon-altar option that selects a new white single-target/no-category attack path.
Operator: Codex
Validator: Claude
Scope: Code/data/script/doc changes needed for the above, plus current compile/data reload/runtime verification. No Git operations, no production Niagara/VFX authoring for idol placeholder visuals in this task.
Stop condition: Changes are implemented and verification is attempted/reported; any blocker or skipped proof is documented.

Clarified safe assumptions:

- Four floors means `Floor 1 - Start`, `Floor 2`, `Floor 3`, and `Floor 4 - Boss`.
- `Floor 1` has the weapon altar and first descent-gate idol altar.
- The first descent gate requires both a weapon altar selection and an idol altar selection.
- "No Weapon" is an explicit weapon altar choice, not the same as ignoring the weapon altar.
- "No Weapon" should use a new single-target/none damage category and white presentation.
- "No Idol" should unlock the relevant gate.
- Minibosses remain as combat encounters but no longer spawn idol altars.
- Normal boss reward flow should not spawn idol altars.
- Equipped idol slots become 3.

Relevant repo state from live inspection:

- `.t66/operator-state.json` sets Operator=Codex and Validator=Claude.
- Root/folder instructions read: `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/World/WORLD_AGENTS.md`, `UI/UI_AGENTS.md`, `Reports/AGENTS.md`, `Gameplay/README.md`, `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`, `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md`, `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- Current tower constants in `Source/T66/Gameplay/T66TowerMapTerrain.cpp`: Start=1, FirstMob=2, LastMob=4, Boss=5.
- Current idol manager constants in `Source/T66/Core/T66IdolManagerSubsystem.h`: MaxEquippedIdolSlots=4, IdolStockSlotCount=16.
- Current idol altar UI constants in `Source/T66/UI/T66IdolAltarOverlayWidget.h`: OfferSlotCount=16, OfferSlotsPerCategory=4, OfferCategoryCount=5 where last page is No Idol.
- Current altar spawn sites include stage-entry `SpawnIdolAltarForPlayer`, guardian drop `HandleTowerGateGuardianDefeated`, and boss reward idol spawn in `T66GameMode_BossFlow.cpp`.
- Current element model has Fire/Ice/Electricity/Nature and FirePower/IcePower/ElectricityPower/NaturePower; Wind is absent.
- Current traveler visual slots are 4 elements x 4 categories; Wind slots are absent.
- Pending issues read; none directly block this scope.

Please provide an independent repo-grounded implementation review before Codex edits: likely files/systems to touch, major risks, and any implementation warnings. Result should be OK unless a user decision is still required.
