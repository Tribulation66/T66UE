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

## Non-Dungeon Theme Atmosphere Specs Need Authoring

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp` now contains the first Dungeon atmosphere spec, but Forest, Ocean, Martian, and Hell return neutral lighting/fog/grading values.
- Why it's out of scope now: Atmosphere Iteration 01 is explicitly Dungeon-only so Pablo can validate the foundation before extending vibe-setter values to other themes.
- What fixing it would entail: Author and tune per-theme sky light, fog, and color-grading specs for Forest, Ocean, Martian, and Hell, then validate them in staged gameplay screenshots.

## NPC Class Names Still Use HouseNPC

- Severity tag: [Minor]
- What's wrong: The data/setup/runtime table seam now uses `NPCs.csv`, `DT_NPCs`, `NPCsDataTable`, and `GetNPCData`, but the underlying C++ actor/data symbols still include `AT66HouseNPCBase`, `FHouseNPCData`, and related subclass/include names in `Source/T66/Gameplay` and registry consumers.
- Why it's out of scope now: The current cleanup is constrained to data/loader/source naming and avoids broad C++ class renames that would touch generated headers, includes, Blueprint references, and asset class bindings.
- What fixing it would entail: Rename the C++ base actor and data struct to neutral NPC names, add Unreal redirects if needed, update all includes/subclasses/registry references, compile, and verify existing Blueprint/class references still resolve.

## Vehicle Class Names Still Use Tractor

- Severity tag: [Minor]
- What's wrong: The authored data row, prompts, damage source token, and interaction row lookup now use `Vehicle`, but the inherited C++ implementation still uses `AT66PilotableTractor`, `T66PilotableTractor.*`, and enum/action names such as `PilotTractor`, subobject names such as `TractorRoot`, and local variable names such as `ClosestTractor` in player interaction code.
- Why it's out of scope now: The current pass is scoped to data/schema/loader cleanup and avoids broad class/file renames that would touch generated headers, includes, actor references, and Blueprint bindings.
- What fixing it would entail: Rename the C++ class/files to a neutral vehicle name, update includes and player-controller interaction variables, add Unreal redirects if required, compile, and verify existing spawned/interactable vehicle references still resolve.

## TutorialGate Class Remains After Tutorial Exit Uses StageGate

- Severity tag: [Minor]
- What's wrong: `Source/T66/Gameplay/T66TutorialGate.*` and the `AT66TutorialGate` interaction path in `Source/T66/Gameplay/T66PlayerController_Combat.cpp` still exist, but `Source/T66/Gameplay/T66TutorialManager.cpp` now spawns `AT66StageGate` for the tutorial exit.
- Why it's out of scope now: This pass was intentionally constrained to swapping the tutorial end to the regular Stage Gate without broad class/file deletion or reference cleanup.
- What fixing it would entail: Prove there are no Blueprint, asset, map, or automation references to `AT66TutorialGate`, then remove the class, includes, player-controller branch, and any stale generated/API references with a focused compile and content reference check.