# T66 World Generation Master

This is the authoritative internal exploration document for `T66` world generation as of `2026-04-19`.

Supporting files:

- [MEMORY.md](C:/UE/T66/World%20Generation/MEMORY.md)
- [USEFUL_LINKS.md](C:/UE/T66/World%20Generation/USEFUL_LINKS.md)
- local report cache: [HY_World_2_0.pdf](C:/UE/T66/tmp/worldgen_research/HY_World_2_0.pdf)
- local cloned repo: [HY-World-2.0_repo](C:/UE/T66/tmp/worldgen_research/HY-World-2.0_repo)

## Purpose

- Understand `HY-World 2.0` deeply enough to decide how it should fit into `T66`.
- Keep release reality, architecture, T66 fit, and next experiments in one place.
- Avoid future agents confusing `officially public now` with `described in the report but not released yet`.

## Release Reality

Official source-of-truth status on `April 16, 2026`:

- the technical report was released
- `WorldMirror 2.0` inference code and model weights were released
- the full world-generation inference stack was **not** released yet
- `HY-Pano 2.0`, `WorldNav`, and `WorldStereo 2.0` are still marked `Coming Soon` in the official repo

Current practical interpretation:

- **Public and runnable today:** `WorldMirror 2.0` world reconstruction
- **Public in design/report only:** the full 4-stage text/image -> navigable 3D world pipeline
- **Important inference from source comparison:** the `ZhihuFrontier` X thread describes the system as effectively full open-source, but the official GitHub README is more limited and should be treated as authoritative for implementation planning

Authority order for this folder:

1. official GitHub repo / README / documentation
2. official technical report
3. official license
4. user-provided discussion links and secondary summaries

## What HY-World 2.0 Actually Is

`HY-World 2.0` is an **offline 3D world model**. It is trying to generate or reconstruct persistent 3D worlds, not just videos.

Core input modes:

- text
- single-view image
- multi-view images
- video

Core output modes:

- panoramas
- point clouds
- depth maps
- normal maps
- camera parameters
- Gaussian splats / `3DGS`
- extracted meshes

The key conceptual claim is:

- video world models let you watch a world
- 3D world models let you keep, edit, and reuse a world

That difference matters for `T66`, because `Unreal` integration only becomes operationally useful when the output behaves like an asset pipeline instead of a disposable clip generator.

## The Four-Stage World Generation Pipeline

### Stage I: HY-Pano 2.0

Goal:

- turn text or a single image into a `360°` panorama that initializes the world

Important implementation ideas:

- trained on a hybrid of real panoramas and synthetic `Unreal Engine` data
- uses an implicit perspective-to-ERP mapping instead of relying on known camera intrinsics
- uses circular padding plus pixel blending to remove left/right seam artifacts

Why it matters:

- panorama quality sets the ceiling for everything downstream
- if the panorama is semantically wrong, the trajectory planner and expansion stages inherit the mistake

### Stage II: WorldNav

Goal:

- parse the initialized panorama and produce useful, collision-safe camera trajectories for expansion

Scene parsing ingredients named in the report:

- panoramic point cloud
- panoramic mesh
- semantic masks
- `NavMesh`

Referenced components:

- `MoGe2` for panoramic point-cloud initialization
- `Qwen3-VL` for identifying landmarks and obstacles
- `SAM3` for semantic masks
- `Recast Navigation` for traversable space

Trajectory families:

- `Regular`: broad expansion from the panorama origin
- `Surrounding`: circle around important foreground objects
- `Reconstruct-Aware`: target missing or under-observed regions
- `Wandering`: push toward farthest reachable space in corridors / streets
- `Aerial`: raised-pitch variants to eliminate remaining blind spots

Why it matters:

- this is not random camera wandering
- the planner is explicitly trying to maximize useful coverage, preserve collision validity, and improve later reconstruction quality

### Stage III: WorldStereo 2.0

Goal:

- generate consistent novel views along the planned trajectories

Important design choices:

- uses **keyframe generation**, not a long dense video as the primary latent space
- replaces a standard `Video-VAE` emphasis with a `Keyframe-VAE` approach to preserve more geometric fidelity
- supports explicit camera control via `Plucker rays` and point-cloud guidance
- uses two memory systems:
  - `GGM` (`Global-Geometric Memory`) for coarse global structure
  - `SSM++` (`Improved Spatial-Stereo Memory`) for local detail and cross-view consistency
- uses a 3-stage training sequence:
  - domain adaptation for camera-guided keyframes
  - memory training for consistency
  - `DMD` post-distillation into a faster `4-step DiT`

Why it matters:

- this is the real bridge between “one panorama” and “many useful views”
- it is also the least available part publicly right now, which is why we cannot yet treat official `HY-World 2.0` as a fully runnable local stack

### Stage IV: World Composition

Goal:

- convert the generated views into a final navigable 3D asset

Pipeline described in the report:

1. use `WorldMirror 2.0` on generated keyframes plus camera priors
2. align predicted depth against the original panoramic point cloud
3. expand the point cloud
4. initialize and optimize `3DGS`
5. extract a mesh for collision / physics / downstream engine use

Important implementation details:

- `WorldMirror 2.0` is used as the reconstruction backbone inside world generation
- depth alignment is linear and `RANSAC`-based, with outlier rejection across coefficient distributions
- `3DGS` optimization uses direct RGB color instead of spherical harmonics in this generative setting
- `MaskGaussian` is used to prune redundant gaussians and reduce floaters
- mesh extraction uses `TSDF` integration plus marching cubes

Why it matters:

- this is the stage that turns view synthesis into something engine-adjacent
- for `T66`, this is the difference between “pretty camera demo” and “something we can actually import, collide against, or use as reference geometry”

