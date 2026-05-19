# T66 Atmosphere Iteration 01 Report

## 1. Summary

Atmosphere Iteration 01 landed the Dungeon light/material foundation and first vibe-setter layer: production environment material instances now point at a Default Lit environment master, interactables and NPC stands were consolidated into the GLB unlit actor band with a `DiffuseColorMap` alias, gameplay atmosphere setup now restores a spared white SkyLight plus Dungeon fog and theme grading, heroes carry a local point light, Dungeon tower floors spawn procedural torch point lights with optional cube markers and a light-function flicker material, and stage-progression color-saturation writes are gated off while preserving their call path.

## 2. Stage 1 - Light + Material Foundation

### 2.1 `M_Environment_Lit`

- Created asset: `/Game/Materials/M_Environment_Lit.M_Environment_Lit`.
- Shading model: Default Lit, verified by `Saved/Atmosphere_Iteration_01_Verification.json`.
- Parameter surface:
  - Texture: `DiffuseColorMap`.
  - Vector: `Tint`.
  - Scalars: `Brightness`, `Roughness`, `Specular`, `Metallic`.
- Graph intent: `DiffuseColorMap * Tint * Brightness` into Base Color, `Roughness`/`Specular`/`Metallic` into their Default Lit material inputs, no emissive contribution.
- Authoring path: task-specific Unreal Python commandlets under `Scripts/` created and verified the asset, then were removed after the assets were saved per the script lifecycle rule.
- Runtime load paths now prefer the lit master in `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:13`, `Source/T66/Gameplay/T66TerrainThemeAssets.cpp:62`, `Source/T66/Gameplay/T66MainMapTerrain.cpp:878`, `Source/T66/Gameplay/T66LavaShared.h:8`, `Source/T66/Gameplay/T66MiasmaBoundary.cpp:61`, and `Source/T66/Core/T66GameInstance.cpp:1471`.

### 2.2 Environment MI Repoints

All entries below were repointed from `/Game/Materials/M_Environment_Unlit.M_Environment_Unlit` to `/Game/Materials/M_Environment_Lit.M_Environment_Lit`; verification reported 47 touched instances, 0 mismatches, and 0 missing entries in `Saved/Atmosphere_Iteration_01_Verification.json`.

