# TRELLIS.2 RunPod Character Workflow

Superseded by the authoritative master doc at:

- [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md)

This is the working runbook for the T66 image-to-3D character pipeline.

It replaces the older JSX setup guide as the canonical internal reference.

## Purpose

- Generate character concept images locally through Codex image generation.
- Send approved images to TRELLIS.2 on RunPod.
- Receive `.glb` output for Blender inspection and retopo.
- Keep the pod setup reproducible without depending on one specific pod state.

## Canonical Files

- Workflow doc: `C:\UE\T66\Docs\Art\TRELLIS2_RunPod_Character_Workflow.md`
- Server template: `C:\UE\T66\Tools\Trellis2\trellis_server.py`
- Pod repo path: `/workspace/TRELLIS.2`
- Pod server path: `/workspace/TRELLIS.2/trellis_server.py`
- Pod log path: `/workspace/TRELLIS.2/trellis_server.log`

## Current Pipeline Decisions

- Use `TRELLIS.2`, not TRELLIS v1.
- Generate `body` and `head` separately when face fidelity matters.
- Keep weapons separate from TRELLIS output unless there is a strong reason not to.
- Use TRELLIS for the temporary benchmark look, then do Blender cleanup and retopo after mesh approval.
- Use an SSH tunnel for generation requests. Do not rely on the RunPod HTTP proxy for long generations.

## Local Prerequisites

- Blender 5.0 installed.
- Blender MCP add-on installed and enabled.
- Codex MCP config updated to include the Blender server.
- Codex restarted after MCP config changes.
- RetopoFlow package installed if manual retopo is needed.

## Recommended Pod Template

Use this when creating a fresh pod:

```text
runpod/pytorch:2.4.0-py3.11-cuda12.4.1-devel-ubuntu22.04
```

Expose:

- `8000` for the TRELLIS server
- `8888` for Jupyter if needed
- SSH as usual

## If The Pod Is A Bare Ubuntu/Python Pod

The current working pod was not the recommended devel template. It still worked after these bootstrap steps:

```bash
apt-get update
apt-get install -y wget curl git cmake libjpeg-dev zlib1g-dev pkg-config build-essential
wget -q https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh -O /tmp/miniforge.sh
bash /tmp/miniforge.sh -b -p /opt/conda
source /opt/conda/etc/profile.d/conda.sh
```

## Clone TRELLIS.2

```bash
cd /workspace
git clone --recursive https://github.com/microsoft/TRELLIS.2.git
```

## Create The Environment

```bash
source /opt/conda/etc/profile.d/conda.sh
conda create -y -n trellis2 python=3.10
conda activate trellis2
python -m pip install --index-url https://download.pytorch.org/whl/cu124 torch==2.6.0 torchvision==0.21.0
```

Notes:

- In the current working pod, `conda install cuda-toolkit=12.4` pulled a `12.9` nvcc toolchain. The compiled stack still worked.
- The recommended devel template should reduce this mismatch.

## Install TRELLIS Dependencies

Run the parts that actually matter, in this order:

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
export NVCC_PREPEND_FLAGS=
export CUDA_HOME=/opt/conda/envs/trellis2
cd /workspace/TRELLIS.2
. ./setup.sh --basic
. ./setup.sh --o-voxel
. ./setup.sh --nvdiffrast
. ./setup.sh --nvdiffrec
python -m pip install psutil
python -m pip install flash-attn==2.7.3 --no-build-isolation
```

Important notes from the working setup:

- `setup.sh --basic` may try to call `sudo apt install -y libjpeg-dev`. On root-based RunPod shells without `sudo`, this is harmless if `libjpeg-dev` is already installed.
- `pillow-simd` failed in the current pod and was not required to get the stack working. Regular `Pillow` was already installed and sufficient.
- `flash-attn` needs `psutil` installed first when using `--no-build-isolation`.

## Verify The Native Stack

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
export NVCC_PREPEND_FLAGS=
export CUDA_HOME=/opt/conda/envs/trellis2
export PYTHONPATH=/workspace/TRELLIS.2
python - <<'PY'
import torch; print("torch OK", torch.__version__)
import nvdiffrast; print("nvdiffrast OK")
import cumesh; print("cumesh OK")
import o_voxel; print("o_voxel OK")
import flash_attn; print("flash_attn OK")
from trellis2.pipelines import Trellis2ImageTo3DPipeline; print("Pipeline imports OK")
PY
```

Expected key line:

```text
[SPARSE] Conv backend: flex_gemm; Attention backend: flash_attn
```

