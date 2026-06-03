# Phase 1B - Atmosphere Extension Report

Date: 2026-05-17

## Scope

Workstream D added the cel-atmosphere data model and runtime parameter delivery path for ToonStyle materials. Runtime discovery is explicit component registration from the TestRoom, not asset path scanning.

## Data Model

Added `FT66ThemeCelAtmosphere` in `Source/T66/Gameplay/T66ThemeAtmosphereData.h`.

Fields:

- `LightDirection`
- `LightColor`
- `RampStep1`
- `RampStep2`
- `ShadeColor`
- `MidtoneColor`
- `LitColor`
- `RimColor`
- `RimPower`
- `RimStrength`
- `OutlineColor`
- `OutlineWidth`
- `EnvShadeColor`
- `EnvMidtoneColor`
- `EnvLitColor`

Extended `FT66ThemeAtmosphereSpec` with:

- `FT66ThemeCelAtmosphere CelAtmosphere`

## Theme Values

Dungeon is the reference TestRoom theme and uses the prompt-provided baseline values:

- LightDirection: normalized `(-0.4, 0.6, -0.7)`
- RampStep1: `0.0`
- RampStep2: `0.5`
- ShadeColor: `(0.35, 0.38, 0.50)`
- MidtoneColor: `(0.7, 0.7, 0.72)`
- LitColor: white
- RimColor: `(1.0, 0.95, 0.85)`
- RimPower: `4.0`
- RimStrength: `0.3`
- OutlineColor: black
- OutlineWidth: `1.5`
- Environment shade/midtone/lit: `(0.3, 0.32, 0.42)`, `(0.55, 0.58, 0.62)`, `(0.85, 0.85, 0.90)`

Hell, Ocean, Martian, and Forest have placeholder cel atmosphere values in `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp`. These are mood placeholders only and need refinement in later theme-tuning phases.

## Parameter Delivery

Added `ET66ToonMaterialKind` in `Source/T66/Gameplay/T66WorldVisualSetup.h`.

Kinds:

- Character
- Outline
- Environment

Added explicit registration APIs:

- `FT66WorldVisualSetup::RegisterToonMaterial(UMeshComponent* Component, ET66ToonMaterialKind Kind, int32 MaterialIndex)`
- `FT66WorldVisualSetup::UnregisterToonMaterial(UMeshComponent* Component)`
- `FT66WorldVisualSetup::ApplyToonCelAtmosphereToRegisteredMaterials(ET66TowerGameplayLevelTheme Theme)`

Registration creates a MID with `CreateAndSetMaterialInstanceDynamic` and records the component, material index, kind, and MID. Theme application pushes parameters to registered MIDs only.

No runtime `/Game/ToonStyle/` path scan is used.

## Gate G6

G6 passed in the staged TestRoom smoke.

Evidence from `Saved/StandaloneLogs/Phase1B_TestRoomSmoke_Final_PostOutlineFix.log`:

- `Applied cel atmosphere theme=0 to 26 registered toon material(s).`
- `Applied cel atmosphere theme=4 to 26 registered toon material(s).`
- `ToonStyle TestRoom G6 parameter probe applied Dungeon=26 Hell=26 RestoredDungeon=26.`

The TestRoom probes Dungeon, applies Hell, and restores Dungeon. This verifies that registered MIDs update on a forced theme change.

## Registered Component Count

The staged TestRoom registers 26 toon material components:

- 10 shading lineup actors
- 10 outline lineup actors
- 6 room surface actors

Labels and non-toon helper actors are not registered.