- `/Game/World/Cliffs/MI_HillTile1`
- `/Game/World/Cliffs/MI_HillTile2`
- `/Game/World/Cliffs/MI_HillTile3`
- `/Game/World/Cliffs/MI_HillTile4`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_Bones_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_Cracked_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_Drain_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_StoneSlabs_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_BonesNiche_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_Chains_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_StoneBlocks_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_TorchSconce_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestFloor_BrambleEdge_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestFloor_LeafCrack_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestFloor_MossStone_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestFloor_RootMat_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestWall_MushroomBark_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestWall_RootBraid_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestWall_TrunkWeave_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/ForestWall_VineTotem_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellFloor_BoneAsh_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellFloor_EmberFissure_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellFloor_Obsidian_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellFloor_RunePlate_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellWall_Brimstone_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellWall_ChainsSkulls_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellWall_LavaCrack_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/HellWall_SpikeBasalt_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianFloor_CraterCracks_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianFloor_CrystalDust_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianFloor_RegolithPlates_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianFloor_RuinTile_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianWall_CrystalVein_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianWall_MeteorScar_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianWall_RedRock_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/MartianWall_RuinPanel_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanFloor_CoralCrack_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanFloor_ReefStone_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanFloor_ShellSand_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanFloor_TidePool_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanWall_CoralReef_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanWall_KelpCoral_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanWall_ReefRuin_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/OceanWall_ShellLimestone_A_UnrealReady_Mat_00`
- `/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof`
- `/Game/World/Terrain/TowerForest/MI_TowerForestGround`
- `/Game/World/Terrain/TowerForest/MI_TowerForestRoof`

### 2.3 `M_GLB_Unlit` Alias And Actor-Band Repoints

- `/Game/Materials/M_GLB_Unlit.M_GLB_Unlit` now exposes both `BaseColorTexture` and `DiffuseColorMap`, verified by `Saved/Atmosphere_Iteration_01_Verification.json`.
- Alias graph choice: the master multiplies `BaseColorTexture * DiffuseColorMap * Tint * Brightness` into the unlit/emissive path, so existing GLB authoring and environment-style `DiffuseColorMap` overrides both remain effective.
- Runtime color utilities continue writing both names: `Source/T66/Gameplay/T66VisualUtil.cpp:28`, `Source/T66/Gameplay/T66VisualUtil.cpp:61-62`, `Source/T66/Gameplay/T66VisualUtil.cpp:98-99`, and `Source/T66/Gameplay/T66VisualUtil.cpp:112-113`.
- Dynamic theme helpers also write both texture names where they create runtime MIDs: `Source/T66/Gameplay/T66TowerThemeVisuals.cpp:121-122`, `Source/T66/Gameplay/T66TerrainThemeAssets.cpp:68-69`, `Source/T66/Gameplay/T66MainMapTerrain.cpp:1695-1696`, and `Source/T66/Gameplay/T66MiasmaBoundary.cpp:75-76`.

All entries below were repointed from `/Game/Materials/M_Environment_Unlit.M_Environment_Unlit` to `/Game/Materials/M_GLB_Unlit.M_GLB_Unlit`; verification reported 32 touched instances and 0 mismatches in `Saved/Atmosphere_Iteration_01_Verification.json`.

- `/Game/Characters/NPCs/Gambler/GamblerDemonStand/Materials/M_GamblerDemonStand`
- `/Game/Characters/NPCs/Gambler/QuadRetro/Materials/M_SM_Gambler_QuadRetro`
- `/Game/Characters/NPCs/Ouroboros/QuadRetro/Materials/M_SM_Ouroboros_QuadRetro`
- `/Game/Characters/NPCs/Saint/QuadRetro/Materials/M_SM_Saint_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_BladeSweep/Materials/M_Arcade_BladeSweep_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_BombSorter/Materials/M_Arcade_BombSorter_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_CartSwitcher/Materials/M_Arcade_CartSwitcher_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_CrystalDash/Materials/M_Arcade_CrystalDash_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_GoldMiner/Materials/M_Arcade_GoldMiner_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_LanternLeap/Materials/M_Arcade_LanternLeap_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_Machine/Materials/M_Arcade_Machine_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_MimicMemory/Materials/M_Arcade_MimicMemory_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_PotionPour/Materials/M_Arcade_PotionPour_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_RelicStack/Materials/M_Arcade_RelicStack_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_RuneSwipe/Materials/M_Arcade_RuneSwipe_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_ShieldParry/Materials/M_Arcade_ShieldParry_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_Topwar/Materials/M_Arcade_Topwar_QuadRetro`
- `/Game/World/Interactables/Arcade/Arcade_WhackAMole/Materials/M_Arcade_WhackAMole_QuadRetro`
- `/Game/World/Interactables/Arcade/Vehicle/Materials/M_Vehicle_QuadRetro`
- `/Game/World/Interactables/ArcadeMachine/Materials/M_ArcadeMachine`
- `/Game/World/Interactables/Chests/ChestModel/Materials/M_Chest`
- `/Game/World/Interactables/Chests/ChestModel/Materials/M_Chest_QuadRetro`
- `/Game/World/Interactables/Crate/Materials/M_Crate_QuadRetro`
- `/Game/World/Interactables/DifficultyTotem/Materials/M_DifficultyTotem_QuadRetro`
- `/Game/World/Interactables/Fountain/Materials/M_Fountain_QuadRetro`
- `/Game/World/Interactables/IdolAltar/Materials/M_IdolAltar_QuadRetro`
- `/Game/World/Interactables/Shroom/Materials/M_Shroom_QuadRetro`
- `/Game/World/Interactables/Vending/Materials/M_QuickReviveVending_QuadRetro`
- `/Game/World/LootBags/Black/Materials/M_SM_LootBag_Black_QuadRetro`
- `/Game/World/LootBags/Red/Materials/M_SM_LootBag_Red_QuadRetro`
- `/Game/World/LootBags/White/Materials/M_SM_LootBag_White_QuadRetro`
- `/Game/World/LootBags/Yellow/Materials/M_SM_LootBag_Yellow_QuadRetro`

The recon inventory contained four stale Arcade Amplifier material paths that were missing from the current asset registry and were not touched:

- `/Game/World/Interactables/ArcadeAmplifierPickup/Materials/M_ArcadeAmplifierPickup`
- `/Game/World/Interactables/ArcadeAmplifierPickup/Materials/M_ArcadeAmplifierPickup_Charged`
- `/Game/World/Interactables/ArcadeAmplifierPickup/Materials/M_ArcadeAmplifierPickup_Charged_QuadRetro`
- `/Game/World/Interactables/ArcadeAmplifierPickup/Materials/M_ArcadeAmplifierPickup_QuadRetro`

### 2.4 SkyLight And Strip Allowlist

- Spared tag: `T66_AtmosphereSpared`, declared in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:22`.
- Strip allowlist helper: `T66DestroyActorsOfTypeExceptTagged` in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:54`.
- The neutral strip still removes legacy sky atmosphere and directional light unconditionally, but SkyLight and Exponential Height Fog now skip actors carrying `T66_AtmosphereSpared`: `Source/T66/Gameplay/T66WorldVisualSetup.cpp:419-429`.
- SkyLight find/create entry point: `FT66WorldVisualSetup::EnsureAtmosphereSkyLightForWorld()` in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:463-471`.
- Full atmosphere entry point: `FT66WorldVisualSetup::EnsureAtmosphereForWorld()` in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:474-484`.
- SkyLight final settings are applied in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:269-287`:
  - Movable `ASkyLight`.
  - `SourceType = SLS_SpecifiedCubemap`.
  - No cubemap.
  - `Intensity = Spec.SkyLightIntensity`; Dungeon spec is `0.5`.
  - `LightColor = Spec.SkyLightColor`; Dungeon spec is white.
  - `LowerHemisphereColor = Spec.SkyLightColor`; Dungeon spec is white.
  - Shadows disabled.
  - Real-time capture disabled.
