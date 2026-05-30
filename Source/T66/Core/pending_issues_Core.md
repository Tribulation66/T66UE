# Pending Issues - Core

## Resolved 2026-05-29 - RetroFX Default-On Recurrence And Low-Resolution Pixelation

- Former severity tag: [Major]
- What was wrong: `FT66RetroFXSettings` and the saved duplicate master flag recreated gameplay RetroFX, real low resolution, and frontend CRT as enabled through settings defaults, migration, reset, safe-mode, UI reset, save load, and world-startup application. The visible pixelation was primarily `r.ScreenPercentage` being reduced by real-low-resolution mode.
- Resolution: RetroFX/CRT now defaults off from the settings struct, the duplicate saved master flag was removed, schema 24 migration forces existing saves off, all named recreation paths were sealed by verification, and the off path restores `r.ScreenPercentage=100`.
- Evidence: `Reports/AgentReviews/20260529_RetroFXOffByDefaultFix/completion_packet.md`

## Resolved 2026-05-28 - Staged Standalone Build Blocked By Undeclared Accuracy Item ID

- Former severity tag: [Blocker]
- What was wrong: `Scripts\StageStandaloneBuild.ps1` failed during the Win64 `T66` target build on 2026-05-28 because `Source/T66/Core/T66GameInstance.cpp` referenced `AccuracyItemID` around lines 774, 775, 777, and 779, but that identifier was undeclared in the compile unit.
- Resolution: The item taxonomy pass retired the old secondary `Accuracy` item, replaced the random-pool fallback with `Execute`, and rebuilt `T66Editor` successfully. The remaining proof gate is the staged standalone refresh for this pass.

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
