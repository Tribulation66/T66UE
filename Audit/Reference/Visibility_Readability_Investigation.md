# Visibility & Readability Investigation

Status: Reference
Date: 2026-05-12
Scope: Read-only fact-finding pass for current Chadpocalypse enemy and character visibility state.

No project code or assets were intentionally changed for this investigation.

## 1. Character and Enemy Materials

Current character visual data comes through `C:\UE\T66\Content\Data\CharacterVisuals.csv` and `C:\UE\T66\Source\T66\Core\T66CharacterVisualSubsystem.cpp`.

| Master | Current use | Shading / blend | Inputs | Opacity / custom data | Edge / rim / fresnel / view lighting |
|---|---:|---|---|---|---|
| `/Game/Materials/M_GLB_Unlit` | 72 loaded character mesh slots, including player/enemies sampled | Unlit, Opaque, two-sided | EmissiveColor = texture/tint/brightness multiply | No Opacity, no OpacityMask, no CustomData0/1 | None found |
| `/Game/Materials/M_Environment_Unlit` | 3 NPC character slots: Saint, Ouroboros, Gambler | Unlit, Opaque, two-sided | EmissiveColor multiply | No opacity/custom data | None found |
| `/Game/Materials/M_Character_Unlit` | Current import/fallback master, not observed as loaded row parent in this pass | Unlit, Masked, two-sided | EmissiveColor multiply, OpacityMask texture sample | Uses OpacityMask only; no custom data | None found |
| `/Game/Materials/M_FBX_Unlit` | Current import/fallback master, not observed as loaded row parent in this pass | Unlit, Opaque, two-sided | EmissiveColor texture sample | No opacity/custom data | None found |

Specific enemy material instances sampled all use `/Game/Materials/M_GLB_Unlit` as parent:

| Enemy | Material instance override | Key overrides |
|---|---|---|
| Dungeon Slime | `/Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/Dungeon_Slime_QuadRetro/Materials/Dungeon_Slime_QuadRemesh_Pixelated_Unlit` | `Brightness=1`, `Tint=white`, `Opacity=0`, texture params all point to `Dungeon_Slime_QuadRetro_Pixelated_512` |
| Dungeon Skeleton | `/Game/Characters/Enemies/Regular/Dungeon_Skeleton/QuadRetro/Dungeon_Skeleton_QuadRetro/Materials/Dungeon_Skeleton_QuadRemesh_Pixelated_Unlit` | Same scalar pattern, texture params point to skeleton pixelated texture |
| Forest Mushroom Brute | `/Game/Characters/Enemies/Regular/Forest_MushroomBrute/QuadRetro/Forest_MushroomBrute_QuadRetro/Materials/Forest_MushroomBrute_QuadRemesh_Pixelated_Unlit` | Same scalar pattern, texture params point to mushroom brute pixelated texture |
| Martian Drone Grunt | `/Game/Characters/Enemies/Regular/Martian_DroneGrunt/QuadRetro/Martian_DroneGrunt_QuadRetro/Materials/Martian_DroneGrunt_QuadRemesh_Pixelated_Unlit` | Same scalar pattern, texture params point to drone grunt pixelated texture |
| Hell Great Dragon | `/Game/Characters/Enemies/Bosses/Hell_GreatDragon/QuadRetro/Hell_GreatDragon_QuadRetro/Materials/Hell_GreatDragon_QuadRemesh_Pixelated_Unlit` | Same scalar pattern, texture params point to great dragon pixelated texture |

No view-space or camera-relative lighting math was found in the current character master materials.

Source/script references inspected:

- `C:\UE\T66\Scripts\MakeGLBImportsUnlit.py`
- `C:\UE\T66\Scripts\MakeCharacterMaterialsUnlit.py`
- `C:\UE\T66\Source\T66\Core\T66CharacterVisualSubsystem.cpp`

## 2. Post-Process Pipeline

Serialized gameplay map post-process volumes:

| Map | Serialized PP volumes | Directional / sky / fog actors |
|---|---:|---|
| `/Game/Maps/GameplayLevel` | 0 | none |
| `/Game/Maps/Gameplay_Tutorial` | 0 | none |
| `/Game/Mini/Maps/T66MiniBattleMap` | 0 | none |
| `/Game/Maps/LabLevel` | 0 | DirectionalLight + SkyLight |
| `/Game/Maps/FrontendLevel` | 0 | DirectionalLight + SkyLight |

Runtime source then creates/normalizes post-process state:

| Order | Source | Runtime material / effect | Enabled state |
|---:|---|---|---|
| 1 | `C:\UE\T66\Source\T66\Gameplay\T66WorldVisualSetup.cpp` | Neutral unbound PP volume, no material chain | Ensures fixed exposure, AO off, bloom off, saturation 0.95 |
| 2 | `C:\UE\T66\Source\T66\Core\T66RetroFXSubsystem.cpp` | `DEV_RetroFX_PostProcessVolume`, priority `5000` | Created during Retro FX apply |
| 3 | same | N64 blur DMI | Weight from `N64BlurBlendPercent` |
| 4 | same | N64 replace-tonemapper DMI | Weight from `N64BlurBlendPercent` |
| 5 | same | PS1 DMI / `/Game/Materials/Retro/PS1/MI_T66_PS1_C*_S*_B*` variant | Weight from PS1 blend/fog settings |
| 6 | same | `/Game/Materials/Retro/M_RetroChromaticAberrationPostProcess` | Weight from chromatic settings |
| 7 | `C:\UE\T66\Source\T66\Core\T66PixelationSubsystem.cpp` | `/Game/UI/M_PixelationPostProcess` | Blend weight `1` only if world or character pixelation level > 0 |

Pixelation / dither state:

| Item | Current fact |
|---|---|
| Internal render resolution | Baseline `r.ScreenPercentage.Default=100` in `C:\UE\T66\Config\DefaultEngine.ini`; real low-res mode is off by default in `FT66RetroFXSettings` |
| Real low-res path | If enabled, `T66RetroFXSubsystem` computes `r.ScreenPercentage` from target height; default `TargetResolutionHeightPercent=60` maps to about 504 px target height |
| Output resolution | Final output remains the active game viewport/backbuffer |
| Pixel grid formula | `Level 0 = 320`; otherwise `320 + (10 - Level) * 40`, so level 1 = 680 and level 10 = 320 |
| Dither source | UE5RFX PS1 post-process assets and material functions, including imported Bayer variants; runtime sets `Dithering Strength` and selects PS1 variant by Bayer/color/fog flags |
| Active during play | Yes. `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp` applies Retro FX settings during gameplay transition. This is not shipping-only. Actual weights depend on player settings/save state. |

Outline / silhouette / edge implementation search:

| Path | Finding | Enabled state |
|---|---|---|
| `C:\UE\T66\Config\DefaultEngine.ini` | `r.CustomDepth=3`, stencil enabled | Globally enabled |
| `C:\UE\T66\Source\T66\Core\T66RetroFXSubsystem.cpp` | Sets CustomDepth/stencil values `1` world, `2` character for pixelation masks | Only when world/character pixelation percent > 0 |
| `C:\UE\T66\Content\UI\M_PixelationPostProcess.uasset` | Binary asset references CustomDepth/stencil mask path | Runtime weight only when pixelation levels > 0 |
| `C:\UE\T66\Source\T66\Gameplay\T66PlayerController.cpp` | Camera wall occlusion fade uses `/Game/Materials/M_CameraWallOccluderFade` | Enabled by default cvar for occluding walls; not enemy outline |
| `C:\UE\T66\Source\T66\Gameplay\T66HeroOneAttackVFX.cpp` | `OutlineColor` parameter for attack VFX | VFX only, not silhouette |
| `C:\UE\T66\Scripts\SetupIdolPixelVFX.py` | `OutlineColor` parameter | VFX setup only |
| `C:\UE\T66\Content\UE5RFX\Materials\MaterialFunctions\UE5RFX_CustomDepthTest.uasset` | Imported UE5RFX function exists | No current T66 runtime source reference found |
| Whole-source search | No active Sobel, inverted-hull, edge-detect, Fresnel rim, or character CustomDepth outline implementation found | Not present/enabled |

