# CoherentThemeKit01 Decision Log

## 2026-04-30

- Corrected the Martian theme target to `VeryHard`, matching the current runtime difficulty mapping.
- Generated eight coherent source sheets with imagegen:
  - dungeon walls and floors
  - forest walls and floors
  - Martian / VeryHard walls and floors
  - hell walls and floors
- Saved the source sheets under `Inputs/source_sheets`.
- Split the source sheets into `32` individual TRELLIS seed PNGs under `Inputs/approved_seed_images`.
- Mirrored the run folder to RunPod pod `69.30.85.6:22054`.
- Ran the full TRELLIS batch on the pod with:
  - seed: `1337`
  - texture size: `2048`
  - decimation: `80000`
  - endpoint: `http://127.0.0.1:8000/generate`
- Generated all `32 / 32` raw GLB outputs successfully.
- Downloaded all raw GLBs and TRELLIS batch logs back to the local run folder.
- Local verification result:
  - expected GLBs: `32`
  - actual GLBs: `32`
  - missing GLBs: `0`
  - total raw GLB bytes: `140706852`

## 2026-04-30 Ocean / Hard Addendum

- Added the missing Hard / Ocean water-level kit.
- Generated two additional coherent source sheets with imagegen:
  - `Ocean_WallSheet.png`: coral reef inspired wall modules
  - `Ocean_FloorSheet.png`: reef stone, shell sand, coral crack, and tide-pool floor modules
- Split the Ocean sheets into eight TRELLIS seed PNGs under `Inputs/approved_seed_images`.
- Ran the Ocean-only TRELLIS batch on the same RunPod with:
  - seed: `1337`
  - texture size: `2048`
  - decimation: `80000`
  - endpoint: `http://127.0.0.1:8000/generate`
- Generated all `8 / 8` Ocean raw GLB outputs successfully.
- Downloaded the Ocean raw GLBs and Ocean batch logs back to the local run folder.
- Updated full local verification result:
  - expected GLBs: `40`
  - actual GLBs: `40`
  - missing GLBs: `0`

Raw outputs are in:

```text
C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01\Raw\Trellis
```

Generation logs are in:

```text
C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01\Notes\trellis_batch.log
C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01\Notes\trellis_batch.stdout
```

## 2026-04-30 Floor Regeneration Addendum

- The original top-down floor seed images produced disconnected or plane-like TRELLIS meshes.
- Built `FloorFix01` seed images as low-angle thick slabs with visible top, front, and side faces.
- Ran all `20 / 20` floor modules through TRELLIS again into `Raw/TrellisFloorFix01`.
- Blender MCP welded-component QA showed five floor meshes still had multiple meaningful components:
  - `DungeonFloor_StoneSlabs_A`
  - `DungeonFloor_Drain_A`
  - `OceanFloor_ShellSand_A`
  - `MartianFloor_CraterCracks_A`
  - `HellFloor_RunePlate_A`
- Built stronger continuous-underlay `FloorFix02` seed images for those five and regenerated them into `Raw/TrellisFloorFix02`.
- Blender MCP QA then showed all floors connected, but two Hell floors remained full-height blocks:
  - `HellFloor_Obsidian_A`
  - `HellFloor_EmberFissure_A`
- Built `FloorFix03` seed images for those two height outliers and regenerated them into `Raw/TrellisFloorFix03`.
- Updated the comparison scene builder to prefer:
  - `TrellisFloorFix03`
  - `TrellisFloorFix02`
  - `TrellisFloorFix01`
  - original `Raw/Trellis`
- Rebuilt the open comparison scene through the official Blender Lab MCP connection.
- Final Blender MCP QA result:
  - floor count: `20`
  - weld tolerance: `0.01`
  - disconnected floor count: `0`
  - height outlier count above `0.45`: `0`
  - final floor height range: `0.1783` to `0.3736`

Updated comparison files:

```text
C:\UE\T66\Model Generation\Scenes\CoherentThemeKit01_SizeCompare.blend
C:\UE\T66\Model Generation\Scenes\CoherentThemeKit01_SizeCompare.png
C:\UE\T66\Model Generation\Scenes\CoherentThemeKit01_SizeCompare_Report.json
```
