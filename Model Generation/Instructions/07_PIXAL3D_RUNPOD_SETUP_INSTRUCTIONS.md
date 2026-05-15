# Pixal3D RunPod Setup

Use this doc when standing up or refreshing the separate Pixal3D RunPod path.
Pixal3D is evaluated as a research pipeline only. Do not replace the TRELLIS
path or import Pixal3D output into shipped Unreal content unless licensing is
explicitly cleared.

## Read First

1. `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
2. `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`
3. `../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
4. This setup doc
5. `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` if setup or export fails

## Expected Pod Shape

- GPU: A40-class pod has been validated. Other NVIDIA GPUs may need dependency
  rebuilds.
- Remote checkout: `/workspace/Pixal3D`
- Conda environment: `pixal3d`
- Local service port: `18001`
- Server file on pod: `/workspace/Pixal3D/pixal3d_server.py`
- Runtime output namespace: `/workspace/T66/ModelGeneration/Runs/Pixal3D/...`

Do not use the public Jupyter port as the model server port. The Flask service
runs locally on the pod and is reached through SSH commands from the local smoke
runner.

## Local Files

- `../Pixal3D/Server/pixal3d_server.py`
- `../Pixal3D/Scripts/bootstrap_pixal3d_pod.sh`
- `../Pixal3D/Scripts/Invoke-Pixal3DHfLogin.ps1`
- `../Pixal3D/Scripts/run_pixal3d_smoke.py`
- `../Pixal3D/Scripts/run_pixal3d_batch.py`
- `../LOCAL_ACCESS.env`

`LOCAL_ACCESS.env` is the repo-local secret convention. Do not paste tokens into
docs, reports, or chat handoffs.

## Bootstrap

Copy and run the bootstrap script on the pod:

```bash
bash /tmp/t66_pixal3d/bootstrap_pixal3d_pod.sh
```

The bootstrap installs Pixal3D into a separate `pixal3d` conda environment. On
A40 pods, `natten==0.21.0` must be rebuilt for CUDA arch `8.6`; otherwise NAF
can fail with `no kernel image is available for execution on the device`.

If Hugging Face auth is needed, use the local helper:

```powershell
powershell -ExecutionPolicy Bypass -File "Model Generation/Pixal3D/Scripts/Invoke-Pixal3DHfLogin.ps1" `
  -PodIp <pod-ip> `
  -PodPort <ssh-port>
```

The helper reads the token from `Model Generation/LOCAL_ACCESS.env`.

## Upload And Start Server

Prefer the smoke runner for server upload and startup because it uses the
current repo server file:

```powershell
python "Model Generation/Pixal3D/Scripts/run_pixal3d_smoke.py" `
  --pod-ip <pod-ip> `
  --pod-port <ssh-port> `
  --upload-support `
  --start-server `
  --skip-generation `
  --skip-qa
```

Manual startup on the pod is:

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate pixal3d
export PYTHONPATH=/workspace/Pixal3D
export PIXAL3D_PORT=18001
export PIXAL3D_ATTN_BACKEND=flash_attn_3
export PIXAL3D_SPARSE_ATTN_BACKEND=flash_attn_3
export PIXAL3D_SPARSE_CONV_BACKEND=flex_gemm
cd /workspace/Pixal3D
python -u pixal3d_server.py
```

Health check:

```bash
curl -sS --max-time 10 http://127.0.0.1:18001/health
```

The response must include `"status":"ok"`, `"pipeline_loaded":true`, GPU name,
VRAM numbers, and the license warning.

## Default Generation Settings

Use these defaults for comparable T66 Pixal3D research runs:

```text
X-Seed: 1337
X-Resolution: 1024
X-Texture-Size: 2048
X-Decimation: 80000
X-Remesh: 1
X-Export-Fallback: 1
X-Fallback-Decimation: 30000
X-SS-Steps: 12
X-SS-Guidance: 7.5
X-Shape-Steps: 12
X-Shape-Guidance: 7.5
X-Tex-Steps: 12
X-Tex-Guidance: 1.0
```

Leave export fallback enabled. It is part of the T66 Pixal3D server contract,
not an experimental toggle.

## Smoke Test

Run the four-sample smoke test before a larger batch:

```powershell
python "Model Generation/Pixal3D/Scripts/run_pixal3d_smoke.py" `
  --pod-ip <pod-ip> `
  --pod-port <ssh-port> `
  --upload-support `
  --start-server `
  --force-generation `
  --force-qa `
  --decimation 80000 `
  --remesh
```

The smoke set covers:

- tree or organic prop
- wall or modular environment piece
- humanoid character
- monster or non-human creature

For every generated row, verify:

- the local GLB exists and has nonzero byte size
- `generation_attempts` records the actual export settings
- Blender QA imports the GLB and writes a front render
- Blender QA metadata includes triangle count and nonzero bounds
- character-like rows can run Quad Retro when requested

## Detached Experiment Batch

For any custom experiment with more than one source image, use the detached
batch runner. It uploads sources, starts one remote job with `nohup`, writes
`Logs/pixal3d_generation_status.jsonl`, writes `Logs/DONE.json` on completion,
and polls through short SSH calls so the local agent does not hang on a long
foreground session.

Example:

```powershell
python "Model Generation/Pixal3D/Scripts/run_pixal3d_batch.py" run `
  --pod-ip <pod-ip> `
  --pod-port <ssh-port> `
  --local-run-root "Model Generation/Experiments/MyPixal3DExperiment" `
  --remote-run-root "/workspace/T66/ModelGeneration/Experiments/MyPixal3DExperiment" `
  --resolution 1536 `
  --image-resolution 1024 `
  --texture-size 2048 `
  --decimation 200000 `
  --ss-steps 25 `
  --shape-steps 25 `
  --tex-steps 25 `
  --tex-guidance 4.0
```

Use `launch`, `poll`, and `download` subcommands separately when a run should
continue while Codex works on other local tasks. A batch is complete only after
`DONE.json` exists or every expected GLB has nonzero size and a matching HTTP
200 row in the status JSONL.

## Output And Retention

Generated output belongs under:

```text
Model Generation/Runs/Pixal3D/...
```

Keep summary reports and durable lessons. Delete generated runs, source plates,
temporary GLBs, Blender scenes, and screenshots after the evaluation is complete
unless the user explicitly asks to retain an experiment folder.

## Promotion Boundary

Pixal3D output remains research-only until legal approval changes the licensing
status. Do not import Pixal3D assets into runtime Unreal content or refresh a
playable standalone build from Pixal3D output without explicit user approval.
