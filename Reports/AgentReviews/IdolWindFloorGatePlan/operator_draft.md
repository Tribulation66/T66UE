Operator draft:

Yes, the revised design resolves the big blockers.

Confirmed scope:
- Keep Pierce as a live category.
- Add Wind as a fifth element with Pierce, AOE, DOT, and Bounce idols.
- Resulting active idol grid is 5 elements x 4 categories = 20 idols, plus No Idol.
- Add WindPower parity for items, steroids/temporary buffs/relics, stat display, backend summary, loot/boost pools, localization, and combat elemental-power scaling.
- Wind uses grey.
- Keep MaxEquippedIdolSlots at 4 unless the user later asks to change it.
- Change tower layout from 5 floors to 4 floors per stage. Concrete likely target: Floor 1 start/gate altar, Floor 2 gameplay/gate altar, Floor 3 gameplay/gate altar, Floor 4 boss.
- Idol altars become progression gates: selecting an idol or No Idol unlocks the next descent gate.
- Remove idol altar spawning from miniboss/guardian death and boss reward paths.

Remaining precision questions:
1. Does "4 floors per stage" mean Floor 4 is the boss floor, or four traversable floors plus a separate boss arena?
2. Do gate guardians/minibosses still exist as combat encounters, just without dropping idols, or are they removed from the floor-gate loop entirely?
3. Should selecting No Idol also unlock the gate? I recommend yes, otherwise No Idol can soft-block progression.

Implementation risks:
- Every hard-coded 4-element loop needs Wind added.
- Idol offer UI currently uses 16 offers + No Idol page constants; this becomes 20 offers + No Idol.
- Traveler visual slots and CombatVFXBindings need four Wind placeholder slots before idol VFX work.
- Save/backend summary needs Wind parse/serialize and legacy save proof.
- Boss reward code still spawns an idol altar today and must be removed or gated off for normal stage clears.
- Guardian defeat code currently spawns an idol altar and must be removed or repurposed.

Verification plan:
- Data reloads for idols/items/combat VFX bindings and any player-experience/tower data touched.
- Compile editor/game targets.
- Run save/backend round-trip with old 4-element saves and new Wind idols.
- Capture idol altar UI showing 20 offers plus No Idol.
- Gameplay proof that each stage has four floors, exactly three idol-gate interactions, idol/No Idol pickup unlocks gates, guardian/boss paths no longer spawn extra idol altars, and the total per difficulty is 12.
- Staged standalone refresh and shortcut check because this affects playable runtime.