- Invocation is sourced from the gameplay game mode after theme resolution rather than from every neutral setup call, so FrontendLevel does not receive Dungeon fog/lighting. The call resolves theme from selected difficulty and invokes atmosphere in `Source/T66/Gameplay/T66GameMode.cpp:1365-1386`; settings changes re-apply neutral setup and atmosphere through `Source/T66/Gameplay/T66GameMode.cpp:1480-1483`.

### 2.5 Carry-Light

- Header property: `Source/T66/Gameplay/T66HeroBase.h:115`.
- Constructor setup: `Source/T66/Gameplay/T66HeroBase.cpp:111-120`.
- Final values:
  - Attached to `RootComponent`.
  - Relative location `(0, 0, 60)`.
  - Movable.
  - Intensity `2000.0`.
  - Attenuation radius `650.0`.
  - Light color `(1.0, 0.85, 0.65)`.
  - Shadows disabled.
  - Inverse squared falloff disabled.
  - Falloff exponent `2.0`.

## 3. Stage 2 - Torches

### 3.1 Module Structure

- Added `Source/T66/Gameplay/T66TowerLighting.h:32-41`.
- Added `Source/T66/Gameplay/T66TowerLighting.cpp`.
- `AT66TowerLightingActor` owns the runtime point-light components, a scene root, and per-floor tags.
- Spawn hook: `Source/T66/Gameplay/T66TowerMapTerrain.cpp:5795` calls `T66TowerLighting::SpawnFloorTorchLights(World, Floor, Layout, StageTheme, nullptr)` after wall and prop spawning.
- Teardown helper: `T66TowerLighting::DestroyFloorTorchLights()` in `Source/T66/Gameplay/T66TowerLighting.cpp:364-387`; spawn calls it before rebuilding a floor actor group in `Source/T66/Gameplay/T66TowerLighting.cpp:292`.

### 3.2 Placement Algorithm

- Dungeon-only and gameplay-floor-only gate in `Source/T66/Gameplay/T66TowerLighting.cpp:276-294`.
- Source wall boxes use `Floor.TrapEligibleWallBoxes` with fallback to `Floor.MazeWallBoxes`, implemented in `Source/T66/Gameplay/T66TowerLighting.cpp:113-125`.
- Doorway exclusions use `Floor.DoorwayHeaderBoxes`, implemented in `Source/T66/Gameplay/T66TowerLighting.cpp:127-135`.
- Long-axis wall sampling, `2800.0` UU spacing, `80.0` UU corridor offset, `50.0` UU vertical offset, `1500.0` UU minimum torch separation, and cap of 24 torches per floor are implemented in `Source/T66/Gameplay/T66TowerLighting.cpp:113-168`.
- The nearest non-unused grid cell is used to estimate the corridor normal because the current wall registry does not carry a canonical wall-side/doorway semantic.
- Dry-run count: not observed in gameplay because this pass intentionally did not launch a playable dungeon smoke. Runtime spawn logs emit `[ATMOSPHERE] Spawned ... torch lights ...` when dungeon floors are generated.

