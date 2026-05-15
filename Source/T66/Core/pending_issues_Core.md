# Pending Issues - Core

## Legacy Lab Unlock IDs In Existing Save Games

- Severity tag: [Minor]
- What's wrong: `UT66ProfileSaveGame::LabUnlockedEnemyIDs` stores raw enemy row names, and existing player saves may still contain legacy IDs such as `Dungeon_Slime` after the roster migration. Runtime source references were moved to the new production IDs, but no save migration remaps old lab unlock IDs to the 50-mob roster.
- Why it's out of scope now: This pass replaces the authored roster/data/assets and verifies the new gallery/spawn path; it does not change persistent player profile migration policy.
- What fixing it would entail: Add a profile migration table from the 25 legacy IDs to appropriate production IDs, run it during profile load, and verify old staged saves still expose expected lab unlocks.

## Skeletal Hero Rows Ignore MeshRelativeScale

- Severity tag: [Minor]
- What's wrong: `UT66CharacterVisualSubsystem::ApplyCharacterVisual` applies `MeshRelativeScale` for static hero visuals, but skeletal `Hero_` visuals are currently forced to `FVector::OneVector`. The accepted Royal Chad/Arthur skeletal asset therefore bakes the former live row scale `1.011123` into `SK_Hero_1_Chad_QuadRetroAnimQA` to preserve the selected character's runtime size.
- Why it's out of scope now: The Arthur pass needed to avoid broad changes to every skeletal hero, companion, and preview alignment path while fixing the live playable visual.
- What fixing it would entail: Add a data-driven skeletal hero scale policy in the character visual subsystem, regression-test preview and gameplay alignment, and remove per-asset baked-scale workarounds only after every affected hero row is revalidated.
