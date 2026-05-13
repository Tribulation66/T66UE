# Pixal3D Pipeline

This folder owns the experimental Pixal3D RunPod path for T66 model generation.
It is separate from the existing TRELLIS.2 pipeline so the current generator can
stay intact while Pixal3D is evaluated.

## Status

Pixal3D is technically useful, but its current license is a blocker for shipped
T66 production assets. The upstream license says Pixal3D is academic-only, not
for commercial or production use, and not intended for use within the European
Union. Keep generated output marked as research until that changes or legal
approval is explicit.

## Sources Checked

Checked on 2026-05-12:

- Project page: https://ldyang694.github.io/projects/pixal3d/
- GitHub repo: https://github.com/TencentARC/Pixal3D
- Hugging Face model: https://huggingface.co/TencentARC/Pixal3D
- Hugging Face demo space: https://huggingface.co/spaces/TencentARC/Pixal3D

## Folder Layout

- `Server/pixal3d_server.py`: Flask server that mirrors the current TRELLIS
  `/health` and `/generate` contract while using Pixal3D.
- `Scripts/bootstrap_pixal3d_pod.sh`: RunPod bootstrap script for a separate
  `pixal3d` conda environment.
- `Scripts/Invoke-Pixal3DHfLogin.ps1`: optional Hugging Face login helper using
  the repo-local `Model Generation/LOCAL_ACCESS.env` convention.
- `Scripts/run_pixal3d_smoke.py`: local smoke runner that creates source plates,
  uploads them to RunPod, calls Pixal3D, downloads GLBs, and can run Blender QA
  plus Quad Retro post-processing.
- `Scripts/run_pixal3d_batch.py`: reusable detached batch runner for arbitrary
  experiment source folders. It launches generation on the pod with `nohup`,
  writes JSONL status plus a `DONE` sentinel, polls with short SSH calls, and
  downloads GLBs/logs after completion.
- `../Instructions/07_PIXAL3D_RUNPOD_SETUP.md`: step-by-step RunPod setup and
  smoke-test runbook.
- `../Instructions/08_PIXAL3D_TROUBLESHOOTING.md`: CuMesh/export/remesh failure
  table and recovery rules.

Generated run output should live under `Model Generation/Runs/Pixal3D/...` and
should be deleted or summarized after the evaluation is complete.

## Existing T66 TRELLIS Baseline

The current T66 flow is:

1. Make or select a clean source image.
2. Send it to a RunPod TRELLIS.2 server through `/generate`.
3. Save raw GLB output under batch-specific raw-output folders.
4. Run Blender QA renders.
5. For characters or retro assets, run Quad Retro and optionally Quad Remesher.
6. Import into Unreal only after visual review and validation.

The current TRELLIS server accepts:

- `X-Seed`
- `X-Texture-Size`
- `X-Decimation`

Existing batch scripts usually use seed `1337`, texture size `2048`, and
decimation `80000` for T66 batches.

## Pixal3D Runtime Contract

The Pixal3D server intentionally keeps the same basic shape:

- `GET /health` returns status, GPU, VRAM, model path, and license warning.
- `POST /generate` accepts raw image bytes and returns a GLB.

Supported headers:

- `X-Seed`
- `X-Texture-Size`
- `X-Decimation`
- `X-Remesh`: `1` by default; use `0` to skip Pixal3D export remeshing when
  flat or modular inputs hit CuMesh UV atlas failures.
- `X-Export-Fallback`: `1` by default. The T66 server exports GLBs in a child
  process and retries safer export settings if CuMesh fails.
- `X-Fallback-Decimation`: default `30000`. Used by the server and smoke runner
  when the requested export setting fails.
- `X-Resolution`: `1024` or `1536`
- `X-SS-Guidance`
- `X-SS-Steps`
- `X-Shape-Guidance`
- `X-Shape-Steps`
- `X-Tex-Guidance`
- `X-Tex-Steps`
- `X-Max-Num-Tokens`

Output namespaces should use Pixal3D-specific names:

- `Raw/Pixal3D/...`
- `QA/Pixal3DFront/...`
- manifest fields such as `raw_pixal3d_glb`

