# Pixal3D Agents

## Owns

The production-cleared Pixal3D model-generation pipeline and active FriendSlop raw import workflow.

## Trigger Words

Pixal3D, TencentARC, CuMesh, Pixal3D RunPod, Pixal3D smoke, FriendSlop raw, `run_pixal3d_batch.py`, `run_pixal3d_smoke.py`.

## Read First

- `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` for active Pixal3D assets entering playable FriendSlop content.
- `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
- `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`
- `Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md`

## Hard Rules

- Pixal3D is production-cleared for T66 replacement assets, but remains technically separate from TRELLIS. Do not debug Pixal3D failures by editing TRELLIS scripts.
- Active production assets must use the FriendSlop raw import path, 200k face target unless the owning manifest accepts another target, explicit generated-texture binding, Unreal importer, and hard validators.
- Do not route active FriendSlop assets through archived ToonStyle tint, close-the-gap, inner-line, or QuadRetro validation unless the user explicitly revives that historical path.
- For multi-model runs, prefer the detached batch runner over one long foreground SSH loop.

## Verification

Verify pod reachability first, then service health, nonzero GLBs, JSONL/DONE logs, export headers/settings, Blender QA/FriendSlop manifests, Unreal verify JSON, hard validator report, material/texture binding proof, and staged standalone evidence when playable content changes.