### 3.3 Light Configuration

Torch point-light values are configured in `Source/T66/Gameplay/T66TowerLighting.cpp:333-349`:

- Movable point light.
- Relative location from the owning floor lighting actor.
- Intensity `4000.0`.
- Light color `(1.0, 0.628, 0.251)`.
- Attenuation radius `900.0`.
- Shadows disabled.
- Inverse squared falloff disabled.
- Falloff exponent `2.0`.
- Light function: `/Game/Materials/M_TorchFlicker_LightFn.M_TorchFlicker_LightFn`, loaded in `Source/T66/Gameplay/T66TowerLighting.cpp:204-212` and assigned in `Source/T66/Gameplay/T66TowerLighting.cpp:345-347`.

### 3.4 Flicker Status

- Created asset: `/Game/Materials/M_TorchFlicker_LightFn.M_TorchFlicker_LightFn`.
- Verified domain: `MD_LIGHT_FUNCTION` in `Saved/Atmosphere_Iteration_01_Verification.json`.
- Final staged build log shows normal shadermap cache misses and compilation display lines for the material, not a material graph warning: `Saved/StandaloneLogs/Atmosphere_Iteration_01_Build.log:292-293`.
- Final build summary: `0 error(s), 6 warning(s)` in `Saved/StandaloneLogs/Atmosphere_Iteration_01_Build.log:610`.

### 3.5 Placeholder Mesh Status

- Placeholder cube support landed behind cvar `t66.TorchPlaceholderMesh`, default `1`, in `Source/T66/Gameplay/T66TowerLighting.cpp:28-31`.
- Mesh: `/Engine/BasicShapes/Cube.Cube`, loaded in `Source/T66/Gameplay/T66TowerLighting.cpp:170-178`.
- Marker creation and material setup live in `Source/T66/Gameplay/T66TowerLighting.cpp:214-260`.
- Marker scale: `(0.2, 0.2, 0.4)`.
- Marker material: runtime DMI from `/Game/Materials/M_GLB_Unlit.M_GLB_Unlit`, white texture plus warm tint and brightness.

## 4. Stage 3 - Vibe Setters

### 4.1 Theme Data

- Added `FT66ThemeAtmosphereSpec` in `Source/T66/Gameplay/T66ThemeAtmosphereData.h:10-56`.
- Dungeon spec uses the struct defaults:
  - `SkyLightColor = White`
  - `SkyLightIntensity = 0.5`
  - `FogDensity = 0.04`
  - `FogHeightFalloff = 0.2`
  - `FogInscatteringColor = (0.12, 0.18, 0.32, 1.0)`
  - `FogStartDistance = 400`
  - `FogCutoffDistance = 20000`
  - `ColorGradeShadowsTint = (0.7, 0.85, 1.1, 1.0)`
  - `ColorGradeMidtonesTint = (0.95, 0.97, 1.02, 1.0)`
  - `ColorGradeHighlightsTint = (1.0, 1.0, 1.0, 1.0)`
  - `ColorGradeSaturation = (0.85, 0.85, 0.85, 1.0)`
  - `ColorGradeContrast = (1.1, 1.1, 1.1, 1.0)`
  - `ColorGradeGain = (0.9, 0.9, 0.9, 1.0)`
- Forest, Ocean, Martian, and Hell return a neutral spec from `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:7-24`; theme switch is in `Source/T66/Gameplay/T66ThemeAtmosphereData.cpp:27-42`.

### 4.2 Atmosphere Setup

- Full setup entry point: `FT66WorldVisualSetup::EnsureAtmosphereForWorld()` in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:474-484`.
- SkyLight setup: `Source/T66/Gameplay/T66WorldVisualSetup.cpp:269-287`.
- Fog density resolves through player settings in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:290-312`.
- Fog actor creation and settings are applied in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:340-357`:
  - Tagged `T66_AtmosphereSpared`.
  - Movable.
  - Density = spec density multiplied by player fog intensity percent, or `0` when fog is disabled.
  - Height falloff, inscattering color, start distance, cutoff distance from spec.
  - Volumetric fog disabled.
- Theme post-process creation and grading are applied in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:390-417`:
  - Tagged `T66_AtmosphereSpared` and `T66_ThemeAtmospherePostProcess`.
  - Unbound.
  - Priority `1000.0`.
  - Blend weight `1.0`.
  - Applies shadows/midtones/highlights through Color Gain channel overrides, plus global saturation, contrast, and gain.
