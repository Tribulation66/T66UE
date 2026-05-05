# Model Generation Master Workflow

This is the authoritative internal workflow for `T66` 3D model generation as of `2026-04-19`.

Use this document as the source of truth.

If you need the workspace map first, start with [README.md](C:/UE/T66/Model%20Generation/README.md).

For character work after raw TRELLIS output exists, read [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md) before Blender assembly, rigging, Unreal import, DataTable wiring, or staged visual verification.

The JSX setup file in this folder is historical reference only:

- [TRELLIS2_RunPod_Setup_Guide.jsx](C:/UE/T66/Model%20Generation/Reference/TRELLIS2_RunPod_Setup_Guide.jsx)

Supporting operational files:

- [RUN_HISTORY.md](C:/UE/T66/Model%20Generation/RUN_HISTORY.md)
- [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md)
- [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
- [KNOWN_ISSUES.md](C:/UE/T66/Model%20Generation/KNOWN_ISSUES.md)
- [NEXT_STEPS.md](C:/UE/T66/Model%20Generation/NEXT_STEPS.md)
- [CURRENT_HANDOFF_PROMPT.md](C:/UE/T66/Model%20Generation/CURRENT_HANDOFF_PROMPT.md)
- [MESH_APPROVAL_CHECKLIST.md](C:/UE/T66/Model%20Generation/MESH_APPROVAL_CHECKLIST.md)
- [WALLS_FLOORS_CEILINGS.md](C:/UE/T66/Model%20Generation/WALLS_FLOORS_CEILINGS.md)
- [SECOND_ATTEMPT_PROMPT.md](C:/UE/T66/Model%20Generation/SECOND_ATTEMPT_PROMPT.md)
- [bootstrap_trellis2_pod.sh](C:/UE/T66/Model%20Generation/Scripts/bootstrap_trellis2_pod.sh)

## Purpose

- Generate reference images for temporary benchmark characters.
- Run `TRELLIS.2` on RunPod to produce raw `.glb` meshes.
- Inspect results in Blender through Blender MCP.
- Keep all inputs, outputs, QA renders, and setup notes in one place.
- Preserve a repeatable baseline before retopo and Unreal import work.
- Generate modular environment-kit meshes for dungeon walls, floors, ceilings, and trim pieces.

## Folder Layout

```text
C:\UE\T66\Model Generation\
  README.md
  MASTER_WORKFLOW.md
  Model Processing.md
  Tools\
    BlenderLabMCP\
      launch_blender_lab_mcp.ps1
      setup_blender_lab_mcp.ps1
      start_blender_lab_mcp.py
    Trellis2\
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
    Environment\
      DungeonKit01\
        Inputs\
        Raw\
        Renders\
        Retopo\
        UnrealImport\
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
- For environment modules, generate one wall/floor/ceiling module at a time instead of full rooms.
- Keep generated dungeon-kit visuals separate from gameplay collision until Unreal tests prove the module set is stable.
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

First-use cache note: `rembg` downloads `/root/.u2net/u2net.onnx` the first time background removal runs. On a fresh pod, expect the first `/generate` request to pause while this roughly 176 MB file is fetched, or prewarm the cache before starting a batch:

```bash
python - <<'PY'
from PIL import Image
from rembg import remove
remove(Image.new("RGB", (8, 8), (0, 255, 0)))
PY
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
- Blender Lab MCP launcher: [launch_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1)
- Blender Lab MCP setup helper: [setup_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/setup_blender_lab_mcp.ps1)
- Blender Lab MCP in-Blender start script: [start_blender_lab_mcp.py](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/start_blender_lab_mcp.py)
- RetopoFlow rule: [RETOPOFLOW_4.md](C:/UE/T66/Model%20Generation/RETOPOFLOW_4.md)
- Blender QA render helper: [blender_glb_qa.py](C:/UE/T66/Model%20Generation/Scripts/blender_glb_qa.py)

## Blender Tool Roles

- Blender MCP is the canonical agent automation layer for import, export, scene inspection, scripted edits, screenshots, and repeatable QA.
- Use Blender Lab MCP from `https://www.blender.org/lab/mcp-server/` and `https://projects.blender.org/lab/blender_mcp`; do not use the third-party `ahujasid/blender-mcp` package for this project.
- RetopoFlow is the approved manual retopology tool for accepted low-poly topology work, especially hero meshes and deformation-critical regions.
- Blender Buddy, if installed locally, should be treated as an interactive in-Blender copilot for human-driven exploration and tool help, not as the authoritative workflow controller.

## Blender Session Rule

- Use live Blender through Blender MCP when it is reachable.
- If Blender MCP is disconnected or stale, run [launch_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1) before falling back to headless tooling.
- Headless Blender is the recovery path for QA render, export, normalization, and re-import verification when the live MCP session is unavailable.
- Before Unreal import, re-import any accepted exported GLB or FBX in Blender and render one verification image from the exported file itself.

## Retopo Rule

- Use RetopoFlow `4.1.5` for accepted low-poly topology work.
- Do not use Blender's Decimate modifier as the accepted poly-reduction path. It is only allowed for throwaway diagnostics or rejected prototypes.
- Static environment walls, floors, and ceilings may skip RetopoFlow when manual redraw would not materially improve the asset; in that case use the raw TRELLIS mesh after Unreal-ready normalization rather than decimating it.
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

From a Windows PowerShell-driven SSH session, prefer absolute paths so shell quoting cannot break `$PATH` and hide basic tools like `nohup`:

```powershell
ssh.exe -p <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 root@<POD_IP> "cd /workspace/TRELLIS.2 && export NVCC_PREPEND_FLAGS= && export CUDA_HOME=/usr/local/cuda && export PATH=/usr/local/cuda/bin:/usr/bin:/bin:/opt/conda/envs/trellis2/bin && export PYTHONPATH=/workspace/TRELLIS.2 && /usr/bin/nohup /opt/conda/envs/trellis2/bin/python -u /workspace/TRELLIS.2/trellis_server.py > /workspace/TRELLIS.2/trellis_server.log 2>&1 < /dev/null &"
```

If the local SSH command times out after submitting the background server, check the pod process list, `/workspace/TRELLIS.2/trellis_server.log`, and `/health` before relaunching.

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
scp.exe -P <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 "C:\UE\T66\Model Generation\Runs\Arthur\Inputs\Arthur_LegacyDirect_Body_Green.png" root@<POD_IP>:/tmp/Arthur_LegacyDirect_Body_Green.png
```

### Example Generate

```bash
curl --fail --silent --show-error --max-time 3600 \
  -X POST \
  -H 'X-Seed: 1440' \
  -H 'X-Texture-Size: 2048' \
  -H 'X-Decimation: 200000' \
  --data-binary @/tmp/Arthur_LegacyDirect_Body_Green.png \
  http://127.0.0.1:8000/generate \
  -o /tmp/Arthur_LegacyDirect_Body_Green_Trellis2.glb
```

### Example Download

```powershell
scp.exe -P <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 root@<POD_IP>:/tmp/Arthur_LegacyDirect_Body_Green_Trellis2.glb "C:\UE\T66\Model Generation\Runs\Arthur\Raw\Arthur_LegacyDirect_Body_Green_Trellis2.glb"
```

## Input Rules

For baseline reproduction:

- Use the real in-game screenshot look, not a stylized redraw, whenever possible.
- Use `opaque PNG` input.
- Use a solid green background.
- Keep the figure centered and readable.
- Avoid alpha inputs for baseline comparison.

### Hero Type A Character Input Rule

For the current masculine Type A hero pass, do not use one all-in character prompt as the production path.

The post-TRELLIS character setup, scale, material, import, and staged-game verification rules are maintained in [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md). Dedicated Unreal import commands and script runbooks live in [Model Importing.md](C:/UE/T66/Model%20Generation/Model%20Importing.md). Read both before changing assembly, rigging, import, or DataTable wiring.

- Generate the body image and head image separately.
- Keep the Type A body silhouette consistent across heroes so armor scale, shoulder width, torso mass, and leg proportions stay compatible.
- Keep head-only inputs completely straight-on like the body-only A-pose input. Reject any head image with a three-quarter turn, side tilt, sideways gaze, or off-center face axis.
- Vary each hero's head and face clearly; do not let every hero inherit Arthur's square head, brow, hairline, or facial proportions.
- Do not generate handheld weapons already attached to the body. TRELLIS can produce a usable weapon mesh from a clean isolated input, but weapon-on-character inputs do not reliably preserve grip placement or blade direction.
- Do not rely on a combined body-plus-head image when face identity matters. Full-body or body-plus-head generations lose too much head definition for the current hero-quality target.
- Treat head/body assembly and weapon placement as a Blender review/attachment step after separate TRELLIS outputs exist.
- Treat fallback seed-image generation as a count-unblocker only. It can produce source-image panels or poster-like GLBs even when TRELLIS completes successfully, so production approvals still require visually reviewed seed images plus Blender lineup QA after TRELLIS.
- For Type A head/body assembly, use the Blender script path first: `Model Generation/Scripts/assemble_typea_head_body.py`. The current validated baseline scales the head to `0.245` of body height, overlaps the neck by `0.045` of body height, and checks front/side/oblique renders before calling the join visually acceptable.
- For Type A rigging, first bake assembled GLB placement empties into mesh data before any weights are generated. The first working prototype was `Model Generation/Scripts/rig_typea_mike_prototype.py`; the batch path is now `Model Generation/Scripts/rig_typea_batch01.py`. It creates an `18`-bone deform armature, rigid/semi-rigid head weighting, procedural body weights, compact disconnected-fragment stabilization for TRELLIS meshes, saved idle arm-sway actions, FBX/GLB exports, and rest/action QA renders. Mark generated actions with `use_fake_user = True` so non-active actions survive in the saved `.blend`.
- For Type A Unreal import, follow [Model Importing.md](C:/UE/T66/Model%20Generation/Model%20Importing.md), use the full editor wrapper `Scripts/RunImportTypeABatch01RiggedHeroesAndExit.py`, and inspect `Saved/TypeABatch01RiggedHeroImportReport.json`. Do not use `UnrealEditor-Cmd.exe -run=pythonscript` for this skeletal mesh plus animation batch; UE 5.7 hit a Slate assertion in that path. Also do not use `-ExecCmds="py path"` because Unreal treats the script path as Python source text.
- Current Type A rigged FBXs should be authored in Blender at `2.0` meters tall with the mesh origin centered at the feet, then imported to Unreal with uniform scale `1.0`. The older `unreal_import_scale_map.json` was a historical workaround for pre-normalized exports; do not use Unreal import scaling as the production fix for Type A size or grounding.
- Current Type A material import depends on the packed base-color PNGs exported by `rig_typea_batch01.py`. After import, run the targeted material repair so `M_Character_Unlit` stays masked and `DiffuseColorMap.A` drives `Opacity Mask`; otherwise alpha-card source geometry can render as opaque poster panels even though the diffuse texture is present.
- If a mesh skeleton is replaced after earlier animation imports, use a fresh animation asset suffix such as `RigIdleV2`. Stale animation assets can retain an invalid skeleton and fail load/delete paths with `Invalid USkeleton` or `FKControlRig` errors.
- After Type A import, reload `DT_CharacterVisuals` from `Content/Data/CharacterVisuals.csv` and run `Scripts/VerifyTypeABatch01HeroVisuals.py`. The verification must confirm all `Hero_X_TypeA` and `Hero_X_TypeA_Beachgoer` rows load skeletal meshes, compatible idle animations, matching skeletons, near-200 cm bounds, near-zero bottom Z, texture-backed material slots, and the masked character-material alpha path.

Active Type A generation roster is documented in [TypeA_Masculine_Batch01](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/batch_plan.md). That roster keeps 12 heroes and removes Merlin, Zeus, Dog, and Forsen from the active gameplay/model-generation set.

Current Type A batch status: standard and beach goer skins for all 12 active heroes were rigged, imported, and wired to `DT_CharacterVisuals` on 2026-05-03. The accepted verification report is [TypeABatch01HeroVisualVerification.json](C:/UE/T66/Saved/TypeABatch01HeroVisualVerification.json), with `24/24` expected rows checked, `0` errors, all `48` material slots texture-backed, mesh heights in the `199.99997` to `200.00003` cm range, and bottom Z within `-0.000002` to `0.000024` cm. Gameplay-rendered visual proof images are [standard](C:/UE/T66/Saved/TypeABatch01VisualCheck/TypeABatch01_Gameplay_QA_standard_Yaw180.png) and [beach goer](C:/UE/T66/Saved/TypeABatch01VisualCheck/TypeABatch01_Gameplay_QA_beachgoer_Yaw180.png).

For the beach goer outfit set:

- Reuse each hero's accepted standard head.
- Generate a new beach body per hero.
- Generate a separate beach-inspired weapon per hero.
- Keep beach outfits varied across heroes; do not make every hero wear the same trunks, vest, colors, or prop family.
- Continue avoiding below-knee cloth and weapon-on-body prompts.

Current Arthur body/head baseline inputs:

- [Arthur_LegacyDirect_Body_Green.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_LegacyDirect_Body_Green.png)
- [Arthur_LegacyDirect_Head_Green.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_LegacyDirect_Head_Green.png)

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

- Separate head passes are now required for the new Type A hero batch when face identity needs to read clearly.
- The body is strong enough to carry equipment and lineup work.
- Weapon-on-body TRELLIS inputs are not accepted for production. The sword hold failures showed that generated placement is unreliable; use separate weapon generation plus Blender placement instead.
- Arthur's current idle hand mesh is still too open for a truly tight grip, so transform-only placement may not be enough.
- Combined body-plus-head inputs are not accepted for production hero identity checks because the head/face loses too much definition.
- Off-axis head-only inputs are not accepted for production because the head and body need to read as parts derived from the same forward-facing A-pose character.

Earlier test assets remain useful for comparison:

- [Arthur_LegacyProxy_Body_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_LegacyProxy_Body_Raw.png)
- [Arthur_LegacyProxy_Head_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_LegacyProxy_Head_Raw.png)
- [Arthur_LegacyDirect_Body_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_LegacyDirect_Body_Raw.png)
- [Arthur_LegacyDirect_Head_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_LegacyDirect_Head_Raw.png)

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

1. Review the imported Type A standard and beach goer heroes in-editor for art issues that automated checks cannot catch, especially fallback-derived panels or card-like fragments.
2. Reroll any failed Type A source images from better reviewed ImageGen references, then re-run assembly, rigging, import, and `Scripts/VerifyTypeABatch01HeroVisuals.py`.
3. Add a socket or attachment rule for separate hero weapons; do not return to weapon-on-body TRELLIS inputs.
4. Replace the placeholder arm-sway idle with production locomotion only after the imported Type A mesh scale and skeleton compatibility stay stable.
5. Extend the same `reference -> TRELLIS -> assemble -> rig -> import -> verify` workflow to the next character or enemy batch only after the local Type A review is accepted.

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
