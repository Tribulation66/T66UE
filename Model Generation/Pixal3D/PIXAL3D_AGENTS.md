# Pixal3D Agents

## Owns

The production-cleared Pixal3D model-generation pipeline and ToonStyle production replacement workflow.

## Trigger Words

Pixal3D, TencentARC, CuMesh, Pixal3D RunPod, Pixal3D smoke, `run_pixal3d_batch.py`, `run_pixal3d_smoke.py`.

## Read First

- `Model Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md` for any Pixal3D asset entering playable content.
- `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
- `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`
- `Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md`

## Hard Rules

- Pixal3D is production-cleared for T66 replacement assets, but remains technically separate from TRELLIS. Do not debug Pixal3D failures by editing TRELLIS scripts.
- Production assets must use the manifest-driven ToonStyle production import wrapper, 200k face target, foundation Blender pipeline, Unreal importer, and hard validators.
- Do not manually assign ToonStyle materials or bypass Tint, close-the-gap, and inner-line validation.
- For multi-model runs, prefer the detached batch runner over one long foreground SSH loop.

## Verification

Verify pod reachability first, then service health, nonzero GLBs, JSONL/DONE logs, export headers/settings, Blender/ToonStyle manifests, Unreal verify JSON, hard validator report, and staged standalone evidence when playable content changes.