The downstream Blender QA and Quad Retro scripts already accept arbitrary GLB
paths. The main integration work is naming and manifest plumbing, not a new
post-processing backend.

## Current Smoke Result

Validated on 2026-05-12 against an A40 RunPod:

- Server health passed on local pod port `18001` after eager-loading Pixal3D.
- Hugging Face auth was required for `briaai/RMBG-2.0`; use
  `Scripts/Invoke-Pixal3DHfLogin.ps1` with `Model Generation/LOCAL_ACCESS.env`.
- A40 generation required rebuilding `natten==0.21.0` for CUDA arch `8.6`.
- `tree_organic_prop` generated with default remesh and passed Blender QA.
- `stone_wall_module` hit a CuMesh UV atlas illegal-memory-access failure with
  default remesh, then generated with `X-Remesh: 0` and passed Blender QA.
- `humanoid_character` generated with default remesh, passed Blender QA, and
  passed Quad Retro.
- `horned_monster` hit the same CuMesh remesh failure with default remesh, then
  generated with `X-Remesh: 0`, passed Blender QA, and passed Quad Retro.

The consolidated report for that run is:

`Model Generation/Runs/Pixal3D/PipelineSmoke01/Reports/Pixal3D_PipelineSmoke01.json`

Earlier smoke tests showed CuMesh failures during Pixal3D GLB export, especially
around UV atlas creation and non-manifold cleanup. These were not image-to-shape
failures. The risky part was running `o_voxel.postprocess.to_glb(...)` inside
the same long-lived Flask process that holds the Pixal3D models and prior CUDA
state. A CUDA illegal memory access there could kill the whole request and leave
the client with an empty reply.

The T66 server now serializes the generated mesh tensors, frees transient GPU
state, and runs GLB export in a short-lived worker process. If the worker fails,
the server can retry the same generated mesh with `X-Fallback-Decimation`
(`30000` by default) and then with remesh disabled. The smoke runner also records
per-attempt export settings and can restart the server before a client-side
fallback if an older server still dies.

With the worker-isolated exporter, `stone_wall_module` and the retained
Experiment 1 `Variant_B` source both generated successfully at the formerly
risky `X-Decimation: 80000`, `X-Remesh: 1` setting on the A40 pod. Keep
`X-Remesh: 1` for normal Pixal3D research runs, and leave fallback enabled.

## RunPod Bootstrap

Use `../Instructions/07_PIXAL3D_RUNPOD_SETUP.md` for the step-by-step pod setup,
HF auth path, startup command, default generation settings, and smoke-test
verification. Use `../Instructions/08_PIXAL3D_TROUBLESHOOTING.md` before
changing server code or generation settings for remesh/export failures.

## Decimation Policy

Pixal3D uses `o_voxel.postprocess.to_glb(... decimation_target=...)`, so its
decimation control is export-time mesh reduction. It is useful for prototypes
and static props.

Do not treat Pixal3D export decimation as production topology for deformation
critical characters. Keep Quad Retro or human-authored topology for characters
that need animation or deformation.

## Research Test Matrix

Use a small but broad test set before larger batches:

- tree or organic prop
- wall or modular environment piece
- humanoid character
- monster or non-human creature

Recommended first pass:

- resolution `1024`
- seed `1337`
- texture size `2048`
- decimation `80000`
- remesh enabled
- export fallback enabled with fallback decimation `30000`

Optional stress pass:

- resolution `1536`
- decimation `200000` and `1000000`

For multi-model experiments, use `Scripts/run_pixal3d_batch.py` instead of a
foreground SSH loop. A foreground loop can finish the GLBs remotely while the
local SSH session remains stuck waiting on a shell pipe or transport state. The
batch runner avoids that by using status files and a final sentinel on the pod.

For every generated GLB, run Blender QA and capture triangle count, bounds, file
size, and front render. For character-like samples, also run the Quad Retro
wrapper to prove the existing post-processing path can consume the output.

Do not import into Unreal or stage standalone from Pixal3D output until the
license issue is resolved.