## 3. Albedo Color Audit

Method: exported representative `Texture2D` assets to temp PNGs, then sampled dominant colors using a 32-color quantized palette. Unreal texture resource query reported `32x32` for the selected pixelated source assets; exported PNGs were `512x512` for characters and `2048x2048` for terrain.

```text
| Asset | Source texture | Top 5 dominant colors |
|---|---|---|
| Dungeon Slime | /Game/Characters/Enemies/Regular/Dungeon_Slime/.../Dungeon_Slime_QuadRetro_Pixelated_512 | #000000 7.1%, #36622A 4.9%, #091108 4.9%, #21401C 4.8%, #548146 4.6% |
| Dungeon Skeleton | /Game/Characters/Enemies/Regular/Dungeon_Skeleton/.../Dungeon_Skeleton_QuadRetro_Pixelated_512 | #000000 10.3%, #0D0A0C 4.2%, #161316 4.2%, #8F8E92 4.0%, #232124 4.0% |
| Forest Mushroom Brute | /Game/Characters/Enemies/Regular/Forest_MushroomBrute/.../Forest_MushroomBrute_QuadRetro_Pixelated_512 | #000000 28.2%, #3B342A 4.9%, #46432D 4.8%, #5C533E 4.2%, #57473F 4.1% |
| Martian Drone Grunt | /Game/Characters/Enemies/Regular/Martian_DroneGrunt/.../Martian_DroneGrunt_QuadRetro_Pixelated_512 | #000000 25.1%, #272524 5.7%, #040404 5.2%, #030303 5.0%, #221F1E 5.0% |
| Hell Great Dragon | /Game/Characters/Enemies/Bosses/Hell_GreatDragon/.../Hell_GreatDragon_QuadRetro_Pixelated_512 | #000000 43.1%, #0D0101 10.8%, #010100 4.0%, #110D05 3.3%, #090100 3.3% |
| Dungeon floor stone slabs | /Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonFloor_StoneSlabs_A_UnrealReady_BaseColor_00 | #92988C 8.9%, #93998D 7.9%, #474A44 5.3%, #82887D 4.6%, #91978B 4.6% |
| Dungeon wall stone blocks | /Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01/DungeonWall_StoneBlocks_A_UnrealReady_BaseColor_00 | #74776E 5.0%, #383A35 4.8%, #545650 4.8%, #595B54 4.7%, #484A45 4.6% |
```

## 4. Scale Audit

| Subject | Mesh | Bounds X/Y/Z UU | LOD0 verts | LOD0 tris | Primary texture resource | Capsule |
|---|---|---:|---:|---:|---|---|
| Player Hero 1 Chad | `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/SM_Hero_1_Chad_QuadRetro` | `142.4 / 89.8 / 197.8` | 45,524 | 21,416 | `32x32` reported resource, `RoyalChad_QuadRetro_Pixelated_512` | radius 34, half-height 100, height 200 |
| Representative enemy Dungeon Slime | `/Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/SM_Dungeon_Slime_QuadRetro` | `132.0 / 109.0 / 193.9` | 42,535 | 19,099 | `32x32` reported resource, `Dungeon_Slime_QuadRetro_Pixelated_512` | radius 34, half-height 88, height 176 |

Enemy-to-player height ratio:

- Mesh bounds ratio: `193.9 / 197.8 = 0.98`
- Capsule height ratio: `176 / 200 = 0.88`

Sources:

- `C:\UE\T66\Source\T66\Gameplay\T66HeroBase.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66EnemyBase.cpp`
- Unreal class default object query

## 5. Camera and Gameplay Framing

Gameplay camera:

