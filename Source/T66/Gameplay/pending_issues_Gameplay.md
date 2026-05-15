# Pending Issues - Gameplay

## Spawn Director Still Uses Fallback-Family Behavior

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66EnemyDirector.cpp` now reads up to 10 stage slots, but production archetype selection is still routed through `FamilyID` fallback classes. The director does not yet account for the full 10-archetype combat mix as distinct mechanics.
- Why it's out of scope now: The current pass must keep the stage spawn path working while the new archetype classes are not implemented.
- What fixing it would entail: Refactor the director to choose from weighted archetype quotas, consume `Archetype` directly, and add deterministic handling for empty stage slots and unsupported archetypes.

## Hell Core Has No Ranged Mob

- Severity tag: [Minor]
- What's wrong: Hell stages intentionally use core mobs `PitImp`, `BoneKnight`, `FireSkull`, `Hellhound`, and `Gargoyle`, which contain no `Ranged` fallback family. Any old logic expecting a guaranteed ranged slot could behave differently in Hell.
- Why it's out of scope now: The roster explicitly marks this as intentional and says not to force a ranged substitution.
- What fixing it would entail: Verify Hell-stage spawn variety after the archetype-aware director refactor and add explicit no-ranged handling if design needs it.

## No Automated Main-Board Enemy Wave Smoke Hook

- Severity tag: [Minor]
- What's wrong: Staged standalone automation can launch `GameplayLevel`, capture the HUD, and confirm stage/bootstrap/gallery logs, but there is no command-line hook that moves the player from the start area into the main board and waits for enemy director wave spawns.
- Why it's out of scope now: This pass needed a roster/data migration and used the existing automation surface without adding new gameplay test harness code.
- What fixing it would entail: Add a development-only command-line automation path that teleports or drives the hero into the active floor, waits for director spawn logs, captures a screenshot, and exits with a nonzero status if no enemy spawns occur.

## Player Experience Tuning Can Be Requested Before DataTable Is Available

- Severity tag: [Minor]
- What's wrong: The Easy mob VAT staged gameplay smoke logged `PlayerExperience tuning requested by GetDifficultyStartStage before DataTable '/Game/Data/DT_PlayerExperience.DT_PlayerExperience' was available; returning empty tuning.`
- Why it's out of scope now: The VAT pass only changed enemy visual animation data and did not change the player-experience subsystem initialization order.
- What fixing it would entail: Audit the player-experience subsystem load sequence, make the tuning table available before gameplay difficulty startup queries, and add a staged smoke assertion that this warning no longer appears.

## Generated Wall Stack Has A Mid-Height Visual Join

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66TowerMapTerrain.cpp` now fills the 1200 UU wall span by stacking two generated wall mesh instances at 600 UU each. The wall source textures/modules are not authored as vertical top/bottom pairs, so a horizontal visual join can remain at Z=600 even though the wall-to-ceiling gap is closed.
- Why it's out of scope now: This terrain pass was limited to runtime assembly logic and explicitly did not touch mesh or texture assets.
- What fixing it would entail: Author vertically tileable wall modules or add paired bottom/top wall variants per theme, then teach the generated-kit assembly to select matched pairs.

## Inter-Walkable-Box Floor Seams Remain Possible

- Severity tag: [Major]
- What's wrong: `Source/T66/Gameplay/T66TowerMapTerrain.cpp` now emits one generated floor visual mesh per walkable source box, which removes internal subdivision seams, but seams can still appear where separate walkable boxes or drop-hole carve-out rectangles meet.
- Why it's out of scope now: Eliminating those joins requires changing floor-box generation/merging semantics beyond the requested visual assembly fix.
- What fixing it would entail: Merge each gameplay floor's compatible walkable rectangles into a single visual surface, or author a dedicated runtime mesh for the floor footprint so there are no visual boundaries between adjacent boxes.

## Doorway Header Mesh Selection Lacks Source Run Metadata

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66TowerMapTerrain.cpp` records doorway headers as `FBox2D` values without the wall-run seed or side metadata from `T66EmitDungeonTileEdgeWall`. The generated-kit header path can spawn a pinned wall module for each header, but it cannot always prove it is the exact same module as both adjacent wall fragments.
- Why it's out of scope now: The current pass only enables generated-kit header spawning and keeps the existing floor/header data model intact.
- What fixing it would entail: Store doorway header metadata alongside each header box, including source direction and wall-run seed, then consume that metadata during generated header visual selection.