- Neutral post-process remains lower-priority and separately tagged `T66_NeutralPostProcess` so the neutral finder does not reuse the theme PP volume: tag declarations in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:22-24`, neutral finder logic in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:213-249`.
- Retro FX remains above both via its existing priority 5000 path.

### 4.3 Player Settings Fog Wiring

- Instead of introducing a second delegate owner, the existing `UT66PlayerSettingsSubsystem::OnSettingsChanged` game-mode wiring is reused.
- Settings changes call `FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld())` and then `ApplyStageProgressionVisuals()`, which now also re-applies theme atmosphere with the current resolved theme: `Source/T66/Gameplay/T66GameMode.cpp:1480-1483`.
- Fog density reads current `GetFogEnabled()` / `GetFogIntensityPercent()` at apply time in `Source/T66/Gameplay/T66WorldVisualSetup.cpp:290-312`.

## 5. Stage 4 - Stage Progression Cleanup

- Stage-progression visual writes are gated by `T66_StageProgressionVisualWritesEnabled()`, currently returning `false`, in `Source/T66/Gameplay/T66StageProgressionVisuals.cpp:11-24`.
- The original `ColorSaturation` assignment remains in the preserved code path but is unreachable while the gate is false: `Source/T66/Gameplay/T66StageProgressionVisuals.cpp:34-35`.
- Bootstrap/main-map call sites are preserved: `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:328`, `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:337`, and `Source/T66/Gameplay/GameMode/T66GameMode_Bootstrap.cpp:348`.
- Gameplay game mode still calls `ApplyStageProgressionVisuals()` during setup and settings changes: `Source/T66/Gameplay/T66GameMode.cpp:1362`, `Source/T66/Gameplay/T66GameMode.cpp:1365-1386`, and `Source/T66/Gameplay/T66GameMode.cpp:1480-1483`.
- `Config/DefaultT66StageProgression.ini` values were left unchanged.

## 6. Pending Issues Added

Added `Source/T66/Gameplay/pending_issues_Gameplay.md:59-64`:

- `Non-Dungeon Theme Atmosphere Specs Need Authoring` `[Minor]`
- Forest, Ocean, Martian, and Hell currently return neutral lighting/fog/grading values until later theme passes author their vibe-setter specs.

## 7. Build Log Excerpt

Last 50 lines of `Saved/StandaloneLogs/Atmosphere_Iteration_01_Build.log`:

