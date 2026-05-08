# T66Idle Memory Progression

## Current Skeleton

The Idle module now has a compile-focused runtime foundation:

- Module: `Source/T66Idle`
- Prefix: `T66Idle`
- Public data structs: `FT66IdleHeroDefinition`, `FT66IdleZoneDefinition`, `FT66IdleProfileSnapshot`
- Core subsystems: `UT66IdleDataSubsystem`, `UT66IdleFrontendStateSubsystem`
- Save layer: `UT66IdleProfileSaveGame`, `UT66IdleSaveSubsystem`
- UI entry point: `UT66IdleMainMenuScreen`

No project registration or shared routing has been added in this worker pass.

## Progression Model Notes

The planned Idle Chadpocalypse loop should start with these local concepts:

- Stage progression: player pushes through linear stages, with boss checks at authored intervals.
- Tap damage: immediate player input damage, upgraded by gold.
- Passive DPS: hero/helper damage used for offline progress and active auto-combat.
- Gold: primary short-loop currency earned from enemies and offline claims.
- Prestige: deferred reset layer that converts long-run progress into permanent multipliers.
- Zones: grouped stage bands with theme, health, reward, and visual identity.

## Save Fields To Preserve

The first save snapshot intentionally keeps the smallest useful offline-progress state:

- `HighestStageReached`
- `Gold`
- `LifetimeGold`
- `TapDamage`
- `PassiveDamagePerSecond`
- `LastSavedUtc`

Future additions should be versioned before changing existing field meaning.

## Deferred Systems

- Hero upgrade tree
- Prestige currency and reset rules
- Offline reward preview/claim UI
- Authored economy curves
- Cloud save or backend reconciliation
- Analytics events
