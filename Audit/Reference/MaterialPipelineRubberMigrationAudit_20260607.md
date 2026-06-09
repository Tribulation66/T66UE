# T66 Material, Texture, Color, And Lighting Pipeline Audit

Date: 2026-06-07

Purpose: document the current material, texture, color, VFX, and lighting pipeline to plan a possible migration to a single shared rubber-style master material where color is the only per-object variable.

Scope: read-only audit evidence plus one user-requested markdown file. No source code, assets, config, data, build outputs, or Unreal content were modified.

Validator status: Claude independent answer and cross-review returned `Result: OK` for the original audit. A follow-up Claude validator pass returned `Result: OK` for writing this single markdown artifact.

Evidence strength note: shading model and blend-mode values below are from current project docs plus `.uasset` binary string scans, not a UE editor asset readback. A UE editor readback would be the authoritative confirmation for every material instance and per-map actor, but it was not run because even read-only editor probes can create project `Saved/` artifacts.

## 1. Color Source On Imported Meshes

Raw Pixal3D color is texture-driven.

Sampled raw GLBs from current hero, enemy, and prop outputs:

- `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`
- `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532/Outputs/BoneWalker.glb`
- `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532/Outputs/WeaponAltar_Pixal3D.glb`

Each sampled GLB had:

- `1` mesh
- `1` primitive
- attributes `POSITION`, `NORMAL`, `TEXCOORD_0`
- no `COLOR_0`
- `1` material
- `2` embedded WebP images/textures
- material `baseColorTexture` present
- `baseColorFactor` set to white `[1, 1, 1, 1]`

This rules out vertex colors and flat material constants as the primary raw Pixal3D color carrier. The raw source color is baked into UV-mapped texture data.

### Category Summary

| Mesh category | Current color source | Typical material setup |
| --- | --- | --- |
| Heroes | Mixed runtime rows: FriendSlop raw, AnimatedToonStyle, Pixal3DToonStyle, and legacy/QuadRetro references. Base color comes from texture assets, not vertex color. | Raw/static hero meshes typically use one material slot. ToonStyle uses one shading mesh/material plus a separate outline mesh/material. |
| Enemies / mobs | Mixed legacy, Pixal3D/Easy, and ToonStyle paths. Pixal3D source is baked UV texture. | Usually one material slot. ToonStyle outline is a separate mesh where present. |
| Props / world imports | Pixal3D base-color texture, often imported as `T_<AssetID>_BaseColor` or as ToonStyle texture set. | Usually one material slot. |

### Raw FriendSlop Export Evidence

Current FriendSlop Easy raw FBX export reports under:

`Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532/RawTexturedFBX`

showed:

- `49` raw textured export reports
- all `49` had `1` material
- all `49` had `2` textures
- all texture pairs were `4096x4096, 4096x4096`

The raw import guideline says to:

1. Import `Textures/<AssetID>_00_Image_0.png` as `/Game/.../Textures/T_<AssetID>_BaseColor`.
2. Create/update `/Game/.../Materials/MI_SM_<AssetID>` with parent `/Game/Materials/M_GLB_Unlit`.
3. Bind `BaseColorTexture` and `DiffuseColorMap`.
4. Set `Tint = (1,1,1,1)`, `Brightness = 1.0`, `Opacity = 1.0`.
5. Assign the material instance to every imported `StaticMesh` material slot.

### ToonStyle Production Evidence

Production manifests under:

`SourceAssets/ToonStyle/Pixal3D/Production`

showed:

| Asset class | Count | Post-strip material count | Source texture count | Texture size |
| --- | ---: | ---: | ---: | --- |
| humanoid | 30 | 1 | 2 | 4096x4096 |
| creature | 6 | 1 | 2 | 4096x4096 |
| prop | 24 | 1 | 2 | 4096x4096 |

The ToonStyle pipeline keeps texture-driven color, but processes it:

- raw Pixal3D WebP textures are extracted
- texture color is flattened/postprocessed
- a tint texture is generated
- an inner-line texture is baked
- material texture refs are stripped before FBX export
- vertex colors are authored for shader behavior
- Unreal import binds the processed textures to ToonStyle material instances

