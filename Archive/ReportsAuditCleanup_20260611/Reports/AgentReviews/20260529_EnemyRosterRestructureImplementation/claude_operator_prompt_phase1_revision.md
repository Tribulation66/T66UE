# Claude Operator Prompt — Enemy Roster Restructure Phase 1 Revision

## Working Task

Revise Phase 1 implementation to close Codex validation gaps before import/build/runtime verification. This is still source/data/report work only unless a focused compile is needed.

Operator: Claude (`claude-opus-4-8`, FullOperator)
Validator/Finisher: Codex
Scope: Fix the specific gaps below, update the Phase 1 completion packet, and do not broaden into unrelated systems.
Stop condition: Apply the fixes if cleanly possible. If one cannot be cleanly applied without violating the user constraints, stop and write the blocker plainly in `phase1_completion.md`.

## Read First

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation\phase1_completion.md`
- `C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md`

## Codex Validation Findings To Fix

### 1. Debuff projectile cleanup is incomplete

The user explicitly requested removing the Debuff enemy and its status projectile, and verification asks for a clean Debuff grep for removed references. Phase 1 removed `AT66UniqueDebuffEnemy` but retained:

- `AT66UniqueDebuffProjectile`
- `FT66TemporaryProjectileSystem::ProfileUniqueDebuff`
- active projectile counters in lag/performance systems
- Backrooms cleanup filters
- a visual/debug spawn in `T66PlayerController_Overlays.cpp`

Revise this to remove the UniqueDebuff projectile path unless you discover it is a real production player ability that cannot be removed in this pass. The currently observed `T66PlayerController_Overlays.cpp` usage appears to be a visual/debug preview, not a production enemy. If it is only preview/debug/proof support, remove or replace that preview with an existing non-debuff projectile so the Debuff projectile class and profile can be deleted. Update performance/lag counters and Backrooms cleanup filters accordingly.

Target after revision: `rg "UniqueDebuff|DebuffEnemy|DebuffProjectile|ProfileUniqueDebuff" Source/T66 Content/Data` should have zero hits, except report artifacts under `Reports/`.

### 2. Gameplay-floor terminology rename is incomplete

Remaining live source/data references include:

- `Content/Data/PlayerExperience.json`: `GameplayFloorsPerStage`, `InitialEnemiesPerGameplayFloor`
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceTypes.h`
- `Source/T66/Core/PlayerExperience/T66PlayerExperienceSubSystem_Spawning.cpp`
- `Source/T66/Core/T66StageProgressionSubsystem.cpp`
- `Source/T66/Gameplay/T66EnemyDirector.h`: `InitialTowerEnemiesPerGameplayFloor`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`: `SpawnBudget.InitialEnemiesPerGameplayFloor`

The user requested "gameplay floors" -> "mob floors" throughout code/docs. Rename these to mob-floor terminology now. If a serialized/DataTable compatibility bridge is required, add one explicitly and document it; do not leave the old names as the active terminology. Phase 2 can rebuild uassets, but Phase 1 should update source JSON/CSV/code keys where practical.

Target after revision: no live code/data hits for `GameplayFloorsPerStage`, `InitialEnemiesPerGameplayFloor`, `InitialTowerEnemiesPerGameplayFloor`, `GameplayFloor`, `gameplay floor`, `Gameplay Floor`, `bGameplayFloor`, `FirstGameplayFloorNumber`, `LastGameplayFloorNumber`, except unrelated concepts that are clearly not tower floor terminology. If retained, document the exact compatibility reason and make the canonical name mob-floor.

### 3. Vendor Token rename is incomplete

The user locked `GamblersToken -> VendorToken`. Phase 1 still has many runtime-facing `GamblersToken` symbols, including:

- `ApplyGamblersTokenPickup`
- `GetActiveGamblersTokenLevel`
- `MaxGamblersTokenLevel`
- `ActiveGamblersTokenLevel`
- `GamblersTokenUnlockedLevel`
- `ET66SecondaryStatType::GamblerToken`
- localization cases and item helpers

Revise to make VendorToken the canonical runtime API/data name. Preserve save compatibility only through clearly deprecated legacy aliases or migration fields where necessary. It is acceptable to keep an explicit `LegacyGamblersToken` item ID alias solely for old saves, but new runtime code, UI, structured events, and display-facing identifiers should use VendorToken.

Target after revision: `rg "GamblersToken|GamblerToken" Source/T66 Content/Data` should only show explicitly marked legacy/deprecated compatibility hooks and casino gambling feature names that are not the token. Do not leave runtime API names as GamblersToken if they can be renamed safely.

### 4. Stale archetype pending docs remain

The data/gameplay/enemies pending docs still describe Exploder/Stutterer/Burrower as production roster issues after those tags were cleared. Update or close those pending issues so the docs do not contradict the roster cleanup.

### 5. Update completion packet

Rewrite/update `Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation\phase1_completion.md` so it reflects the revised state, not the previous deviations. Include:

- Section-by-section status.
- Exact remaining exceptions, if any.
- Grep commands and results for Debuff, gameplay-floor, VendorToken/GamblersToken, Goblin, dormant miniboss tuning.
- What remains deferred to Phase 2.

## Exclusions

- No Git commit/stage/push/reset/clean/checkout.
- No editor import, staged build, or runtime smoke in this revision unless needed only as a focused blocker check.
- No Mini/minigame work.
- No broad casino redesign.
- No unrelated cleanup outside these validation findings.
