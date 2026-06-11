# Pixal3D RunPod Setup

Use this doc when standing up or refreshing the Pixal3D RunPod path. Pixal3D is
production-cleared for T66 replacement assets, while remaining technically
separate from TRELLIS.

## Read First

1. `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
2. `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`
3. `11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` for active FriendSlop production assets
4. `../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`
5. This setup doc
6. `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` if setup or export fails

## Expected Pod Shape

- GPU: A40-class and L40S pods have been validated. Other NVIDIA GPUs may need
  dependency rebuilds.
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
A40 pods, `natten==0.21.0` must be rebuilt for CUDA arch `8.6`; on L40S pods,
use CUDA arch `8.9`. Otherwise NAF can fail with `no kernel image is available
for execution on the device`.

For L40S pods, force the rebuild with:

```bash
PIXAL3D_NATTEN_SOURCE_ARCH=8.9 bash /tmp/t66_pixal3d/bootstrap_pixal3d_pod.sh
```

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
and VRAM numbers.

## Default Generation Settings

Use these defaults for T66 Pixal3D production replacement runs:

```text
X-Seed: 1337
X-Resolution: 1536
X-Texture-Size: 4096
X-Decimation: 200000
X-Remesh: 1
X-Export-Fallback: 0
X-Fallback-Decimation: 80000
X-Safe-Fill-Holes-Fallback: 0
X-SS-Steps: 25
X-SS-Guidance: 7.5
X-Shape-Steps: 25
X-Shape-Guidance: 7.5
X-Tex-Steps: 25
X-Tex-Guidance: 4.0
```

Leave export fallback disabled for strict production. `X-Decimation: 200000` is
the production face target; fallback exports are diagnostic evidence, not
accepted production output. Use `--diagnostic-mode` with
`X-Export-Fallback: 1` and `X-Safe-Fill-Holes-Fallback: 1` only when
investigating CuMesh export failures. If that fallback is used, the response
headers must show it and the production wrapper must fail or explicitly mark a
per-asset accepted limitation.

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
  --decimation 200000 `
  --fallback-decimation 80000 `
  --remesh
```

The smoke set covers:

- tree or organic prop
- wall or modular environment piece
- humanoid character
- monster or non-human creature

For every generated row, verify:

- the local GLB exists and has nonzero byte size
- `generation_attempts` records the actual export settings, including any safe
  `fill_holes` fallback
- Blender QA imports the GLB and writes a front render
- Blender QA metadata includes triangle count and nonzero bounds
- character-like rows can run Blender QA and FriendSlop import validation; retired QuadRetro processing is archived and should not be used for active FriendSlop assets unless the user explicitly revives that path

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

### Reliable completion notification (agents)

Inline poll loops die inside agent tool windows and silently drop the chain
(measured repeatedly). Use the detached watcher instead:

```powershell
# after `launch`: start the watcher DETACHED, output to a log
Start-Process pwsh -WindowStyle Hidden -ArgumentList '-NoProfile','-File',
  'Model Generation\Pixal3D\Scripts\PollPixal3DRun.ps1',
  '-LocalRunRoot','<local run root>','-RemoteRunRoot','<remote run root>' `
  -RedirectStandardOutput '<local run root>\poll.log'
```

Then arm a PERSISTENT file monitor on `<local run root>\poll.log` for the
`POLL_RESULT` line (Claude: Monitor tool, persistent, tail -f + grep
--line-buffered "POLL_RESULT"). The watcher polls DONE.json with per-call SSH
timeouts (transient hangs never kill it), auto-downloads the GLBs on
completion (with scp fallback), and prints exactly one final line:
`POLL_RESULT=DONE files=N | FAILED | TIMEOUT`.

## Output And Retention

Generated output belongs under:

```text
Model Generation/Runs/Pixal3D/...
```

Keep summary reports and durable lessons. Delete generated runs, source plates,
temporary GLBs, Blender scenes, and screenshots after the production import or
diagnostic review is complete unless the user explicitly asks to retain a run
folder.

## Promotion Boundary

Pixal3D assets move into active runtime content through
`11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`, the relevant production
manifest/source-run evidence, explicit texture/material binding, Unreal import
validation, DataTable reloads when needed, and standalone verification gates.
