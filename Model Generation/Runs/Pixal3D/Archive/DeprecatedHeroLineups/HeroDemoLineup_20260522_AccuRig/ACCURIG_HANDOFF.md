# Hero Demo Lineup AccuRig Handoff

Archived status: this is historical AccuRig / Animated ToonStyle provenance only. Do not use it as a current FriendSlop model source.

Run root:

`C:\UE\T66\Model Generation\Runs\Pixal3D\Archive\DeprecatedHeroLineups\HeroDemoLineup_20260522_AccuRig`

Primary textured AccuRig inputs:

`C:\UE\T66\Model Generation\Runs\Pixal3D\Archive\DeprecatedHeroLineups\HeroDemoLineup_20260522_AccuRig\AccuRig_Textured`

The earlier `AccuRig_FBX` folder contains ToonStyle-import FBXs with placeholder
gray materials. Do not use those for AccuRig texture review.

## Assets

| Asset | Textured FBX | OBJ fallback |
| --- | --- | --- |
| Hero 1 Chad | `AccuRig_Textured/Hero_1_Chad/Hero_1_Chad_Textured.fbx` | `AccuRig_Textured/Hero_1_Chad/Hero_1_Chad_Textured.obj` |
| Hero 1 Stacy | `AccuRig_Textured/Hero_1_Stacy/Hero_1_Stacy_Textured.fbx` | `AccuRig_Textured/Hero_1_Stacy/Hero_1_Stacy_Textured.obj` |
| Hero 2 Chad | `AccuRig_Textured/Hero_2_Chad/Hero_2_Chad_Textured.fbx` | `AccuRig_Textured/Hero_2_Chad/Hero_2_Chad_Textured.obj` |
| Hero 2 Stacy | `AccuRig_Textured/Hero_2_Stacy/Hero_2_Stacy_Textured.fbx` | `AccuRig_Textured/Hero_2_Stacy/Hero_2_Stacy_Textured.obj` |
| Hero 3 Chad | `AccuRig_Textured/Hero_3_Chad/Hero_3_Chad_Textured.fbx` | `AccuRig_Textured/Hero_3_Chad/Hero_3_Chad_Textured.obj` |
| Hero 3 Stacy | `AccuRig_Textured/Hero_3_Stacy/Hero_3_Stacy_Textured.fbx` | `AccuRig_Textured/Hero_3_Stacy/Hero_3_Stacy_Textured.obj` |
| Hero 4 Chad | `AccuRig_Textured/Hero_4_Chad/Hero_4_Chad_Textured.fbx` | `AccuRig_Textured/Hero_4_Chad/Hero_4_Chad_Textured.obj` |
| Hero 4 Stacy | `AccuRig_Textured/Hero_4_Stacy/Hero_4_Stacy_Textured.fbx` | `AccuRig_Textured/Hero_4_Stacy/Hero_4_Stacy_Textured.obj` |
| Hero 5 Chad | `AccuRig_Textured/Hero_5_Chad/Hero_5_Chad_Textured.fbx` | `AccuRig_Textured/Hero_5_Chad/Hero_5_Chad_Textured.obj` |
| Hero 5 Stacy | `AccuRig_Textured/Hero_5_Stacy/Hero_5_Stacy_Textured.fbx` | `AccuRig_Textured/Hero_5_Stacy/Hero_5_Stacy_Textured.obj` |

Each asset folder also contains `Textures/*.png` and an `.mtl` file. Keep the
whole folder together if using the OBJ fallback.

## Generation Settings

- Pixal3D pod: RTX 6000 Ada, health status `ok`
- Resolution: `1536`
- Texture size: `4096`
- Decimation: `200000`
- Remesh: `1`
- Export fallback: `0`
- Safe fill-holes fallback: `0`
- Seed: `1337`

## Verification

- Pixal3D generated all 10 GLBs with HTTP `200`.
- All response headers reported requested export: label `requested`, decimation `200000`, remesh `1`, safe fill holes `0`.
- Blender ToonStyle processing produced all 10 main ToonStyle FBX files and all 10 foundation manifests.
- AccuRig textured export produced all 10 textured FBX files, all 10 OBJ/MTL fallbacks, and texture PNG bundles.
- Verification summary: `AccuRig_FBX_Verification_Report.json`.
- Textured export summary: `AccuRig_Textured/AccuRig_Textured_Export_Report.json`.
- Raw GLBs: `Outputs/`.
- Source-gate report: `source_gate_report.json`.

## Source-Gate Note

The source PNGs were preserved unchanged. The strict linear-luminance source gate flags all selected hero PNGs as hard rejects, but this batch was generated under the user-requested pod-generation override for AccuRig evaluation.

## Promotion Boundary

These FBXs are manual AccuRig source exports only. They were not imported into Unreal, no gameplay references were changed, and no standalone build was refreshed.
