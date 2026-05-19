# Phase 1C Pixal3D Lineup Report

Date: 2026-05-17

## Scope

This report covers Phase 1C Track D Part 1: Pixal3D raw GLB generation from the QA-passed isolated PNGs, plus the explicit decision to retain the older Slime raw model as a comparison asset.

Part 2 has now run. K-means flattening, Blender pipeline, UE import, material changes, and test-room changes are documented in `Phase1C_TextureFlattening_Report.md` and `Phase1C_Pipeline_And_Materials_Report.md`.

## RunPod Setup

The original RunPod was closed, and a fresh PIXAL3D pod was configured.

- SSH: `ssh root@213.173.109.175 -p 13721 -i ~/.ssh/id_ed25519`
- Pod name shown by RunPod: `PIXAL3D`
- Visible hardware from `nvidia-smi`: two NVIDIA GeForce RTX 4090 devices, each about 24 GiB VRAM
- Server health in normal mode: one visible RTX 4090, `low_vram=false`, about 25.3 GB total VRAM, about 20.2 GB allocated by the server
- Server health in low-vram mode: one visible RTX 4090, `low_vram=true`, about 25.3 GB total VRAM, about 1.3 GB allocated before generation
- Endpoint: `http://127.0.0.1:18001/generate`
- Bootstrap log: `Saved/Codex/ToonStyle/Phase1C/NewPodSetup/bootstrap_pixal3d_4090.log`
- Hugging Face login log: `Saved/Codex/ToonStyle/Phase1C/NewPodSetup/hf_login_pixal3d_4090.log`

The pod had two 4090 GPUs, but the Pixal3D server process used one CUDA device at a time. Slime was retried on GPU 0 and GPU 1; both failed at original settings. Pablo accepted retaining the old Slime model from Phase 1A instead of spending more RunPod time on Slime in this pass.

## Gates

- G2: Passed. Lu Bu generated successfully at the original Phase 1C settings and produced a valid nonzero GLB.
- G3: Passed under the amended asset rule. Ten new Phase 1C raw GLBs exist, and Slime is intentionally retained from Phase 1A as a comparator at `SourceAssets/ToonStyle/Pixal3D/Phase1C/Slime/Raw/slime.glb`.

Phase 1C Part 1 is complete under this amended rule. Part 2 treated Slime as a retained legacy comparator, not as a newly regenerated Phase 1C model.

## Final Raw GLB Inventory

| Asset | Settings | Final Raw GLB | Bytes | Result |
| --- | --- | --- | ---: | --- |
| Lu Bu | R1536 T4096, default sampling, remesh export | `SourceAssets/ToonStyle/Pixal3D/Phase1C/LuBu/Raw/lubu.glb` | 12169892 | Success |
| ARIA | R1536 T4096, default sampling, remesh export, fresh 4090 low-vram | `SourceAssets/ToonStyle/Pixal3D/Phase1C/ARIA/Raw/aria.glb` | 9734276 | Success |
| Gambler | R1536 T4096, default sampling, remesh export | `SourceAssets/ToonStyle/Pixal3D/Phase1C/Gambler/Raw/gambler.glb` | 11243464 | Success |
| Slime | Retained Phase 1A raw GLB after Phase 1C retries failed at original settings | `SourceAssets/ToonStyle/Pixal3D/Phase1C/Slime/Raw/slime.glb` | 11508280 | Retained comparator |
| TombSpider | R1536 T4096, default sampling, remesh export after clean retry | `SourceAssets/ToonStyle/Pixal3D/Phase1C/TombSpider/Raw/tombspider.glb` | 11245032 | Success |
| CaveBat | R1536 T4096, default sampling, remesh export, fresh 4090 low-vram | `SourceAssets/ToonStyle/Pixal3D/Phase1C/CaveBat/Raw/cavebat.glb` | 8580100 | Success |
| Idol Altar | R1536 T4096, default sampling, remesh export | `SourceAssets/ToonStyle/Pixal3D/Phase1C/IdolAltar/Raw/idolaltar.glb` | 11318868 | Success |
| Arcade Machine | R1536 T4096, default sampling, remesh export | `SourceAssets/ToonStyle/Pixal3D/Phase1C/ArcadeMachine/Raw/arcademachine.glb` | 10060668 | Success |
| Loot Chest | R1536 T4096, default sampling, explicit no-remesh export fallback | `SourceAssets/ToonStyle/Pixal3D/Phase1C/LootChest/Raw/lootchest.glb` | 13564536 | Success |
| Loot Bag Yellow | R1024 T4096 documented exception, default sampling, remesh export | `SourceAssets/ToonStyle/Pixal3D/Phase1C/LootBagYellow/Raw/lootbag_yellow.glb` | 10016180 | Success |
| Loot Crate | R1536 T4096, default sampling, explicit no-remesh export fallback, fresh 4090 low-vram | `SourceAssets/ToonStyle/Pixal3D/Phase1C/LootCrate/Raw/lootcrate.glb` | 11558220 | Success |