```text
LogIoStore: Display: Input:      2.30 GiB UExp
LogIoStore: Display: Input:      7.10 MiB UAsset
LogIoStore: Display: Input:      1.23 GiB UBulk
LogIoStore: Display: Input:    109.24 MiB for 1002 Global shaders
LogIoStore: Display: Input:      3.83 MiB for 430 Shared shaders
LogIoStore: Display: Input:         0   B for 0 Unique shaders
LogIoStore: Display: Input:     16.17 MiB for 463 Inline shaders
LogIoStore: Display:
LogIoStore: Display: Output:    78006 Name map entries
LogIoStore: Display: Output:     3691 Imported package entries
LogIoStore: Display: Output:     1766 Packages without imports
LogIoStore: Display: Output:        0 Public runtime script objects
LogIoStore: Display: Output:     4.94 MiB HeaderData
LogIoStore: Display: Output:     2.71 MiB InitialLoadData
LogIoStore: Display:
LogIoStore: Display: Success
LogPakFile: Display: UnrealPak executed in 6.916176 seconds
Took 7.53s to run UnrealPak.exe, ExitCode=0
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_UFSFiles.txt, NumItems: 8756
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFiles.txt, NumItems: 1562
DumpManifest: C:\Users\DoPra\AppData\Roaming\Unreal Engine\AutomationTool\Logs\C+Program+Files+Epic+Games+UE_5.7\FinalCopyWin64_NonUFSFilesDebug.txt, NumItems: 6
Copying NonUFSFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Copying DebugFiles to staging directory: C:\UE\T66\Saved\StagedBuilds\Windows
Stage command time: 15.34 s
********** STAGE COMMAND COMPLETED **********
********** PACKAGE COMMAND STARTED **********
Package command time: 0.00 s
********** PACKAGE COMMAND COMPLETED **********
BuildCookRun time: 156.41 s
BUILD SUCCESSFUL
AutomationTool executed for 0h 2m 37s
AutomationTool exiting with ExitCode=0 (Success)
Refreshed loose runtime root 'RuntimeDependencies/T66/Fonts/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\Fonts' (2 files).
Refreshed loose runtime root 'RuntimeDependencies/T66/Arcade/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\Arcade' (61 files).
Refreshed loose runtime root 'RuntimeDependencies/T66/UI/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI' (653 files).
Refreshed loose runtime root 'RuntimeDependencies/T66/Video/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\Video' (97 files).
Refreshed loose runtime root 'Content/Movies/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Movies' (99 files).
Refreshed loose runtime root 'Content/Mini/Data/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Mini\Data' (13 files).
Refreshed loose runtime root 'SourceAssets/Mini/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\Mini' (470 files).
Refreshed loose runtime root 'SourceAssets/ItemSprites/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\ItemSprites' (1 files).
Refreshed loose runtime root 'Content/TD/Data/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\TD\Data' (10 files).
Refreshed loose runtime root 'SourceAssets/TD/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\TD' (46 files).
Refreshed loose runtime root 'Content/Deck/Data/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Deck\Data' (11 files).
Refreshed loose runtime root 'SourceAssets/Deck/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\Deck' (26 files).
Refreshed loose runtime root 'Content/Idle/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Idle' (10 files).
Refreshed loose runtime root 'SourceAssets/Idle/...' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\SourceAssets\Idle' (18 files).
Standalone build ready at 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe'.
Reset standalone GameUserSettings: C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\Config\Windows\GameUserSettings.ini (1920 x 1080, windowed).
Updated standalone shortcut 'C:\UE\T66\T66 Standalone.lnk' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe'.
Updated standalone shortcut 'C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk' -> 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe'.
```

Additional verification:

- Final build warning summary: `Saved/StandaloneLogs/Atmosphere_Iteration_01_Build.log:610` reports `Success - 0 error(s), 6 warning(s)`.
- Remaining build warnings are pre-existing configuration/missing Pixal3D soft-reference warnings: `Saved/StandaloneLogs/Atmosphere_Iteration_01_Build.log:65-170`.
- Staged executable path: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Shortcut verification: `C:\UE\T66\T66 Standalone.lnk` and the taskbar `T66 Standalone.lnk` both target the staged executable.
- Launch-only smoke: `Saved/StandaloneLogs/Atmosphere_Iteration_01_LaunchSmoke.log:463` initialized the engine, `Saved/StandaloneLogs/Atmosphere_Iteration_01_LaunchSmoke.log:476-479` quit by request, and `Saved/StandaloneLogs/Atmosphere_Iteration_01_LaunchSmoke.log:501` exited cleanly.

## 8. Deferred Decisions

- No playable dungeon visual smoke was performed by Codex because Pablo explicitly owns the eye check for this pass.
- Torch counts per floor were not observed in a gameplay walk for the same reason; runtime logs now report counts when Dungeon floors spawn.
- `AttachOwner` in the requested tower-lighting API was not used by the hook because the existing spawn seam is a helper inside `T66TowerMapTerrain::Spawn()` without a stable terrain actor pointer at that call. The lighting actor is world-spawned, tagged by floor number, and cleaned by floor tag before respawn.
- Direct player-settings delegate binding inside `T66WorldVisualSetup` was not added because the existing `AT66GameMode` settings-change callback already has the correct world/theme context and now re-applies atmosphere idempotently.
- Requested repo path `Audit/Reference/Atmosphere_Recon/Handoff.md` did not exist; the provided desktop handoff `C:\Users\DoPra\Desktop\T66_Atmosphere_Pass_Handoff.md` was used instead during inspection.
- Four Arcade Amplifier material paths from the recon inventory were stale and absent from the current asset registry; they are listed in section 2.3.

## 9. Open Questions for Pablo

- Confirm the Dungeon fog density, grading, SkyLight intensity, torch intensity, and carry-light intensity from staged screenshots.
- Decide when to set `t66.TorchPlaceholderMesh=0` after real torch/sconce art lands.
