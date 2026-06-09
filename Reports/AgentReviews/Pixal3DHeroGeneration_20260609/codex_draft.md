# Pixal3D Hero Generation Evidence

## Task

Update the Pixal3D/model-generation source rules to remove the luminance and white-background blockers, then generate Pixal3D models from:

- `C:\UE\T66\SourceAssets\Hero1Stacy.png`
- `C:\UE\T66\SourceAssets\Hero2Chad.png`

Use the images as-is, create a side-by-side Blender scene, and open Blender.

## Process Docs Updated

Edited:

- `Model Generation/Instructions/02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`
- `Model Generation/Instructions/10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md`

Current search result over those two docs:

- no `luminance`
- no `Brightness Target`
- no `brightness bar`
- no `bright enough`
- no `clean white background`
- no `Pure white background`
- no `white canvas`
- no `sample source mean`
- no `white background`

The remaining source gate is readability/composition/color discipline: one clear subject, full body/front-readable silhouette for character sources, clear subject/background separation, no UI/text/contact-sheet labels, and other remaining sanity checks.

## Pixal3D Batch

Run root:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HeroChadStacy_SourceAssets_20260609_0536`

Command:

```text
python "Model Generation/Pixal3D/Scripts/run_pixal3d_batch.py" run --pod-ip 69.30.85.73 --pod-port 22079 --server-port 18001 --local-run-root "Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536" --wait-timeout 7200 --generate-timeout 1800 --poll-interval 20
```

Settings used by script defaults:

- seed `1337`
- resolution `1536`
- texture size `4096`
- decimation `200000`
- remesh enabled
- export fallback disabled
- safe fill holes fallback disabled

DONE:

```json
{
  "exit_code": 0,
  "ok": true
}
```

Outputs:

- `Outputs/Hero1Stacy.glb`, `10052292` bytes
- `Outputs/Hero2Chad.glb`, `10398104` bytes

Status rows:

- `Hero1Stacy`: `ok=true`, `status=200`, `bytes=10052292`, `X-Pixal3D-Export-Decimation=200000`, `X-Pixal3D-Export-Remesh=1`, `X-Pixal3D-Export-Attempts=1`, `X-Pixal3D-Export-Safe-Fill-Holes=0`
- `Hero2Chad`: `ok=true`, `status=200`, `bytes=10398104`, `X-Pixal3D-Export-Decimation=200000`, `X-Pixal3D-Export-Remesh=1`, `X-Pixal3D-Export-Attempts=1`, `X-Pixal3D-Export-Safe-Fill-Holes=0`

## Blender Evidence

Blender QA imported both GLBs and rendered front views with yaw `180`.

- `QA/Hero1Stacy_front_yaw180.png`
- `QA/Hero1Stacy_front_yaw180_metadata.json`
- `QA/Hero2Chad_front_yaw180.png`
- `QA/Hero2Chad_front_yaw180_metadata.json`

Triangle counts from metadata:

- `Hero1Stacy`: `199573`
- `Hero2Chad`: `187951`

Side-by-side Blender scene:

- `Blender/HeroChadStacy_side_by_side.blend`, `23232932` bytes
- `QA/HeroChadStacy_side_by_side.png`, `2274945` bytes

Blender was opened visibly with the saved scene. Process evidence:

```text
blender  HeroChadStacy_side_by_side
```

## Scope Notes

- No Unreal import was run.
- No DataTable reload was run.
- No staged standalone build was run.
