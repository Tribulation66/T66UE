# T66 World Generation Setup

This file records the setup baseline for `T66` HY-World room-generation experiments as of `2026-04-29`.

Use this file before starting a RunPod session. Use [ROOM_GENERATION_PROCESS.md](C:/UE/T66/World%20Generation/ROOM_GENERATION_PROCESS.md) after the pod is reachable.

## Current Release Boundary

Official sources still matter more than summaries or social posts.

Current official status:

- `WorldMirror 2.0` inference code and model weights are public.
- Full image/text-to-world generation is still marked `Coming Soon`.
- `HY-Pano 2.0`, `WorldNav`, and `WorldStereo 2.0` are still marked `Coming Soon`.
- The public repo's `hyworld2/worldgen/README.md` says the world-generation module is not open-sourced in the repository yet.

Practical meaning for `T66`:

- We can prepare and run the public `WorldMirror 2.0` reconstruction path on RunPod.
- A true single-image `dungeon room image -> navigable room mesh` flow is not currently available through the public repo alone.
- Until Tencent releases the full stack, room generation must use one of these controlled experiment paths:
  - public `WorldMirror 2.0` with multi-view or video room inputs
  - Tencent's hosted/product path, if available and license-acceptable
  - an interim research stack explicitly labeled as non-canonical

Do not document a generated dungeon-room run as successful until the output has been imported, inspected, and tested in `T66`.

## Local Workspace

```text
C:\UE\T66\World Generation\
  MASTER.md
  MEMORY.md
  USEFUL_LINKS.md
  SETUP.md
  ROOM_GENERATION_PROCESS.md
  Runs\
    DungeonRoom01\
      Inputs\
      PodOutput\
      BlenderQA\
      UnrealImport\
      Notes\
```

Create run folders on demand. Keep every generated image, pod output, QA render, conversion log, and Unreal import candidate under the run folder.

## Local Cache

Older research cache paths:

- `C:\UE\T66\tmp\worldgen_research\HY-World-2.0_repo`
- `C:\UE\T66\tmp\worldgen_research\HY_World_2_0.pdf`

Treat the local clone as a convenience cache only. Refresh from the official GitHub repo before installing or debugging a pod.

## TRELLIS RunPod Target For Modular Kits

For modular dungeon/world-kit generation, use the `Model Generation` TRELLIS.2
baseline. Do not use the HY-World / WorldMirror package versions in the next
section for TRELLIS work.

Authoritative local files:

- [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md)
- [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
- [bootstrap_trellis2_pod.sh](C:/UE/T66/Model%20Generation/Scripts/bootstrap_trellis2_pod.sh)
- [Invoke-RunPodHfLogin.ps1](C:/UE/T66/Model%20Generation/Scripts/Invoke-RunPodHfLogin.ps1)

Current locked TRELLIS baseline:

- repo: `microsoft/TRELLIS.2`
- commit: `5565d240c4a494caaf9ece7a554542b76ffa36d3`
- repo path on pod: `/workspace/TRELLIS.2`
- conda env: `trellis2`
- Python: `3.10`
- RunPod template: `runpod/pytorch:2.4.0-py3.11-cuda12.4.1-devel-ubuntu22.04`
- required exposed ports: `8000`, `8888`, and SSH
- `torch==2.6.0` from the `cu124` wheel index
- `torchvision==0.21.0` from the `cu124` wheel index
- `transformers==5.2.0`
- `flash-attn==2.7.3`
- native extensions: `cumesh`, `o_voxel`, `flex_gemm`,
  `nvdiffrast.torch`, and `nvdiffrec_render`
- active compatibility patches: `BiRefNet.py` replaced by the `rembg` wrapper,
  and `flow_euler.py` drops leaked `cfg_strength`
- Hugging Face access required for `facebook/dinov3-vitl16-pretrain-lvd1689m`
- server: `/workspace/TRELLIS.2/trellis_server.py`
- health check: `curl -s http://127.0.0.1:8000/health`
- generation endpoint: `POST http://127.0.0.1:8000/generate`

Bootstrap a fresh TRELLIS pod by uploading and running:

```powershell
scp.exe -P <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 `
  "C:\UE\T66\Model Generation\Scripts\bootstrap_trellis2_pod.sh" `
  root@<POD_IP>:/workspace/bootstrap_trellis2_pod.sh

ssh.exe -p <SSH_PORT> -i C:\Users\DoPra\.ssh\id_ed25519 root@<POD_IP> `
  "chmod +x /workspace/bootstrap_trellis2_pod.sh && bash /workspace/bootstrap_trellis2_pod.sh"
```

Then authenticate Hugging Face without printing or writing raw tokens into repo
docs:

```powershell
powershell -ExecutionPolicy Bypass `
  -File "C:\UE\T66\Model Generation\Scripts\Invoke-RunPodHfLogin.ps1" `
  -PodIp "<POD_IP>" `
  -Port <SSH_PORT>
```

Start the TRELLIS server with the command in
[MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md). Prefer
pod-local `curl` plus `scp` for generation; SSH tunnels are not the reliable
path for long runs.

Known-good modular-kit settings:

- seed: `1337`
- texture size: `2048`
- decimation: `80000`
- `preprocess_image=True`
- source images: split module crops, not full coherent sheets

## HY-World / WorldMirror RunPod Target

Preferred first pod:

- Linux GPU pod with CUDA `12.4` support
- Python `3.10`
- high-memory NVIDIA GPU preferred for first experiments
- expose SSH plus at least one HTTP port for Gradio, usually `8081`
- persistent volume recommended if we expect repeated tests

Official install baseline:

- `torch==2.4.0`
- `torchvision==0.19.0`
- CUDA `12.4` wheel index
- repo requirements from `requirements.txt`
- FlashAttention recommended by the official docs

Single-GPU inference is the first target. Multi-GPU is available for `WorldMirror 2.0`, but the official pipeline requires the number of input images to be at least the GPU count.

This section is for HY-World / WorldMirror reconstruction only. For TRELLIS
modular-kit generation, use the locked TRELLIS section above.

## HY-World Pod Bootstrap Draft

Run this after the user provides RunPod SSH details for a HY-World /
WorldMirror reconstruction experiment.

```bash
cd /workspace
git clone https://github.com/Tencent-Hunyuan/HY-World-2.0.git
cd HY-World-2.0

conda create -n hyworld2 python=3.10 -y
source /workspace/miniconda3/etc/profile.d/conda.sh
conda activate hyworld2

pip install torch==2.4.0 torchvision==0.19.0 --index-url https://download.pytorch.org/whl/cu124
pip install -r requirements.txt

# Preferred if the pod can compile it cleanly.
pip install flash-attn --no-build-isolation
```

If FlashAttention fails to build, stop and record the exact compiler/CUDA error. Do not keep changing versions blindly.

## Hugging Face Access

The first `WorldMirrorPipeline.from_pretrained("tencent/HY-World-2.0")` call downloads weights from Hugging Face.

Before running inference, confirm:

- the pod can reach Hugging Face
- the correct token is available if gated files require it
- model cache is on the persistent volume if repeated runs are expected

Recommended cache root:

```bash
export HF_HOME=/workspace/hf_cache
export HUGGINGFACE_HUB_CACHE=/workspace/hf_cache/hub
```

## Smoke Tests

Run these before uploading T66 images.

```bash
conda activate hyworld2
cd /workspace/HY-World-2.0
python - <<'PY'
from hyworld2.worldrecon.pipeline import WorldMirrorPipeline
print("WorldMirror import OK")
PY
```

Optional Gradio smoke test:

```bash
python -m hyworld2.worldrecon.gradio_app --host 0.0.0.0 --port 8081
```

CLI shape for a reconstruction run:

```bash
python -m hyworld2.worldrecon.pipeline \
  --input_path /workspace/t66_worldgen/DungeonRoom01/input_views \
  --output_path /workspace/t66_worldgen/DungeonRoom01/out \
  --target_size 952 \
  --save_rendered \
  --no_interactive
```

Expected useful outputs include:

- `camera_params.json`
- `points.ply`
- `gaussians.ply`
- `depth/*.png` and `depth/*.npy`
- `normal/*.png`
- `rendered/rendered_rgb.mp4`, if `--save_rendered` succeeds
- `pipeline_timing.json`

## Transfer Paths

Local run folder:

```text
C:\UE\T66\World Generation\Runs\DungeonRoom01
```

Suggested pod folder:

```text
/workspace/t66_worldgen/DungeonRoom01
```

PowerShell upload example:

```powershell
scp.exe -P <SSH_PORT> -i <KEY_PATH> -r "C:\UE\T66\World Generation\Runs\DungeonRoom01\Inputs\input_views" root@<POD_IP>:/workspace/t66_worldgen/DungeonRoom01/
```

PowerShell download example:

```powershell
scp.exe -P <SSH_PORT> -i <KEY_PATH> -r root@<POD_IP>:/workspace/t66_worldgen/DungeonRoom01/out "C:\UE\T66\World Generation\Runs\DungeonRoom01\PodOutput"
```

## Unreal Import Boundary

Do not copy raw pod outputs directly into live gameplay content.

Required order:

1. Store raw pod output under `World Generation/Runs/<RunName>/PodOutput`.
2. Inspect point cloud, Gaussian, or mesh output in Blender.
3. Create or verify a gameplay-scale mesh candidate.
4. Create a simplified collision proxy if the visual mesh is not collision-clean.
5. Put accepted GLB candidates under `SourceAssets/Import/WorldGen/<RunName>/`.
6. Import through the project static-mesh import path.
7. Confirm materials end up on `M_GLB_Unlit` or another approved world material path.
8. Test in `GameplayLevel` before wiring it into the start-flow experiment.

For generated rooms, do not rely on a single visual mesh for gameplay collision unless the collision has been tested. Use separate floor, wall, portal, and blocker proxies when needed.

## RunPod Information Needed From User

When the user spins up the pod, ask for:

- pod IP or hostname
- SSH port
- username, usually `root`
- key path or password method
- exposed HTTP port for Gradio, if any
- GPU model and VRAM
- whether the pod has persistent storage
- whether Hugging Face login/token is already configured

With those details, the next step is to bootstrap HY-World, run the smoke test, and then generate or reconstruct `DungeonRoom01`.

## License Check

The official license says it does not apply in the `European Union`, `United Kingdom`, or `South Korea`, defines the licensed territory as worldwide excluding those regions, and restricts use/output outside that territory.

For internal prototyping in the current United States context, continue to keep license notes attached to the workflow. Before production distribution, get a proper legal review.