Typical ToonStyle static production setup:

- one shading mesh with one material slot
- one separate inverted-hull outline mesh with one outline material slot
- not two slots on the same mesh

### Runtime Visual Row Mix

`Content/Data/CharacterVisuals.csv` has `136` rows. Current row-family grouping by path content:

| Runtime group | Count |
| --- | ---: |
| Mobs, legacy/other | 40 |
| Other game, QuadRetro | 19 |
| Heroes, AnimatedToonStyle | 18 |
| Heroes, QuadRetro | 14 |
| Companions, legacy/other | 12 |
| Mobs, Pixal3D/Easy | 12 |
| Other game, Pixal3D/Easy | 10 |
| Companions, AnimatedToonStyle | 5 |
| Companions, Pixal3D/Easy | 3 |
| Heroes, FriendSlopRaw / PhysicsFirst | 2 |
| Other game, legacy/other | 1 |

This means a rubber migration cannot be scoped only to the newest ToonStyle production manifests. Runtime visual rows still reference several families.

## 2. Current Materials

The main project-authored material families found under `Content/Materials`, `Content/ToonStyle/Materials`, `Content/VFX`, and `Content/VFXLab` are grouped below.

### Core Character / Environment / Shared Materials

| Content path | Role | Shading / blend evidence |
| --- | --- | --- |
| `/Game/Materials/Generated/M_Unlit_DiffuseColorMap` | generated unlit diffuse map master | unlit family by name; not individually editor-read |
| `/Game/Materials/M_CameraWallOccluderFade` | camera occluder fade material | fade/occluder special case; not a rubber candidate |
| `/Game/Materials/M_Character_Unlit` | legacy character unlit master | binary scan: Unlit / Masked |
| `/Game/Materials/M_EasyMobVAT_Unlit_UV2` | EasyMob VAT unlit material | unlit VAT family by name; not a general rubber candidate |
| `/Game/Materials/M_Environment_Lit` | environment lit fallback | binary scan: Default Lit / Opaque |
| `/Game/Materials/M_Environment_Unlit` | environment unlit master | binary scan: Unlit / Opaque |
| `/Game/Materials/M_FBX_Unlit` | FBX import unlit master | unlit family; used by import/retro paths |
| `/Game/Materials/M_GLB_Unlit` | GLB/Pixal3D unlit master | binary scan: Unlit / Opaque |
| `/Game/Materials/M_GLB_ViewSpaceLit_Character` | retained view-space/cel character A/B master | binary scan: Unlit / Opaque |
| `/Game/Materials/M_TorchFlicker_LightFn` | light function material | lighting special case |
| `/Game/Materials/MI_GLB_Unlit_Character_Shared` | shared GLB character material instance | inherits `M_GLB_Unlit` family |

Characters are currently in the Unlit material family for the active GLB, ToonStyle, and view-space/cel masters. The main exception in the core environment stack is `M_Environment_Lit`, which remains a Default Lit fallback.

### ToonStyle Materials

| Content path | Role | Shading / blend evidence |
| --- | --- | --- |
| `/Game/ToonStyle/Materials/M_Toon_Character` | ToonStyle character shading master | docs and binary scan: Unlit / Opaque |
| `/Game/ToonStyle/Materials/M_Toon_Character_Outline` | inverted-hull outline master | docs and binary scan: Unlit / Opaque |
| `/Game/ToonStyle/Materials/M_Toon_Environment` | ToonStyle environment master | docs and binary scan: Unlit / Opaque |

These are existing toon/cel-like masters. They are not rubber/clay masters, and they are texture plus vertex-color driven.

### Retro / PS1 Materials

