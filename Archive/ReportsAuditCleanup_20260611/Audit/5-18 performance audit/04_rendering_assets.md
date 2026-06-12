# Section 4 - Rendering and Assets

## Instancing State

HISM/ISM migration is partial.

HISM/ISM paths found:

- Tower floor visuals use `UHierarchicalInstancedStaticMeshComponent` in `T66TowerMapTerrain.cpp`.
- Main map terrain floor/slope batches use instancing in `T66MainMapTerrain.cpp`.
- Hero attack/close/long-range rings use instancing in `T66HeroBase.cpp`.
- Miasma tiles use instancing in `T66MiasmaManager.cpp`.
- Miasma boundary walls use instancing.
- Floor spike traps use instancing.

Still individual component heavy:

- Enemies use direct `UStaticMeshComponent` paths.
- Interactables use direct `UStaticMeshComponent` paths.
- Heroes use direct mesh components.
- Boss projectiles use direct mesh components.
- Torch markers use direct mesh components.

## Master Materials and Material Stats

Core materials referenced from code/scripts include:

- `/Game/Materials/M_Character_Unlit`
- `/Game/Materials/M_FBX_Unlit`
- `/Game/Materials/M_GLB_Unlit`
- `/Game/Materials/MI_GLB_Unlit_Character_Shared`
- `/Game/Materials/M_Environment_Unlit`
- `/Game/Materials/M_Environment_Lit`
- `/Game/Materials/M_GLB_ViewSpaceLit_Character`
- `/Game/Materials/Retro/M_Environment_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_Character_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry`
- `/Game/Materials/Retro/M_T66_OutlinePostProcess`
- `/Game/UI/Materials/M_T66_UI_CRTPostProcess`
- `/Game/VFX/M_PixelSprite`
- `/Game/ToonStyle/Materials/M_Toon_Character`
- `/Game/ToonStyle/Materials/M_Toon_Environment`

Readable binary metadata indicated most game materials above are opaque/unlit, except `M_Environment_Lit`, which is opaque/default-lit. `M_T66_UI_CRTPostProcess` appears as a translucent UI/default-lit material in metadata.

Latest material stats CSV:

- `Saved/MaterialStats/Stats-2026.05.18-17.56.02.csv`

Relevant rows from that CSV:

- `M_GLB_Unlit`: cooked counts 12/18 across platforms seen.
- `M_Character_Unlit`: cooked counts 30/36.
- `M_Environment_Lit`: cooked counts 6/13.
- `M_Environment_Unlit`: cooked counts 12/18.
- `M_FBX_Unlit`: cooked counts 12/14.
- `M_Toon_Character`: cooked counts 4/8.
- `M_Toon_Environment`: cooked counts 4/10.
- `M_T66_UI_CRTPostProcess`: cooked count 15.
- `M_T66_OutlinePostProcess`: cooked count 5.

These are material stat CSV counts, not live `ProfileGPU` pass cost.

## Mesh LOD Spot Checks

Representative readable metadata:

- Hero: `SM_Hero_1_Chad_QuadRetro`
  - 4 LODs
  - Nanite enabled
  - 21,416 triangles
  - 45,524 vertices

- Enemy: `SM_Dungeon_BoneJailer_QuadRetro`
  - 4 LODs
  - Nanite enabled
  - 28,075 triangles
  - 60,383 vertices
  - Nanite triangles 46,689
  - Nanite vertices 93,690

- Environment: `DungeonFloor_StoneSlabs_A_UnrealReady`
  - 3 LODs
  - Nanite enabled
  - 1,646 triangles
  - 2,756 vertices

- Prop/interactable: `Arcade_Machine_QuadRetro`
  - 1 LOD
  - Nanite disabled
  - 27,214 triangles
  - 56,807 vertices

- VFX mesh: `Arthur_Sword`
  - 1 LOD
  - Nanite enabled
  - 494 triangles
  - 430 vertices

Most readable Interchange metadata had `bAutoComputeLODScreenSizes=true` and empty `lODScreenSizes`, so explicit screen-size thresholds were not available from static string reads.

## Texture Spot Checks

- Hero UI portrait `T_Hero_1_Chad`
  - 1024x1024
  - `CompressionSettings=TC_EditorIcon`
  - `LODGroup=TEXTUREGROUP_UI`
  - `NeverStream=True`

- Enemy albedo `Dungeon_BoneJailer_QuadRetro_Pixelated_512`
  - 512x512
  - `CompressionSettings=TC_Default`
  - `LODGroup=TEXTUREGROUP_Character`
  - `NeverStream=False`

- Environment albedo `DungeonFloor_StoneSlabs_A_UnrealReady_BaseColor_00`
  - 2048x2048
  - `CompressionSettings=TC_Default`
  - `LODGroup=TEXTUREGROUP_World`
  - `Filter=TF_Nearest`
  - `NeverStream=False`

- Arcade machine texture `T_Arcade_Machine_QuadRetro_Pixelated_512`
  - 512x512
  - `CompressionSettings=TC_Default`
  - no mipmaps/nearest behavior seen in metadata

- Pixel particle texture `T_PixelParticle`
  - 256x256
  - `CompressionSettings=TC_Default`
  - `LODGroup=TEXTUREGROUP_World`
  - `Format=DXT1`
  - `Filter=TF_Nearest`
  - `NeverStream=False`

## Niagara and VFX

Binary-string scan found 256 `.uasset` files containing `NiagaraSystem`:

- `Content/Stylized_VFX_StPack`: 250
- `Content/UE5RFX`: 4
- `Content/VFX`: 2

Frequently referenced systems:

- `NS_PixelParticle`
- `VFX_Attack1`
- `P_Fire`
- `P_Dirt_Spikes_02`

VFX budget controls exist:

- Pixel VFX quality budgets in `T66PixelVFXSubsystem`.
- Combat imported VFX per-frame budgets in `T66CombatVFX`.
- Boss projectile and boss AOE VFX caps/scales.
- Scalability controls `r.EmitterSpawnRateScale` and `fx.Niagara.QualityLevel`.

Particle count limits, CPU/GPU sim type, and Effect Type assignment could not be reliably extracted without editor/runtime inspection.

## Lighting and Shadows

Runtime lighting setup:

- Removes legacy atmosphere/directional/skylight/fog actors.
- Creates a tagged SkyLight.
- Disables SkyLight shadows.
- Uses theme atmosphere data.

Dungeon theme:

- SkyLight intensity is set to 0 after atmosphere apply.
- Carry light intensity is set to 0.

Fog:

- Uses `AExponentialHeightFog`.
- Respects player fog settings.
- Volumetric fog explicitly disabled in runtime setup.
- Scalability also disables volumetric fog at ShadowQuality 0/1.

Torches:

- Movable point lights.
- Shadows disabled.
- Inverse-squared falloff disabled.
- Count capped by theme spec.

## Retro Post FX

Defaults in structs/save defaults:

- `bRetroFXMasterEnabled=true`
- `bEnableRetroFXMaster=true`
- `UIFullScreenCRTEnabled=true`

Runtime behavior:

- `T66RetroFXSubsystem` disables/zeroes effect fields when master is off.
- Actual active state depends on user save state.

No `stat gpu` or `ProfileGPU` capture was found, so pass cost is unknown.