## Hugging Face Requirements

TRELLIS.2 needs access to gated model dependencies. Before first server start:

- Accept access in the browser for `facebook/dinov3-vitl16-pretrain-lvd1689m`
- Accept access in the browser for `briaai/RMBG-2.0`

Login on the pod:

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
python -c "from huggingface_hub import login; login(token='YOUR_HF_TOKEN_HERE')"
```

Do not store tokens in repo files.

## Apply The Two Required Code Fixes

### 1. Replace BiRefNet With rembg

Install support packages:

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
python -m pip install flask onnxruntime rembg
```

Replace `BiRefNet.py` with a rembg-backed wrapper:

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

Path:

```text
/workspace/TRELLIS.2/trellis2/pipelines/rembg/BiRefNet.py
```

### 2. Patch cfg_strength Leakage

Patch this line:

```text
/workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py
```

Use:

```bash
sed -i 's/return model(x_t, t, cond, \*\*kwargs)/kwargs.pop("cfg_strength", None); return model(x_t, t, cond, **kwargs)/' /workspace/TRELLIS.2/trellis2/pipelines/samplers/flow_euler.py
```

## Sync The Server Script To The Pod

Use the repo template at:

```text
C:\UE\T66\Tools\Trellis2\trellis_server.py
```

and copy it to:

```text
/workspace/TRELLIS.2/trellis_server.py
```

If needed from Windows:

```powershell
scp -P <SSH_PORT> -i ~/.ssh/id_ed25519 C:\UE\T66\Tools\Trellis2\trellis_server.py root@<POD_IP>:/workspace/TRELLIS.2/trellis_server.py
```

## Start The Server

Use unbuffered Python so the log is readable:

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
export NVCC_PREPEND_FLAGS=
export CUDA_HOME=/opt/conda/envs/trellis2
export PYTHONPATH=/workspace/TRELLIS.2
cd /workspace/TRELLIS.2
nohup python -u /workspace/TRELLIS.2/trellis_server.py > /workspace/TRELLIS.2/trellis_server.log 2>&1 < /dev/null &
```

Watch startup:

```bash
tail -f /workspace/TRELLIS.2/trellis_server.log
```

Expected lines:

```text
[SPARSE] Conv backend: flex_gemm; Attention backend: flash_attn
[TRELLIS] Loading pipeline...
[ATTENTION] Using backend: flash_attn
[TRELLIS] Pipeline loaded!
[TRELLIS] Environment map loaded!
[TRELLIS] Starting server on port 8000...
```

## Create The SSH Tunnel

From Windows:

```powershell
ssh -L 8000:localhost:8000 root@<POD_IP> -p <SSH_PORT> -i ~/.ssh/id_ed25519 -N
```

## Health Check

```powershell
Invoke-RestMethod -Uri "http://localhost:8000/health" -Method Get
```

## Generate A Model

```powershell
curl.exe -X POST -H "Content-Type: image/png" -H "X-Seed: 42" -H "X-Decimation: 200000" -H "X-Texture-Size: 2048" --data-binary "@C:\\TRELLIS\\images\\Test.png" --output "C:\\TRELLIS\\outputs\\Test.glb" --max-time 600 http://localhost:8000/generate
```

## Character Workflow After Server Setup

- Generate a clean front-view image in Codex.
- Prefer white or easy-to-remove backgrounds.
- For faces, generate the `head` separately from the `body`.
- Run `body` and `head` through TRELLIS.2 separately if needed.
- Inspect the returned `.glb` in Blender before retopo.
- Only proceed to retopo if silhouette, proportions, and temporary style match the target closely enough.

## Blender Side Notes

- Blender MCP is the preferred control bridge for Codex-to-Blender work.
- Codex must be restarted after updating `C:\Users\DoPra\.codex\config.toml`.
- The currently downloaded RetopoFlow zip was RF3 source, not the packaged RF4 build. Treat that as a separate upgrade task.

## Known Failure Cases

- Empty server log after background start: use `python -u`, not plain `python`.
- RunPod proxy timeout during generation: use SSH tunneling, not the public HTTP proxy.
- `flash-attn` metadata failure: install `psutil` first, then retry `pip install flash-attn==2.7.3 --no-build-isolation`.
- `pillow-simd` build failure: not a blocker for the current TRELLIS workflow.
- Floor/platform generated under the character: background removal is not working. Re-check the `BiRefNet` override and input background.
- `cfg_strength` unexpected keyword: sampler patch missing.
- Gated model 403 errors: the HF account has not accepted access for required models.