| Content path | Role |
| --- | --- |
| `/Game/Materials/Retro/M_Character_Unlit_RetroGeometry` | character retro geometry replacement |
| `/Game/Materials/Retro/M_Environment_Unlit_RetroGeometry` | environment retro geometry replacement |
| `/Game/Materials/Retro/M_FBX_Unlit_RetroGeometry` | FBX retro geometry replacement |
| `/Game/Materials/Retro/M_GLB_Unlit_RetroGeometry` | GLB retro geometry replacement |
| `/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess` | chromatic post-process |
| `/Game/Materials/Retro/M_T66_OutlinePostProcess` | outline post-process |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C0_S0_B0` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C0_S0_B1` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C0_S1_B0` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C0_S1_B1` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C1_S0_B0` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C1_S0_B1` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C1_S1_B0` | PS1 variant |
| `/Game/Materials/Retro/PS1/MI_T66_PS1_C1_S1_B1` | PS1 variant |

These are tied to `UT66RetroFXSubsystem` material replacement and post-process logic. A rubber master migration needs a RetroFX update or an explicit exclusion.

### VFX Materials

Key project VFX material families:

- `/Game/VFX/Foundation/OutgoingTravelers/M_OutgoingTravelerPool_AdditiveFamily`
- `/Game/VFX/Foundation/OutgoingTravelers/M_OutgoingTravelerPool_TranslucentFamily`
- `/Game/VFX/Foundation/OutgoingTravelers/Profiles/Materials/MI_TravelerVisual_*`
- `/Game/VFX/Hero1/Axe/Shared/M_Hero1AxeAOE_*`
- `/Game/VFX/Hero1/M_Hero1_PixelAttack`
- `/Game/VFX/Hero1/MI_Hero1_Attack_Impact`
- `/Game/VFX/Hero1/MI_Hero1_Attack_Streak`
- `/Game/VFX/M_PixelSprite`
- `/Game/VFX/Projectiles/Hero1/Arthur_Sword/Materials/M_Blade`
- `/Game/VFX/Projectiles/Hero1/Arthur_Sword/Materials/M_Fuller`
- `/Game/VFX/Projectiles/Hero1/Arthur_Sword/Materials/M_Grip`
- `/Game/VFX/Projectiles/Hero1/Arthur_Sword/Materials/M_Guard`
- duplicate Hero1Axe lab materials under `/Game/VFXLab/Hero1Axe/Shared/`

Binary scans confirmed:

- `M_OutgoingTravelerPool_AdditiveFamily`: Unlit / Additive
- `M_OutgoingTravelerPool_TranslucentFamily`: Unlit / Translucent

VFX materials should not be folded into a first-pass opaque rubber master swap.

### Existing Rubber / Clay / Toon / Cel Materials

No runtime UE Content rubber or clay master material was found by path search.

Existing toon/cel-style assets do exist:

- `/Game/ToonStyle/Materials/M_Toon_Character`
- `/Game/ToonStyle/Materials/M_Toon_Character_Outline`
- `/Game/ToonStyle/Materials/M_Toon_Environment`
- `/Game/Materials/M_GLB_ViewSpaceLit_Character`

Rubber appears only in Blender/lookdev recipe artifacts under:

`Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/LookDev/Hero_1_Chad_Male_Rubber_20260605`

Those files are not runtime UE master materials.

## 3. Vertex Color Usage

Raw Pixal3D GLBs sampled have no `COLOR_0` attribute, so raw Pixal3D color is not vertex color.

ToonStyle production explicitly uses vertex colors functionally. They are not free to repurpose as base color without replacing existing shader behavior.

Authoritative channel layout from `ToonStyle/Docs/MaterialSpec.md` and `ToonStyle/Docs/PipelineSpec.md`:

| Channel | Current function |
| --- | --- |
| R | Lighting threshold offset, AO-derived, used by `M_Toon_Character` |
| G | Outline width multiplier and outline mask; `G=0` means no outline extrusion |
| B | Outline depth offset used in outline world-position offset |
| A | Unused in production |

Important UE 5.7 issue: `VertexColor.A` was unavailable in the outline WPO path, so mask behavior is folded into `G`. This is also called out in `ToonStyle/Source/pending_issues_Source.md`.

Migration implication: a future rubber master cannot assume vertex colors are available for base color across ToonStyle assets. The existing ToonStyle vertex colors are load-bearing for lighting threshold and outline behavior.

## 4. Import Pipeline

### ToonStyle Production Import

Canonical route:

1. `Model Generation/Pixal3D/Scripts/run_pixal3d_toonstyle_production_import.py`
2. `ToonStyle/BlenderScripts/run_toon_pipeline.py`
3. `ToonStyle/Source/ImportPixal3DAsset_Phase1C.py`

Blender pipeline responsibilities:

- import raw Pixal3D GLB
- extract source textures
- join meshes
- normalize scale/orientation
- flatten/postprocess texture color
- generate tint texture
- bake inner-line texture
- author vertex colors on `Col`
- create inverted-hull outline mesh
- reverse outline face winding
- export shading FBX and outline FBX

Unreal import injection point:

`ToonStyle/Source/ImportPixal3DAsset_Phase1C.py`

Important constants:

```python
CHARACTER_PARENT_MATERIAL = "/Game/ToonStyle/Materials/M_Toon_Character"
OUTLINE_PARENT_MATERIAL = "/Game/ToonStyle/Materials/M_Toon_Character_Outline"
```

Material binding behavior:

- creates `MI_<Asset>` parented to `/Game/ToonStyle/Materials/M_Toon_Character`
- binds `BaseColorTexture`, `TintTexture`, `InnerLineTexture`
- creates `MI_<Asset>_Outline` parented to `/Game/ToonStyle/Materials/M_Toon_Character_Outline`
- binds or preserves `OutlineColor`
- assigns material slot `0` on the shading mesh
- assigns outline material slot `0` on the outline mesh

This is one primary injection point for a future rubber material assignment.

### FriendSlop Raw Pixal3D Import

Canonical route:

1. `Model Generation/Pixal3D/Scripts/export_accurig_textured_batch.py`
2. `Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py`
3. `Scripts/ApplyFriendSlopRawCharacterVisualRows.py`
4. `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py`
5. `Scripts/ValidateFriendSlopRawPixal3DAndExit.py`

Unreal import injection point:

`Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py`

Important constant:

```python
PARENT_MATERIAL = "/Game/Materials/M_GLB_Unlit"
```

Material behavior:

- imports `Textures/<AssetID>_00_Image_0.png`
- creates or updates `MI_SM_<AssetID>`
- parents it to `/Game/Materials/M_GLB_Unlit`
- binds `BaseColorTexture` and `DiffuseColorMap`
- sets `Tint`, `Brightness`, and `Opacity`
- loops through static mesh material slots and assigns the material instance

This is the second primary injection point for a future rubber material assignment.

### Animated Hero Import

Animated heroes are a separate active family:

- `Scripts/RunAnimatedToonStyleHeroImport.py`
- `Model Generation/Rigging and Animation/07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`

`CharacterVisuals.csv` confirms active hero rows can point at AnimatedToonStyle skeletal meshes and `PixelatedTextureAssetPath` base-color textures. A global rubber migration must include this path.

## 5. World / Environment Materials

World surfaces are currently material-plus-texture driven.

`Source/T66/Gameplay/T66TowerThemeVisuals.cpp` resolves theme surface material instances at:

```text
/Game/ToonStyle/Environment/<Theme>/Materials/MI_<Theme>_<Surface>
```

Themes include:

- `Forest`
- `Ocean`
- `Martian`
- `Hell`
- `Dungeon`

Surfaces include:

- `Floor`
- `Ceiling`
- `Wall_XZ`
- `Wall_YZ`

Direct C++ fallback evidence:

- `T66TowerThemeVisuals.cpp` defines `T66EnvironmentLitMaterialPath` as `/Game/Materials/M_Environment_Lit.M_Environment_Lit`.
- `ResolveTheme` falls back floor, wall, and roof materials to that path when theme materials are missing.
- `T66TerrainThemeAssets.cpp` also creates ground dynamic MIDs from `/Game/Materials/M_Environment_Lit.M_Environment_Lit`.

Texture binding behavior:

- `T66TowerThemeVisuals.cpp` binds theme textures into `DiffuseColorMap` and `BaseColorTexture`.
- `T66TerrainThemeAssets.cpp` binds `/Game/World/Terrain/TowerForest/T_TowerForestGround` into `DiffuseColorMap` and `BaseColorTexture`.

World/interactable material and texture folders are present under:

- `Content/World/VisualProps/Easy`
- `Content/World/Gates`
- `Content/World/Interactables/Boosts`
- `Content/World/Interactables/DifficultyTotem`
- `Content/World/Interactables/IdolAltar`
- `Content/World/Interactables/Crate`
- `Content/World/Interactables/LootWheel`
- `Content/World/Interactables/Fountain`
- `Content/World/Interactables/Chests/ChestModel`
- `Content/World/Interactables/WeaponAltar`
- `Content/World/LootBags/Shared`
- `Content/World/Interactables/Vehicles`
- `Content/World/Backrooms`

Backrooms texture paths:

- `/Game/World/Backrooms/Textures/T_Backrooms_Wall`
- `/Game/World/Backrooms/Textures/T_Backrooms_Floor`
- `/Game/World/Backrooms/Textures/T_Backrooms_Door`

`Content/World/pending_issues_World.md` says Backrooms textures are intentionally preserved. A rubber migration should treat Backrooms as an explicit decision point, not an automatic swap.

## 6. VFX / Niagara Materials

Current VFX materials are special-purpose effect materials, not part of the opaque character/world material family.

Main current project VFX families:

- outgoing traveler pooled visual masters and profile instances under `/Game/VFX/Foundation/OutgoingTravelers`
- Hero 1 axe AOE slash, ground trace, impact flare, mote, and spark materials under `/Game/VFX/Hero1/Axe/Shared`
- Hero 1 pixel attack material and impact/streak instances under `/Game/VFX/Hero1`
- shared `/Game/VFX/M_PixelSprite`
- Arthur sword projectile materials under `/Game/VFX/Projectiles/Hero1/Arthur_Sword/Materials`
- lab duplicates under `/Game/VFXLab/Hero1Axe/Shared`

Current state:

- outgoing traveler family masters are Unlit/Additive and Unlit/Translucent
- Hero/weapon VFX materials are authored for particles/projectiles/slashes, not static rubber surfaces
- third-party VFX/retro packs also exist under `Content/Stylized_VFX_StPack` and `Content/UE5RFX`

Migration implication: VFX should be excluded from a first opaque rubber master swap. It needs a separate VFX material strategy if the art direction also changes particles.

## 7. Lighting And Post-Process

### Gameplay

`Source/T66/Gameplay/T66GameMode.cpp` calls:

```cpp
FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(GetWorld());
```

`Source/T66/Gameplay/T66PlayerController.cpp` also calls the neutral setup path.

`FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld` removes legacy runtime lighting actors:

- `ASkyAtmosphere`
- `ADirectionalLight`
- `ASkyLight` except atmosphere-spared actors
- `AExponentialHeightFog` except atmosphere-spared actors
- tagged Quake sky actors
- legacy `T66QuakeSkyActor`
- legacy `T66EclipseActor`

It then creates or reuses an unbound post-process volume tagged `T66_NeutralPostProcess`.

Neutral post-process settings:

- AutoExposure min brightness: `1.0`
- AutoExposure max brightness: `1.0`
- Ambient occlusion intensity: `0.0`
- Bloom intensity: `0.0`
- Bloom threshold: `10.0`
- Color saturation: `(0.95, 0.95, 0.95, 1.0)`

### Frontend

`Source/T66/Gameplay/T66FrontendGameMode.cpp` also calls:

```cpp
FT66WorldVisualSetup::EnsureNeutralVisualSetupForWorld(World);
```

The code comment says the frontend map is now an invisible shell, but neutral setup is retained to avoid latent dependencies. Frontend RetroFX is driven by the UI retainer path, not the gameplay RetroFX subsystem.

### Renderer Baseline

`Config/DefaultEngine.ini` contains:

- `r.AllowStaticLighting=False`
- `r.DefaultFeature.AutoExposure=False`
- `r.DefaultFeature.AmbientOcclusion=False`
- `r.DefaultFeature.MotionBlur=False`
- `r.DefaultFeature.LensFlare=False`

`Gameplay/World/T66_LIGHTING_REFERENCE.md` matches this: there is no active runtime sky/day-night rig, and gameplay/frontend previews use the neutral visual setup.

### Retro / PS1 Stack

RetroFX exists but defaults off.

`Source/T66/Core/T66RetroFXSettings.h` defaults:

- `bEnableRetroFXMaster = false`
- `PS1BlendPercent = 0.0f`

`UT66RetroFXSubsystem` can load and apply:

- PS1 post-process material variants
- outline post-process material
- chromatic aberration material
- retro geometry replacement materials for character, environment, FBX, and GLB families

Migration implication: there is no baked-lighting dependency blocking a rubber material migration. The project is already mostly Unlit-first. The main lighting/material decision is whether rubber should remain Unlit/Opaque to match current runtime behavior or introduce a lit/specular model.

## 8. Texture Assets

### Raw Pixal3D Source

Raw Pixal3D GLBs embed WebP images. Sampled current GLBs contained:

- `2` images
- `2` textures
- `image/webp`
- `4096x4096` source size in raw export reports

### FriendSlop Raw Export

Raw textured FBX export extracts PNGs named:

- `<AssetID>_00_Image_0.png`
- `<AssetID>_01_Image_1.png`

Current FriendSlop Easy batch evidence:

- 49 assets
- 2 textures each
- both textures `4096x4096`

UE raw import convention:

```text
/Game/.../Textures/T_<AssetID>_BaseColor
```

### ToonStyle Production

ToonStyle manifests record:

- source format: `WEBP`
- two source textures per asset
- source size: `4096x4096`
- flattened PNG output
- generated tint PNG
- generated inner-line PNG
- UV padding/postprocess reports

UE ToonStyle texture naming convention:

- `T_<Asset>_0`
- `T_<Asset>_1`
- `T_<Asset>_Tint`
- `T_<Asset>_InnerLines`
- `/Game/ToonStyle/Textures/T_InnerLines_DefaultBlack`

### World / Environment

World and interactable texture conventions include:

- `T_*_Pixal3D_BaseColor`
- `*_BaseColor_00`
- `T_Backrooms_Wall`
- `T_Backrooms_Floor`
- `T_Backrooms_Door`
- theme/environment textures under `/Game/World/Terrain` and `/Game/ToonStyle/Environment`

Texture origin is generally direct Pixal3D output, optionally processed by ToonStyle/Blender tooling for flattened color, tint, UV padding, and inner-line bakes.

## 9. Scope And Hard Blockers For A Single Rubber Master Swap

### Rough Counts

Broad prefix counts across `Content/`:

| Asset class | Count |
| --- | ---: |
| mesh-like `SM_` / `SK_` uassets | 528 |
| material-like uassets | 604 |
| `MI_*` instances | 348 |
| `M_*` master-like assets | 147 |
| `Material_*` named assets | 109 |

Mesh-like grouping:

| Group | Count |
| --- | ---: |
| heroes | 129 |
| enemies / mobs | 115 |
| world / props / weapons / items | 57 |
| other | 227 |

Pipeline-specific evidence:

- `136` active `CharacterVisuals.csv` rows
- `60` ToonStyle production manifests
- `49` current FriendSlop Easy raw textured exports

These counts are glob/prefix-derived estimates, not an editor asset dependency graph. They are useful for migration sizing but should not be treated as a complete swap list.

### Hard Blockers / Migration Risks

1. Pixal3D color is baked UV texture.
   - A color-only rubber master means discarding texture detail or deriving a dominant/palette color per asset.

2. ToonStyle vertex colors are functional.
   - R/G/B drive threshold, outline width/mask, and depth offset. They cannot be globally repurposed as base color without replacing those mechanisms.

3. ToonStyle outline is a separate mesh/material path.
   - A single rubber master has to decide whether the inverted-hull outline remains, is folded into the rubber material, or is removed.

4. Active runtime visuals are mixed.
   - Runtime references include legacy, QuadRetro, AnimatedToonStyle, Pixal3D/Easy, and FriendSlopRaw families.

5. World geometry is texture driven.
   - Floors, walls, ceilings, props, interactables, gates, loot objects, and Backrooms textures all currently rely on material texture bindings.

6. Backrooms textures are intentionally preserved.
   - They should be an explicit art/design decision, not a blind swap.

7. RetroFX hard-codes current material families.
   - Geometry replacement paths distinguish character, environment, FBX, and GLB source materials. Rubber migration needs a new retro variant or RetroFX update.

8. VFX is not compatible with an opaque rubber master.
   - Additive/translucent/emissive particle/projectile materials need separate handling.

9. `M_Environment_Lit` remains a live fallback.
   - Even though the project is Unlit-first, the world terrain/theme fallback still uses Default Lit/Opaque `M_Environment_Lit`.

10. No baked lighting dependency blocks the swap.
    - Static lighting is disabled and neutral runtime setup removes legacy sky/light actors.

### Recommended Migration Shape

The least disruptive rubber migration would be:

1. Keep VFX and post-process out of scope initially.
2. Make the shared rubber master Unlit/Opaque unless the user explicitly wants new lighting behavior.
3. Add rubber assignment injection points first in:
   - `ToonStyle/Source/ImportPixal3DAsset_Phase1C.py`
   - `Scripts/ImportFriendSlopRawPixal3DFBXAndExit.py`
   - AnimatedToonStyle import tooling
   - world/theme material resolution code
4. Decide whether rubber color is:
   - manually authored per object,
   - generated from texture dominant color,
   - generated from texture palette clusters,
   - or stored in data tables.
5. Decide whether ToonStyle outline/vertex-color behavior is preserved, ported, or removed.
6. Update RetroFX geometry-material replacement after the base rubber family exists.
7. Validate through a staged standalone build and Unreal-owned captures only after implementation starts.

## Verification Performed For This Audit

- Read root/user-provided T66 process instructions.
- Read `.t66/operator-state.json`; current state was Codex operator, Claude validator.
- Read `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Read folder routers and relevant docs:
  - `Audit/AUDIT_AGENTS.md`
  - `Audit/README.md`
  - `Model Generation/MODEL_GENERATION_AGENTS.md`
  - `Model Generation/Pixal3D/PIXAL3D_AGENTS.md`
  - `Gameplay/GAMEPLAY_AGENTS.md`
  - `Gameplay/World/WORLD_AGENTS.md`
  - `UI/UI_AGENTS.md`
  - `Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`
  - `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`
  - `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
  - `Gameplay/World/T66_LIGHTING_REFERENCE.md`
  - `ToonStyle/Docs/MaterialSpec.md`
  - `ToonStyle/Docs/PipelineSpec.md`
- Read relevant pending issue files under `Content/Materials`, `Content/World`, `Model Generation`, `Model Generation/Pixal3D`, and `ToonStyle/Source`.
- Sampled raw GLB JSON chunks for hero, enemy, and prop assets.
- Summarized ToonStyle production manifest counts.
- Summarized FriendSlop raw textured export reports.
- Scanned selected material `.uasset` binaries for shading model and blend mode strings.
- Reviewed current runtime C++ for neutral lighting, terrain/theme material fallback, RetroFX material paths, and import material binding paths.
- Ran Claude independent answer and Claude cross-review for the original audit; both returned `Result: OK`.
- Ran Claude independent answer for this file-writing request; it returned `Result: OK`.

## Token Notes From Original Audit

Original audit final report token section:

- Codex token spent: `166,406` latest completed Codex turn before final answer; final answer tokens were not included.
- Claude tokens spent: `624,255` total, made of `479,594` independent answer plus `144,661` cross-review.

This follow-up file-writing request also invoked Claude validator:

- Claude tokens spent for file-writing validation: `105,375`.