## WorldMirror 2.0: What We Can Use Right Now

Current public code in the official repo is centered on `WorldMirror 2.0`.

What it supports now:

- multi-view image or video -> reconstruction
- optional prior injection:
  - camera poses
  - intrinsics
  - depth maps
- outputs:
  - `points.ply`
  - `gaussians.ply`
  - `camera_params.json`
  - depth maps
  - normal maps
  - optional rendered fly-through video
  - optional `COLMAP` sparse output

Operational details from the released code/docs:

- Python `3.10`
- recommended `CUDA 12.4`
- `torch==2.4.0`, `torchvision==0.19.0`
- FlashAttention is recommended
- single-GPU inference is supported
- multi-GPU inference uses `torchrun`, `FSDP`, `BF16`, and sequence parallelism
- in multi-GPU mode, the number of input images must be at least the number of GPUs

Key model facts:

- `WorldMirror-2` is listed at about `1.2B` parameters
- the open repo includes a CLI pipeline and a Gradio app
- the open repo’s `hyworld2/worldgen/README.md` explicitly says the world-generation module is **not** open-sourced yet

## Performance And Practicality

Numbers explicitly reported in the paper for full world generation on `NVIDIA H20` GPUs:

- panorama: `15s`
- trajectory planning: `182s`
- world expansion: `286s`
- reconstruction + alignment: `102s`
- `3DGS`: `127s`
- total: `712s` (`~10 minutes`)

Important interpretation:

- these are **paper/runtime numbers on server-class GPUs**
- this is not evidence that the full pipeline is lightweight on consumer hardware
- today we cannot validate end-to-end local cost anyway, because the official full generator is not public yet

Useful practical contrast from the paper:

- their lightweight linear alignment is reported as `< 2 minutes`
- they describe `video2world` as around `5 hours per scene` under their comparison setup

WorldMirror-only practicality is stronger than full-pipeline practicality:

- the public code can run in single-GPU mode
- the released model is concrete and inspectable
- this makes `WorldMirror 2.0` the best short-term entry point for `T66`

## T66 Fit

### Strong Fits

- offline generation of explorable environment prototypes
- menu / hub / shrine / promo scene creation
- reconstruction of real or authored spaces from video / multi-view captures
- scenic world ideation before manual Unreal rebuild
- generating reference geometry and lighting context for future authored spaces

### Weak Fits

- replacing deterministic runtime procedural map generation
- generating gameplay-critical combat arenas at runtime
- shipping collision-authoritative multiplayer maps with no cleanup pass
- committing the project to unreleased Tencent modules

### Why This Should Not Replace Current Runtime Terrain

`T66` already has its own runtime procedural world stack:

- `AT66GameMode` currently owns major procedural map-generation orchestration
- `T66ProceduralLandscapeGenerator`
- `T66MainMapTerrain`
- `T66TowerMapTerrain`
- older terrain experiments in [SourceAssets/MegabonkMapGeneration](C:/UE/T66/SourceAssets/MegabonkMapGeneration)

That means the current best framing is:

- `HY-World 2.0` is an **offline content-generation / reconstruction pipeline**
- `T66`’s current procedural stage assembly is a **runtime gameplay system**

Those are adjacent, but not the same job.

## Recommended T66 Adoption Path

### Phase 1: Research Baseline

- keep this folder current as Tencent releases more of the stack
- treat the official report and repo as the only hard source of release status

### Phase 2: Immediate Hands-On Candidate

- run `WorldMirror 2.0` against:
  - controlled multi-view screenshots from Unreal blockouts
  - phone / editor video captures of spaces we want to reconstruct
  - synthetic benchmark image sets

Goal:

- learn how cleanly its point clouds / gaussians / cameras can be turned into useful `T66` references

### Phase 3: Pre-Release Stopgap If Needed

- if we need world-generation experiments before Tencent opens the full pipeline, treat these as temporary research branches only:
  - `HunyuanWorld 1.0`
  - `WorldStereo` preview repo

Do not treat those as production anchors unless they survive direct Unreal import and cleanup testing.

### Phase 4: Full HY-World 2.0 Re-evaluation

Re-evaluate only after Tencent releases:

- official `HY-Pano 2.0`
- official `WorldNav`
- official `WorldStereo 2.0`
- official end-to-end world generation inference code

At that point, validate:

- import quality into Unreal
- mesh cleanup cost
- collision usefulness
- memory/runtime cost
- legal fit for our distribution regions and product scale

## Legal And Licensing Notes

The repo ships under the `Tencent HY-WORLD 2.0 Community License Agreement`.

Important restrictions to remember:

- the license explicitly does **not** apply in the `European Union`, `United Kingdom`, and `South Korea`
- usage outside that territory is unlicensed
- if the licensee’s products/services exceed `1 million monthly active users`, Tencent approval is required
- the license says outputs/results must not be used to improve another AI model
- there is an attached acceptable-use policy with additional restrictions

This means legal review is required before any serious production dependency, especially if `T66` distribution plans expand beyond the currently allowed territory.

## Current Bottom Line

As of `2026-04-19`, the correct T66 position is:

- **We should track HY-World 2.0 seriously.**
- **We should not pretend the official full world generator is public yet.**
- **The best immediate technical entry point is WorldMirror 2.0 reconstruction.**
- **For T66, the likely role is offline world asset generation / reconstruction, not runtime stage generation.**

## Authority Rule

When future summaries, tweets, mirrors, or discussion posts disagree with the official repo or license:

- trust the official repo first
- trust the technical report second
- update this file when Tencent’s actual public release status changes
