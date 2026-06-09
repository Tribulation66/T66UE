# Hero 1 Chad Male Rubber Look-Dev Recipe

## Source

- Asset: `Hero_1_Chad_Male`
- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Source image: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Sources\Hero_1_Chad_Male.png`
- Blender file: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`
- Provenance basis: the look-dev scene imports directly from the FriendSlop Pixal3D source GLB above, not from any archived AccuRig asset.

## Locked Lighting Rig

- Engine: EEVEE Next when available, EEVEE fallback.
- World color: white `(1, 1, 1)`.
- View transform: `Standard`, look `Medium High Contrast`, exposure `0`, gamma `1`.
- Shadows are disabled on all area lights; ambient occlusion is weak and used only for form readability.
- Lights:
  - `Key_Softbox_L`: area, location `(-5.5, -5.2, 5.0)`, energy `470`, size `6.5`, shadows off.
  - `Key_Softbox_R`: area, location `(5.5, -5.2, 4.6)`, energy `360`, size `7.0`, shadows off.
  - `Top_Soft_Fill`: area, location `(0, -1.5, 7.2)`, energy `230`, size `8.0`, shadows off.
  - `Front_Fill`: area, location `(0, -7.5, 2.1)`, energy `150`, size `9.0`, shadows off.

## Material Node Graph

Per original material slot:

1. Image Texture from the imported GLB material, when present.
2. Hue/Saturation/Value node.
3. Bright/Contrast node with negative contrast to reduce Pixal3D texture noise.
4. Principled BSDF.
5. Optional Noise Texture -> Bump -> Principled Normal, very low strength.
6. Material Output.

Geometry softness is a non-destructive Bevel modifier plus Weighted Normal modifier per rubber object.

## Variation Matrix

| ID | Roughness | Specular IOR | Coat | Coat Rough | Subsurface | Saturation | Value | Contrast | Bevel | Bump | Notes |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| V01_soft_satin | 0.46 | 0.66 | 0.16 | 0.3 | 0.06 | 1.12 | 1.02 | -0.08 | 0.012 | 0.006 | Control rubber pass with a mild satin sheen. |
| V02_rubber_pop | 0.34 | 0.78 | 0.3 | 0.2 | 0.1 | 1.24 | 1.07 | -0.16 | 0.024 | 0.004 | Balanced first candidate: saturated, soft, and broad highlight. |
| V03_vinyl_bounce | 0.25 | 0.88 | 0.46 | 0.12 | 0.07 | 1.34 | 1.08 | -0.24 | 0.034 | 0.003 | Glossier vinyl direction, likely close if the critique is 'not rubbery enough'. |
| V04_candy_rubber | 0.29 | 0.82 | 0.34 | 0.16 | 0.18 | 1.3 | 1.12 | -0.3 | 0.044 | 0.002 | Softest bouncy candidate; more subsurface and edge rounding. |
| V05_matte_gum | 0.56 | 0.62 | 0.1 | 0.36 | 0.16 | 1.18 | 1.06 | -0.22 | 0.03 | 0.004 | Softer gum rubber, useful lower-gloss boundary. |
| V06_toy_vinyl_gloss | 0.18 | 0.96 | 0.62 | 0.08 | 0.04 | 1.4 | 1.1 | -0.34 | 0.04 | 0.001 | Upper gloss boundary; may read more toy vinyl than rubber. |

## Recommended First Review Candidates

- `V02_rubber_pop`: balanced target.
- `V03_vinyl_bounce`: stronger broad highlights if the model still reads too matte.
- `V04_candy_rubber`: softest/bounciest static read.
- `V06_toy_vinyl_gloss`: upper gloss boundary; useful if Pablo wants more toy/vinyl.

## UE5 Port

This is a portable parameter mapping, not a direct Blender node-graph transfer. UE5 subsurface, clear coat, and sheen/fresnel reads will need final tuning under the matching UE flat rig because they are not 1:1 with Blender.

- Metallic: `0`.
- Roughness: use the chosen variation value directly.
- Specular: approximate Blender `Specular IOR Level` as UE `Specular` in the same 0-1 range, then tune under the UE flat rig.
- Clear Coat: use Blender `Coat Weight`; Clear Coat Roughness: use Blender `Coat Roughness`.
- Subsurface: use a Subsurface Profile or Subsurface shading model for the soft body/skin-like pieces; start from `Subsurface Weight` and the radius triplet as color-channel guidance.
- Texture flattening: reduce contrast/noise in the base texture before Base Color; do not bake specular highlights into the texture.
- Edge softness: bevelled geometry or weighted normals should carry the rubber edge catch.

## Rendered Review Artifacts

- Comparison grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_rubber_comparison_grid.png`
- App preview grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_rubber_comparison_grid_preview_1300.png`
- Turntable `V02_rubber_pop`: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V02_rubber_pop_turntable.mp4`
- Turntable `V03_vinyl_bounce`: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V03_vinyl_bounce_turntable.mp4`
- Turntable `V04_candy_rubber`: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V04_candy_rubber_turntable.mp4`
- Turntable `V06_toy_vinyl_gloss`: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V06_toy_vinyl_gloss_turntable.mp4`
