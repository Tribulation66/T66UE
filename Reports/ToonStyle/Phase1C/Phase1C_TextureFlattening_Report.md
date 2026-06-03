# Phase 1C Texture Flattening Report

Date: 2026-05-17

## Scope

This report covers Phase 1C Track C: K-means-style diffuse flattening integrated into the Blender pipeline. The step ran on all eleven final lineup slots, including the retained Phase 1A Slime.

Slime is intentionally marked `retained_from_phase1a=true`: it did not come from a successful Phase 1C Pixal3D generation, but it did go through the same texture flattening and Blender/UE import pipeline as the ten regenerated assets. That keeps the final comparison honest: new corrected source through the new pipeline versus old uncorrected source through the same post-processing.

## Implementation

- Script: `ToonStyle/BlenderScripts/flatten_diffuse_texture.py`
- Pipeline integration: `ToonStyle/BlenderScripts/run_toon_pipeline.py`
- Output evidence: `Saved/Codex/ToonStyle/Phase1C/all_pipeline_summary.json`

The original prompt asked for `sklearn.cluster.MiniBatchKMeans`, but Blender's Python environment did not have scikit-learn available. I used a deterministic numpy-only clustering implementation instead so the step runs inside the same Blender session as the rest of the pipeline. It still logs the convergence iteration count per texture.

## Gate G4

G4 passed on Lu Bu before bulk processing.

- Working directory: `SourceAssets/ToonStyle/Pixal3D/Phase1C/LuBu/Working/lubu_validation/`
- Primary diffuse colors: `244782 -> 8`
- Secondary texture colors: `622 -> 7`
- Iterations: `22`, `5`
- Major red/gold/black/skin regions remained represented after flattening.

## Per-Asset Results

| Asset | Retained | k | Unique colors before -> after | Iterations | Notes |
| --- | --- | ---: | --- | --- | --- |
| Lu Bu | no | 8 | `244782/622 -> 8/7` | `22/5` | G4 validation asset. |
| ARIA | no | 8 | `101414/176 -> 8/7` | `24/5` | Humanoid k=8. |
| Gambler | no | 8 | `114121/494 -> 8/8` | `14/5` | Demon dealer + table composition. |
| Slime | yes | 6 | `56582/17180 -> 6/6` | `20/15` | Retained Phase 1A comparator, same post-processing. |
| TombSpider | no | 6 | `39644/2355 -> 6/6` | `24/6` | Creature k=6. |
| CaveBat | no | 6 | `102320/384 -> 6/5` | `10/5` | Creature k=6. |
| Idol Altar | no | 6 | `127916/2208 -> 6/6` | `18/17` | Prop k=6. |
| Arcade Machine | no | 6 | `575951/26520 -> 6/6` | `19/10` | Prop k=6. |
| Loot Chest | no | 4 | `235468/4864 -> 4/4` | `24/5` | Pixal3D no-remesh fallback raw GLB. |
| Loot Bag Yellow | no | 4 | `105539/1654 -> 4/4` | `10/4` | R1024 Pixal3D exception. |
| Loot Crate | no | 4 | `58651/2974 -> 4/4` | `16/4` | Pixal3D no-remesh fallback raw GLB. |

## Outcome

All eleven lineup slots produced flattened texture outputs and were consumed by the Blender/UE pipeline. No asset skipped flattening. No per-asset k override beyond the approved category defaults was required.
