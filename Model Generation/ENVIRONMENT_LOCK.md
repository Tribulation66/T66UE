# Environment Lock

This file records the known-good baseline used for the restored JSX-parity TRELLIS workflow.

If future agents need to recreate or debug the pod, start here.

## Canonical Pod Baseline

- Date locked: `2026-04-18`
- TRELLIS repo: `microsoft/TRELLIS.2`
- Repo commit: `5565d240c4a494caaf9ece7a554542b76ffa36d3`
- Python env name: `trellis2`
- Python version: `3.10`

## Recommended RunPod Template

```text
runpod/pytorch:2.4.0-py3.11-cuda12.4.1-devel-ubuntu22.04
```

Expose:

- `8000`
- `8888`
- SSH

## Required Python Package Versions

- `torch==2.6.0` from the `cu124` wheel index
- `torchvision==0.21.0` from the `cu124` wheel index
- `transformers==5.2.0`
- `flash-attn==2.7.3`
- `flask`
- `onnxruntime`
- `rembg`
- `psutil`
- `opencv-python-headless`
- `gradio==6.0.1`
- `kornia`
- `timm`
- `trimesh`
- `lpips`
- `pandas`
- `tensorboard`
- `zstandard`

## Native Extensions Expected

- `cumesh`
- `o_voxel`
- `flexgemm`
- `nvdiffrast`
- `nvdiffrec`

## Required Hugging Face Access

- `facebook/dinov3-vitl16-pretrain-lvd1689m`
- `briaai/RMBG-2.0`

## Canonical Patch Set

These are required:

1. `trellis2/pipelines/rembg/BiRefNet.py`
   - replace with `rembg` wrapper
2. `trellis2/pipelines/samplers/flow_euler.py`
   - drop leaked `cfg_strength`
3. `/workspace/TRELLIS.2/trellis_server.py`
   - use the custom Flask server

This is not part of the canonical baseline:

- a custom DINO compatibility patch in `trellis2/modules/image_feature_extractor.py`

That patch was only needed after the environment drifted to `transformers 5.5.4`.

## Invariants

These should remain true unless there is a deliberate workflow change:

- use `preprocess_image=True`
- use opaque green-background PNGs for baseline reproduction
- prefer pod-local `curl` plus `scp` for reliable transport
- keep weapons separate from character generation
- treat full-body generation as the primary current path

## Known-Good Generation Settings

These are not environment requirements, but they are the current proven operating settings:

- Hero full body:
  - input: [Arthur_HeroReference_Full_White.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_HeroReference_Full_White.png)
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 80000`
- Easy enemy family batch:
  - input style: clean front-view opaque green reference
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 200000`
- Sword raw:
  - input: [Arthur_ExcaliburProxy_FlatGreen_Tight.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Inputs/Arthur_ExcaliburProxy_FlatGreen_Tight.png)
  - seed: `1337`
  - `X-Texture-Size: 2048`
  - `X-Decimation: 200000`

## Current Post-TRELLIS Poly Targets

- Arthur low-poly body:
  - preferred: `40k` tris
  - workable but softer: `20k` tris
- Easy enemy bipeds:
  - `24k` tris
- Slime-like enemies:
  - `10k` tris
- Wide flying enemies:
  - `14k` tris

## Local Workstation Lock

- Blender: `5.1.1`
- Blender MCP: installed and working
- RetopoFlow: `4.1.5`
- Blender MCP recovery helper: [start_blender_mcp.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/start_blender_mcp.py)
- RetopoFlow rule: [RETOPOFLOW_4.md](C:/UE/T66/Model%20Generation/RETOPOFLOW_4.md)
- Blender batch QA helper: [blender_glb_qa.py](C:/UE/T66/Model%20Generation/Scripts/blender_glb_qa.py)
- If Blender MCP is disconnected, reopen Blender and rerun the MCP helper before assuming the toolchain is unavailable.

## If Results Degrade

Check these first:

1. `transformers` version
2. `image_feature_extractor.py` was not custom-patched
3. `BiRefNet.py` still points to `rembg`
4. `flow_euler.py` still contains the `cfg_strength` fix
5. input images are opaque green PNGs, not alpha cutouts
6. Blender-exported low-poly GLBs still survive a clean Blender re-import render
