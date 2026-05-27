# Pending Issues - Core

## Retire Unused Hero Move Speed Multiplier Formula

- Severity tag: [Minor]
- What's wrong: `UT66RunStateSubsystem::GetHeroMoveSpeedMultiplier()` remains declared and defined as a legacy formula, but live hero walking speed now reads the raw `Speed` stat through `UT66HeroMovementComponent` instead of consuming this multiplier.
- Why it's out of scope now: The current pass changes movement authority to the user-approved Speed-stat model and updates docs; broad stat API deletion could affect UI/reference code and should be handled as a separate cleanup.
- What fixing it would entail: Remove or repurpose `GetHeroMoveSpeedMultiplier()`, audit any Blueprint or external references, and update stats documentation once the API is no longer needed for compatibility.

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

## QuadRetro Mob Rows Reference Missing Pixel Textures

- Severity tag: [Minor]
- What's wrong: The map-transition staged gameplay smoke logged `LogT66CharacterVisuals` warnings from `Source/T66/Core/T66CharacterVisualSubsystem.cpp` for QuadRetro static mob visuals such as `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`, `TombSpider`, `StoneSentinel`, `MimicLure`, `BoneConjurer`, and `CryptWraith` because their expected `/Game/Characters/Mobs/.../Textures/T_<Mob>` pixelated textures are missing in the packaged build.
- Why it's out of scope now: The map-transition pass only replaced tower wall/floor/ceiling visuals and did not alter mob visual rows, mob texture assets, or the QuadRetro fallback path.
- What fixing it would entail: Audit the mob visual data rows and packaged texture assets, either restore/import the referenced pixel textures or update the rows to the current production ToonStyle/VAT assets, then add a staged smoke check that `LogT66CharacterVisuals` no longer emits these missing-pixel-texture warnings.
