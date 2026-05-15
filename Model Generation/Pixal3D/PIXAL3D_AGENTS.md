# Pixal3D Agents

## Owns

The separate Pixal3D research pipeline only.

## Trigger Words

Pixal3D, TencentARC, CuMesh, Pixal3D RunPod, Pixal3D smoke, `run_pixal3d_batch.py`, `run_pixal3d_smoke.py`.

## Read First

- `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
- `Model Generation/Instructions/07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`
- `Model Generation/Instructions/08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md`

## Hard Rules

- Pixal3D is research-only until licensing changes or legal approval is explicit.
- Do not replace TRELLIS globally.
- Do not import Pixal3D assets into shipped Unreal content unless explicitly approved.
- Do not debug Pixal3D failures by editing TRELLIS scripts.
- For multi-model runs, prefer the detached batch runner over one long foreground SSH loop.

## Verification

Verify pod reachability first, then service health, nonzero GLBs, JSONL/DONE logs, export headers/settings, and Blender import counts.