- Player camera is `USpringArmComponent` + `UCameraComponent` in `C:\UE\T66\Source\T66\Gameplay\T66HeroBase.cpp`.
- Base spring arm length in hero constructor: `1440 UU`, socket offset/pivot location `Z=60`.
- Locked chase camera cvars in `C:\UE\T66\Source\T66\Gameplay\T66PlayerController.cpp`: preset default `0`; locked chase preset has pitch `-30`, arm length `1150`, pivot height `145`, forward offset `35`.
- Pitch clamp cvars: min `-72`, max `-4`.
- Projection is perspective for the main gameplay `FollowCamera`; no gameplay projection override was found.
- Gameplay FOV is treated as `90` in related camera code/comments; scoped ultimate overrides to `18`, with wheel zoom clamp `8..30`.
- Normal mouse-wheel zoom changes spring-arm length, clamped `350..2800`.
- Combat/camera modulation exists: scoped ultimate camera/FOV override, mouse-wheel zoom, locked chase mode, optional side-wall spring, and camera wall occlusion fade.

Mini battle camera:

- `C:\UE\T66\Source\T66Mini\Private\Gameplay\T66MiniPlayerPawn.cpp` uses orthographic camera, `OrthoWidth=2800`.
- Mini framing reads viewport size and falls back to `1920x1080`, aspect `16:9`.

Resolution/aspect assumptions:

- UI reference/layout code is heavily normalized to `1920x1080`, including generated files under `C:\UE\T66\Source\T66\UI\Style\*ReferenceLayout.generated.h`.
- `C:\UE\T66\Config\DefaultEngine.ini` has a UI scale curve keyed by heights `720, 800, 900, 1080, 1440, 2160`.
- `C:\UE\T66\Source\T66\Gameplay\T66FrontendGameMode.cpp` sets frontend camera FOV `90` and `bConstrainAspectRatio=false`.

## 6. Lighting in Gameplay Maps

Serialized map lighting:

| Map | Directional Light | Sky Light | Sky Atmosphere | Exponential Height Fog | PP exposure actor state |
|---|---|---|---|---|---|
| `/Game/Maps/GameplayLevel` | none | none | none | none | no serialized PP volume |
| `/Game/Maps/Gameplay_Tutorial` | none | none | none | none | no serialized PP volume |
| `/Game/Mini/Maps/T66MiniBattleMap` | none | none | none | none | no serialized PP volume |
| `/Game/Maps/LabLevel` | `DirectionalLight_0`, intensity `10`, white, indirect `1`, rotation `pitch -89 yaw 180 roll 180` | `SkyLight_0`, intensity `1`, white, indirect `1` | none | none | no serialized PP volume |
| `/Game/Maps/FrontendLevel` | `DirectionalLight_0`, intensity `4`, white, indirect `1`, rotation `pitch 35 yaw 0 roll -35` | `SkyLight_0`, intensity `0.35`, white, indirect `1` | none | none | no serialized PP volume |

Runtime lighting/exposure:

- `C:\UE\T66\Source\T66\Gameplay\T66WorldVisualSetup.cpp` destroys DirectionalLight, SkyLight, SkyAtmosphere, and ExponentialHeightFog actors when neutral setup runs.
- Same file ensures an unbound PP volume with fixed exposure: `AutoExposureMinBrightness=1`, `AutoExposureMaxBrightness=1`, AO intensity `0`, bloom `0`, bloom threshold `10`, saturation `0.95`.
- `C:\UE\T66\Config\DefaultEngine.ini` has `r.DefaultFeature.AutoExposure=False`.
- Calls to neutral setup exist in gameplay/frontend paths, including:
  - `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Bootstrap.cpp`
  - `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_MainMap.cpp`
  - `C:\UE\T66\Source\T66\Gameplay\T66GameMode.cpp`
  - `C:\UE\T66\Source\T66\Gameplay\T66FrontendGameMode.cpp`
  - `C:\UE\T66\Source\T66\Gameplay\T66PlayerController.cpp`

No view-space or camera-relative lighting setup was found anywhere in the inspected project source. Camera-relative systems found are framing, zoom, scoped FOV, wall spring, and wall occlusion fade, not lighting.

## Verification

Evidence gathered during this pass:

- Repo source searches with `rg` across `Source`, `Scripts`, `Config`, `Gameplay`, and relevant `Content` asset strings.
- Unreal commandlet metadata extraction against `C:\UE\T66\T66.uproject`.
- Texture export and dominant-color sampling from representative albedo textures.
- Unreal class default object query for hero/enemy capsule defaults.
- Map asset enumeration with `rg --files Content | rg "\.umap$"`.
