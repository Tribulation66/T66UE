# Quad Retro Model Test 01

Date: 2026-05-06

## Purpose

Test the selected Boxer Chad and Boxer Stacy source images in TRELLIS before
running the Quad Remesher / retro texture pipeline.

## Selected Sources

- `Inputs/approved_seed_images/BoxerChad_ExaggeratedV_04_Source.png`
- `Inputs/approved_seed_images/BoxerStacy_ExaggeratedHourglass_03_Source.png`

## TRELLIS Settings

- `X-Seed`: `1337`
- `X-Texture-Size`: `2048`
- `X-Decimation`: `80000`
- Server: `http://127.0.0.1:8000/generate` on RunPod

## Raw Outputs

- `Raw/Trellis/BoxerChad_ExaggeratedV_04_S1337_D80000_Trellis2.glb`
  - raw triangles: `77656`
  - bounds size: `0.7286 x 0.2474 x 1.0018`
- `Raw/Trellis/BoxerStacy_ExaggeratedHourglass_03_S1337_D80000_Trellis2.glb`
  - raw triangles: `78740`
  - bounds size: `0.5624 x 0.2640 x 1.0012`

## Blender QA

Rendered front, right, and oblique views from the downloaded GLBs.

Status: both models pass the first raw TRELLIS visual gate. No rerun was needed.

- Chad: silhouette, gloves, boots, tank, and trunks read clearly. Strong V shape
  survived from the source.
- Stacy: hourglass silhouette, gloves, boots, top, trunks, and hair mass read
  clearly. No background card or missing limb issue observed.

Next step: run both raw GLBs through the Quad Retro Character Pipeline.