## Generation Attempts

| Asset / Run | Result | Duration | Notes |
| --- | --- | ---: | --- |
| Lu Bu | Success | 118.1s | First Pixal3D smoke and G2 pass. |
| Gambler | Success | 161.3s | Amended demon dealer prompt. |
| Slime old pod first pass | Failed | 260.9s | CuMesh fill_holes OOM, `utils.h` line 42. |
| Slime old pod clean retry | Failed | 217.0s | CuMesh fill_holes OOM, `utils.h` line 42. |
| Slime fresh 4090 normal mode | Failed | 60.5s | CUDA OOM during NAF image conditioning before fill_holes. |
| Slime fresh 4090 low-vram GPU0 | Failed | 192.6s | CuMesh fill_holes OOM, `connectivity.cu` line 419. |
| Slime fresh 4090 low-vram GPU1 | Failed | 143.8s | CuMesh fill_holes OOM, `utils.h` line 42. |
| Slime retained comparator | Success | n/a | Copied from `SourceAssets/ToonStyle/Pixal3D/Phase1A/Slime/Raw/slime.glb` into the Phase 1C raw slot after Pablo approved retaining the old Slime for comparison. |
| TombSpider first pass | Failed | 0.8s | Dirty-state CUDA OOM immediately after Slime failure. |
| TombSpider clean retry | Success | 113.9s | Clean server state fixed the dirty-state failure. |
| Idol Altar | Success | 172.3s | Original settings. |
| Arcade Machine | Success | 177.4s | Original settings. |
| Loot Chest remesh export | Failed | 244.2s | GLB export attempts failed after generation. |
| Loot Chest no-remesh export | Success | 228.4s | Explicit export fallback, no geometry-generation fallback. |
| Loot Bag Yellow R1024 T4096 | Success | 93.6s | First diagnostic ladder rung succeeded. |
| Loot Crate old pod remesh export | Failed | 263.8s | GLB export attempts failed after generation. |
| Loot Crate fresh 4090 remesh export | Failed | 225.2s | GLB export attempts failed after generation. |
| Loot Crate fresh 4090 no-remesh export | Success | 190.1s | Explicit export fallback. |
| CaveBat fresh 4090 dirty attempt | Failed | 0.5s | Dirty-state CUDA OOM after Slime failure. |
| CaveBat fresh 4090 clean low-vram | Success | 146.7s | Original settings. |
| ARIA fresh 4090 clean low-vram | Success | 86.8s | Original settings. |

## Evidence Paths

- Old-pod batch logs: `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/Logs/`
- Old-pod retry logs: `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/Retries/`
- Fresh-pod logs: `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/NewPod/`

The most relevant Slime failure logs are:

- `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/Logs/03_slime/Logs/slime_error.txt`
- `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/Retries/03_slime_retry01/Logs/slime_error.txt`
- `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/NewPod/03_slime_original/Logs/slime_error.txt`
- `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/NewPod/03_slime_original_lowvram/Logs/slime_error.txt`
- `SourceAssets/ToonStyle/Pixal3D/Phase1C/LineupBatch/NewPod/03_slime_original_lowvram_gpu1/Logs/slime_error.txt`

## Slime Decision

Slime is no longer a Phase 1C Part 1 blocker. The new Phase 1C Slime source still appears to trigger Pixal3D/CuMesh memory failures at original settings, but Pablo decided to retain the old Slime model for this pass. That gives the next visual evaluation a useful A/B point: ten corrected Phase 1C assets against one older Slime model.

Part 2 processing outcome:

1. K-means flattening and the Blender/UE pipeline ran on all eleven raw slots.
2. Slime is marked `retained_from_phase1a=true` in the Phase 1C manifests and reports.
3. Slime used the same k=6 texture flattening and Blender/UE pipeline as the other creature assets so the final comparison isolates upstream image/source quality rather than post-processing.
4. In the final test-room review, compare Slime specifically against the ten regenerated assets to decide whether a later Slime-specific prompt/resolution pass is worth doing.
