# Pixal3D RunPod Setup Evidence

## Task Contract

Working task:
Operator: Codex
Validator: Claude if the local T66 validator path is available
Scope: set up the Pixal3D service on the RunPod shown in the screenshot, using the repo's current Pixal3D setup instructions and verify service health.
Stop condition: the pod is reachable, setup is run or resumed, and `/health` proves the service is up with the expected pipeline state, or the concrete blocker is reported.

## Pod

- Direct SSH: `root@69.30.85.73 -p 22079`
- RunPod pod ID from screenshot: `9nl6plrihaoybh`
- Hostname observed: `43c75bb1686f`
- GPU observed before setup: `NVIDIA A40, 8.6, 570.195.03, 46068 MiB`

## Setup Performed

1. Probed SSH and GPU with the screenshot's direct TCP endpoint.
2. Uploaded `Model Generation/Pixal3D/Scripts/bootstrap_pixal3d_pod.sh` to `/tmp/t66_pixal3d/bootstrap_pixal3d_pod.sh`.
3. Ran the bootstrap with A40 source-build settings:
   - `PIXAL3D_NATTEN_SOURCE_ARCH=8.6`
   - `PIXAL3D_NATTEN_N_WORKERS=16`
4. Ran `Model Generation/Pixal3D/Scripts/Invoke-Pixal3DHfLogin.ps1 -PodIp 69.30.85.73 -Port 22079`.
5. Started the service through the repo smoke runner without generation or QA:
   - `python "Model Generation/Pixal3D/Scripts/run_pixal3d_smoke.py" --pod-ip 69.30.85.73 --pod-port 22079 --upload-support --start-server --skip-generation --skip-qa --server-port 18001 --health-timeout 1800`
   - Runner output included `PIXAL3D_SERVER_PID=8377`.

## Evidence

Bootstrap log markers from `/tmp/t66_pixal3d/bootstrap.log`:

```text
Building NATTEN from source for CUDA arch 8.6
Successfully installed natten-0.21.0
torch 2.6.0+cu124
cuda 12.4
gpu NVIDIA A40
pixal3d pipeline import OK
Bootstrap complete.
```

Server process:

```text
8377 python -u /workspace/Pixal3D/pixal3d_server.py
```

Server log markers from `/workspace/Pixal3D/pixal3d_server.log`:

```text
[SPARSE] Conv backend: flex_gemm; Attention backend: flash_attn_3
[PIXAL3D] Loading pipeline from TencentARC/Pixal3D low_vram=False
[ATTENTION] Using backend: flash_attn_3
[PIXAL3D] Building image-conditioning models
[PIXAL3D] Preloading NAF upsamplers
[PIXAL3D] Loading MoGe camera model Ruicheng/moge-2-vitl
[PIXAL3D] Models loaded
[PIXAL3D] Starting server on port 18001
```

Health response from `curl http://127.0.0.1:18001/health` over SSH:

```json
{
  "attention_backend": "flash_attn_3",
  "cuda": true,
  "gpu": "NVIDIA A40",
  "loaded_at": 1780992997.584098,
  "low_vram": false,
  "model_path": "TencentARC/Pixal3D",
  "pipeline_loaded": true,
  "sparse_attention_backend": "flash_attn_3",
  "sparse_conv_backend": "flex_gemm",
  "status": "ok",
  "vram_allocated_gb": 20.2,
  "vram_reserved_gb": 20.3,
  "vram_total_gb": 47.7
}
```

## Scope Notes

- No generation batch was run.
- No Unreal import was run.
- The setup stopped at the requested service health verification.
