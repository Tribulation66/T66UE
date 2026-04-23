# Model Generation Master Workflow

This is the authoritative internal workflow for `T66` 3D model generation as of `2026-04-19`.

Use this document as the source of truth.

If you need the workspace map first, start with [README.md](C:/UE/T66/Model%20Generation/README.md).

The JSX setup file in this folder is historical reference only:

- [TRELLIS2_RunPod_Setup_Guide.jsx](C:/UE/T66/Model%20Generation/Reference/TRELLIS2_RunPod_Setup_Guide.jsx)

Supporting operational files:

- [RUN_HISTORY.md](C:/UE/T66/Model%20Generation/RUN_HISTORY.md)
- [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
- [KNOWN_ISSUES.md](C:/UE/T66/Model%20Generation/KNOWN_ISSUES.md)
- [NEXT_STEPS.md](C:/UE/T66/Model%20Generation/NEXT_STEPS.md)
- [CURRENT_HANDOFF_PROMPT.md](C:/UE/T66/Model%20Generation/CURRENT_HANDOFF_PROMPT.md)
- [MESH_APPROVAL_CHECKLIST.md](C:/UE/T66/Model%20Generation/MESH_APPROVAL_CHECKLIST.md)
- [SECOND_ATTEMPT_PROMPT.md](C:/UE/T66/Model%20Generation/SECOND_ATTEMPT_PROMPT.md)
- [bootstrap_trellis2_pod.sh](C:/UE/T66/Model%20Generation/Scripts/bootstrap_trellis2_pod.sh)

## Purpose

- Generate reference images for temporary benchmark characters.
- Run `TRELLIS.2` on RunPod to produce raw `.glb` meshes.
- Inspect results in Blender through Blender MCP.
- Keep all inputs, outputs, QA renders, and setup notes in one place.
- Preserve a repeatable baseline before retopo and Unreal import work.

## Folder Layout

```text
C:\UE\T66\Model Generation\
  README.md
  MASTER_WORKFLOW.md
  Tools\
    Trellis2\
      start_blender_mcp.py
      trellis_server.py
  Reference\
    TRELLIS2_RunPod_Setup_Guide.jsx
  Runs\
    Arthur\
      Inputs\
      Raw\
      Renders\
    Enemies\
      Easy\
        <Family>\
          Inputs\
          Raw\
          Renders\
          Retopo\
```

Current Arthur assets already copied here:

- Inputs: [Inputs](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs)
- Raw GLBs: [Raw](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw)
- QA renders: [Renders](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders)

## Current Baseline Decisions

- Use `TRELLIS.2`, not TRELLIS v1.
- Use RunPod for generation, not the local laptop GPU.
- For baseline parity with the older JSX workflow, use `opaque PNGs with green background`, not alpha cutouts.
- Leave `preprocess_image=True` so TRELLIS runs its own background removal path.
- Keep `rembg` override and the `cfg_strength` patch.
- Do not use the extra DINO compatibility patch unless the environment drifts away from the JSX version.
- Weapons are separate assets and should be attached after the character mesh is acceptable.
- Retopo starts only after a raw mesh is visually approved.
- For small enemy-family batches, one clean front-view green reference plus a strong seed is enough to test the family quickly before spending time on rerolls.
- Equipment placement is not approved from a single screenshot. For swords or other handheld props, require front, side, oblique, and user-like Blender checks, plus explicit user approval, before calling the placement done.
- Saved PNGs and file links are the source of truth for review. Inline chat image previews are helpful when they work, but they are not reliable enough to be the only review surface.

## What Matches The JSX Baseline

The active baseline now matches the original JSX setup in the ways that matter:

- `transformers==5.2.0`
- original `image_feature_extractor.py`
- `BiRefNet.py` replaced with a `rembg` wrapper
- `flow_euler.py` patched to drop leaked `cfg_strength`
- `trellis_server.py` behavior equivalent to the JSX server
- same request defaults:
  - `X-Decimation: 200000`
  - `X-Texture-Size: 2048`
  - `20` steps per stage
  - texture guidance `2.0`

## Pod State

Current pod assumptions:

- Repo path: `/workspace/TRELLIS.2`
- Server path: `/workspace/TRELLIS.2/trellis_server.py`
- Server log: `/workspace/TRELLIS.2/trellis_server.log`
- Model: `microsoft/TRELLIS.2-4B`

Recommended RunPod template:

```text
runpod/pytorch:2.4.0-py3.11-cuda12.4.1-devel-ubuntu22.04
```

Expose:

- `8000`
- `8888`
- SSH

## Required Pod Fixes

### 1. Background Removal Override

Install:

```bash
pip install flask onnxruntime rembg
```

Override:

```text
/workspace/TRELLIS.2/trellis2/pipelines/rembg/BiRefNet.py
```

with:

```python
from typing import *
from PIL import Image
from rembg import remove


class BiRefNet:
    def __init__(self, model_name: str = "ZhengPeng7/BiRefNet"):
        pass

    def to(self, device: str):
        return self

    def cuda(self):
        return self

    def cpu(self):
        return self

    def __call__(self, image: Image.Image) -> Image.Image:
        return remove(image)
```

### 2. cfg_strength Fix

Patch:

```text
/workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py
```

with:

```bash
sed -i 's/return model(x_t, t, cond, \*\*kwargs)/kwargs.pop("cfg_strength", None); return model(x_t, t, cond, **kwargs)/' /workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py
```

## Environment Version Lock

The baseline that worked is:

```bash
pip install --no-cache-dir "transformers==5.2.0"
```

Do not casually upgrade `transformers` on the pod.

If `transformers` drifts forward again, TRELLIS may fail with DINO-related errors and tempt a compatibility patch. That patch is not part of the canonical baseline.

## Canonical Local Files

- Server template: [trellis_server.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/trellis_server.py)
- Blender MCP helper: [start_blender_mcp.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/start_blender_mcp.py)
- Blender QA / decimate helper: [blender_glb_qa.py](C:/UE/T66/Model%20Generation/Scripts/blender_glb_qa.py)

## Blender Tool Roles

- Blender MCP is the canonical agent automation layer for import, export, scene inspection, scripted edits, screenshots, and repeatable QA.
- RetopoFlow is the manual retopology tool for assets that need cleaner topology than decimation can provide, especially hero meshes and deformation-critical regions.
- Blender Buddy, if installed locally, should be treated as an interactive in-Blender copilot for human-driven exploration and tool help, not as the authoritative workflow controller.

## Blender Session Rule

- Use live Blender through Blender MCP when it is reachable.
- If Blender MCP is disconnected or stale, reopen Blender and rerun [start_blender_mcp.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/start_blender_mcp.py) before falling back to headless tooling.
- Headless Blender is the recovery path for QA render, decimate, export, and re-import verification when the live MCP session is unavailable.
- Before Unreal import, re-import any accepted decimated GLB in Blender and render one verification image from the exported file itself.

## Retopo Rule

- Start with decimate-plus-verify when the asset class has already proven that path is visually acceptable.
- Escalate to RetopoFlow when decimation damages silhouette, hand readability, deformation zones, or future rigging viability.
- For any manual retopo pass, keep the working `.blend`, exported result, and a fresh verification render together with the rest of the model-generation artifacts.

## Starting The Server

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
export NVCC_PREPEND_FLAGS=
export CUDA_HOME=/usr/local/cuda
export PATH=/usr/local/cuda/bin:/usr/bin:/bin
export PYTHONPATH=/workspace/TRELLIS.2
cd /workspace/TRELLIS.2
nohup python -u /workspace/TRELLIS.2/trellis_server.py > /workspace/TRELLIS.2/trellis_server.log 2>&1 < /dev/null &
```

Health check:

```bash
curl -s http://127.0.0.1:8000/health
```

Expected:

```json
{"status":"ok", "...":"..."}
```

## Canonical Generation Method

For reliability, use:

1. `scp` input PNGs to the pod
2. run `curl` on the pod against `http://127.0.0.1:8000/generate`
3. `scp` the `.glb` back

This transport method is more stable than an SSH tunnel and does not change output quality.

### Example Upload

```powershell
scp.exe -P <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 "C:\UE\T66\Model Generation\Runs\Arthur\Inputs\Arthur_MegabonkDirect_Body_Green.png" root@<POD_IP>:/tmp/Arthur_MegabonkDirect_Body_Green.png
```

### Example Generate

```bash
curl --fail --silent --show-error --max-time 3600 \
  -X POST \
  -H 'X-Seed: 1440' \
  -H 'X-Texture-Size: 2048' \
  -H 'X-Decimation: 200000' \
  --data-binary @/tmp/Arthur_MegabonkDirect_Body_Green.png \
  http://127.0.0.1:8000/generate \
  -o /tmp/Arthur_MegabonkDirect_Body_Green_Trellis2.glb
```

### Example Download

```powershell
scp.exe -P <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 root@<POD_IP>:/tmp/Arthur_MegabonkDirect_Body_Green_Trellis2.glb "C:\UE\T66\Model Generation\Runs\Arthur\Raw\Arthur_MegabonkDirect_Body_Green_Trellis2.glb"
```

## Input Rules

For baseline reproduction:

- Use the real in-game screenshot look, not a stylized redraw, whenever possible.
- Use `opaque PNG` input.
- Use a solid green background.
- Keep the figure centered and readable.
- Avoid alpha inputs for baseline comparison.

Current Arthur body/head baseline inputs:

- [Arthur_MegabonkDirect_Body_Green.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_MegabonkDirect_Body_Green.png)
- [Arthur_MegabonkDirect_Head_Green.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_MegabonkDirect_Head_Green.png)

Current hero-reference inputs:

- [Arthur_HeroReference_Full_Green.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_HeroReference_Full_Green.png)
- [Arthur_HeroReference_Full_White.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_HeroReference_Full_White.png)

## Current Hero Status

### Successful Artifacts

Best current full-body raw / low-poly / equipment artifacts:

- [best raw full body](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Trellis2.glb)
- [best low-poly body](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k.glb)
- [clean sword raw](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_ExcaliburProxy_FlatGreen_Tight_S1337_Trellis2.glb)
- [latest sword-placement prototype](C:/UE/T66/Model%20Generation/Runs/Arthur/Raw/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold.glb)

Latest QA renders:

- [best raw full body render](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_HeroReference_Full_White_S1337_D80000_Raw.png)
- [best low-poly body render](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k.png)
- [latest sword front check](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold_Front.png)
- [latest sword side check](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold_Right.png)
- [latest sword oblique check](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_HeroReference_Full_White_S1337_D80000_Decimate40k_WithSword_ValidatedHold_Oblique.png)

### Known Problems

- A separate head pass is not the current priority.
- The body is strong enough to carry equipment and lineup work.
- The sword hold is still not user-approved. `A22` improved the overlap and direction, but the result still does not read convincingly enough as a true held weapon.
- Arthur's current idle hand mesh is still too open for a truly tight grip, so transform-only placement may not be enough.
- Split body/head generation can still produce bad seams or unusable geometry, so it remains a fallback path rather than the default.

Earlier test assets remain useful for comparison:

- [Arthur_MegabonkProxy_Body_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_MegabonkProxy_Body_Raw.png)
- [Arthur_MegabonkProxy_Head_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_MegabonkProxy_Head_Raw.png)
- [Arthur_MegabonkDirect_Body_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_MegabonkDirect_Body_Raw.png)
- [Arthur_MegabonkDirect_Head_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_MegabonkDirect_Head_Raw.png)

## Equipment Placement Approval Loop

For handheld props such as swords:

1. Start from the accepted character body and the accepted prop raw.
2. Try transform-only placement first.
3. Save at least these review PNGs:
   - front
   - side
   - oblique
   - one user-like angle close to the intended gameplay or showcase read
4. Reject the pose immediately if any view shows:
   - the grip clearly outside the hand volume
   - the sword reading as placed beside the hand instead of through it
   - the blade axis pointing in the wrong directional family
5. Do not call the pose solved until the user also agrees that it reads as held.
6. If transform-only placement keeps failing, escalate to a minimal hand-pose edit, socket rule, or lightweight rig step.

## Current Easy Enemy Status

Accepted easy-family source candidates:

- `Cow / Melee` -> `SkeletonWarrior`
  - [raw GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/SkeletonWarrior/Raw/SkeletonWarrior_Front_Green_S1337_D200000_Trellis2.glb)
  - [accepted low-poly GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/SkeletonWarrior/Retopo/SkeletonWarrior_Front_Green_S1337_D200000_Retopo24k.glb)
- `Pig / Rush` -> `GoblinCharger`
  - [raw GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/GoblinCharger/Raw/GoblinCharger_Front_Green_S1337_D200000_Trellis2.glb)
  - [accepted low-poly GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/GoblinCharger/Retopo/GoblinCharger_Front_Green_S1337_D200000_Retopo24k.glb)
- `Goat / Ranged` -> `SlimeSpitter`
  - [raw GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/SlimeSpitter/Raw/SlimeSpitter_Front_Green_S1337_D200000_Trellis2.glb)
  - [accepted low-poly GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/SlimeSpitter/Retopo/SlimeSpitter_Front_Green_S1337_D200000_Retopo10k.glb)
- `Roost / Flying` -> `CaveBat`
  - [raw GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/CaveBat/Raw/CaveBat_Front_Green_S1337_D200000_Trellis2.glb)
  - [accepted low-poly GLB](C:/UE/T66/Model%20Generation/Runs/Enemies/Easy/CaveBat/Retopo/CaveBat_Front_Green_S1337_D200000_Retopo14k.glb)

## Recommended Next Experiments

Run these in order:

1. Keep the full-body hero path as primary unless a specific face issue forces a head-only experiment.
2. Fix the sword hold so it reads as truly held from front, side, oblique, and user-like review angles.
3. If transform-only sword placement still fails, move into a light hand-pose or rig step instead of endless transform nudging.
4. After the sword hold is solved, explore rigging and baked-mesh workflows for Arthur and the easy enemy batch.
5. Extend the same `reference -> TRELLIS -> decimate -> verify` workflow to the next difficulty-family batch only after the local character pipeline is stable.
6. Only move into Unreal import once the exported low-poly GLB has a verification render from a fresh Blender re-import and the intended rig/bake direction is decided.

## Retopo Policy

Retopo is not the current bottleneck.

Do not begin retopo until:

- the raw mesh is visually acceptable
- the silhouette is stable
- there is no major hallucinated geometry

Current retopo tool:

- `RetopoFlow 4.1.5` in Blender

Current accepted enemy-family targets from the easy batch:

- biped melee / rush families: about `24k` tris
- slime-like blob families: about `10k` tris
- wide flying families: about `14k` tris

## Practical Notes

- The canonical Flask server listens on port `8000`, not `5000`.
- Parallel generation requests can look like a hang because the Flask server is single-threaded and processes them serially.
- Long console output is often `xatlas` and mesh postprocess, not a crash.
- If the local command is interrupted, the pod job may still finish successfully.
- Always check the pod log before assuming a run failed.
- Blender can emit glTF export warnings after heavy decimation. Treat the warning as a verification trigger, not an automatic rejection.
- In Codex chat, local inline image rendering can fail or lag. Save the review PNGs to disk anyway and use file links or the live Blender scene as the reliable review path.

## Authority Rule

When this folder and the old JSX guide disagree:

- trust this file first
- use the JSX guide only as a historical reference
- update this file when the process changes
