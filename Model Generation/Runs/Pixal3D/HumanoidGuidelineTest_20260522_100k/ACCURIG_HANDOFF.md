# Humanoid Guideline 100k AccuRig Handoff

Run root:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HumanoidGuidelineTest_20260522_100k`

## Scope

Generated 18 humanoid Pixal3D source models for AccuRig:

- Hero 1-5 Chad demo outfit
- Hero 1-5 Stacy demo outfit
- 4 companion identity slots in regular outfit
- 4 companion identity slots in demo outfit

This pass is Pixal3D source generation plus textured AccuRig FBX/OBJ handoff only.
No Unreal runtime import, gameplay reference update, or staged standalone refresh was
performed.

## Settings

The run follows the Pixal3D ToonStyle production flow except for the explicit
user-requested face target override:

- `X-Seed: 1337`
- `X-Resolution: 1536`
- `X-Image-Resolution: 1024`
- `X-Texture-Size: 4096`
- `X-Decimation: 100000`
- `X-Remesh: 1`
- `X-Export-Fallback: 0`
- `X-Fallback-Decimation: 80000`
- `X-Safe-Fill-Holes-Fallback: 0`
- `X-SS-Steps: 25`
- `X-Shape-Steps: 25`
- `X-Tex-Steps: 25`
- `X-Tex-Guidance: 4.0`

The standard Pixal3D production target is `200000`; this run uses `100000`
because Pablo explicitly requested it for this batch.

## Main Outputs

Raw Pixal3D GLBs:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HumanoidGuidelineTest_20260522_100k\Outputs`

Textured AccuRig FBX/OBJ bundles:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HumanoidGuidelineTest_20260522_100k\AccuRig_Textured`

Each asset directory contains:

- `<Asset>_Textured.fbx`
- `<Asset>_Textured.obj`
- `<Asset>_Textured.mtl`
- `Textures\*.png`
- `<Asset>_accurig_textured_export.json`

## Verification

Generation verification:

`Reports\pixal3d_100k_generation_verification.json`

Result:

- 18/18 GLBs downloaded locally
- 18/18 HTTP 200 generation rows
- 18/18 `X-Pixal3D-Export-Label: requested`
- 18/18 `X-Pixal3D-Export-Decimation: 100000`
- 18/18 `X-Pixal3D-Export-Remesh: 1`
- 0 fallback exports
- 0 verification errors

AccuRig export verification:

`AccuRig_Textured\AccuRig_Textured_Export_Report.json`

Result:

- 18/18 textured FBX exports
- 18/18 OBJ/MTL exports
- 18/18 texture bundles
- 0 export validation errors

Pod health proof:

`pod_health.json`

Source-gate report:

`Reports\source_gate_report.json`

The strict source luminance gate flags the source PNGs as hard rejects. This was
recorded and waived for this user-requested model-generation pass because Pablo
explicitly requested generation from the accepted humanoid guideline-test image
batch. Source PNGs were not modified.

