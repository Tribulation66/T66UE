# T66 Tower Terrain Fix - Iteration 01

Date: 2026-05-14  
Status: Implemented, built, staged, and screenshot-verified from the staged standalone build.

## Changes Applied

| Fix | File / location | Before | After |
|---|---|---|---|
| Wall-to-ceiling gap | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:643-763` | Generated wall visuals emitted one native-height wall instance per segment, leaving a black band below the ceiling when meshes were shorter than 1200 UU. | Each generated wall segment now emits two instances of the same wall mesh. Each instance is Z-scaled to `DesiredHeight * 0.5f` (600 UU for the current 1200 UU wall span), with the top instance grounded at `BaseZ + 600`. The hidden collision proxy remains a single 1200 UU proxy. |
| Wall-to-wall segment selection | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:607-625`, `Source/T66/Gameplay/T66TowerMapTerrain.cpp:706` | Mesh selection used `Seed + SideIndex * 977 + SegmentIndex * 37`, so adjacent segments in a run could use different modules. | Mesh selection now uses `Seed + SideIndex * 977`. The segment term is gone, so a wall run uses one pinned module across every segment and both stacked halves. |
| Floor seams and microscopic floor | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:3976-4116` | Each walkable source box was subdivided into generated floor tiles by target tile size, creating internal mesh seams and small repeated floor detail. | Each valid floor surface rectangle now emits exactly one generated floor mesh scaled to the full rectangle. Collision slabs are unchanged. Drop-hole carve-outs still emit separate rectangles to preserve holes. |
| Ceiling visual consistency | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4135-4279` | Ceiling undersides used the same subdivision approach as floors. | Ceiling undersides now follow the same one-mesh-per-surface-rectangle pattern as floors, so the visible ceiling uses the same no-internal-subdivision rule. |
| Generated-kit doorway headers | `Source/T66/Gameplay/T66TowerMapTerrain.cpp:4690-4847` | The generated-kit branch returned before spawning doorway headers. | Generated-kit doorway headers now spawn as HISM wall mesh instances, scaled to each header box and top-flush with the 1200 UU wall span. The legacy cube fallback path is unchanged. |
| Obsolete generated floor tile CVar | `Source/T66/Gameplay/T66TowerMapTerrain.cpp` top-level CVar block | `T66.Tower.GeneratedKitFloorVisualTileSize` controlled floor/ceiling subdivision size. | Removed because generated floor and ceiling visuals no longer subdivide by a target tile size. |

## Implementation Notes

- Wall stack height uses the existing `DesiredHeight` parameter, so the current 1200 UU wall span becomes two 600 UU visual pieces without changing collision dimensions.
- Wall X and Y scale behavior stays aligned with the previous generated-kit path: X scale remains the wall module depth behavior, Y scale still spans the segment length, and only Z scale is newly derived from the target 600 UU half-height.
- Floor and ceiling HISM batching remains in place. The change reduces per-floor visual instance counts by removing intra-box subdivision rather than switching back to individual actors.
- Doorway headers use deterministic per-header wall mesh selection because `Floor.DoorwayHeaderBoxes` currently stores only `FBox2D`, not the original wall-run metadata. The limitation is tracked in pending issues.

## Verification Screenshots

Screenshots were captured from:

`C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

Capture path used `/Game/Maps/GameplayLevel` with no `?listen`, windowed 1920x1080 output, and `-T66GameplayAutoScreenshot`.

### Floor 1 Gallery

Path: `Saved/TerrainFix/Iteration_01/gallery_floor1_overview.png`

![Floor 1 gallery overview](../../../Saved/TerrainFix/Iteration_01/gallery_floor1_overview.png)

### Stage 1 Wall / Ceiling / Floor

Path: `Saved/TerrainFix/Iteration_01/stage1_wall_ceiling_floor.png`

![Stage 1 wall ceiling floor](../../../Saved/TerrainFix/Iteration_01/stage1_wall_ceiling_floor.png)

Log evidence:

- `Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:624` - floor 1 spawned with `generatedKit=1`, `HISM instances +309`, `walkable boxes=9`.
- `Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:629` - full tower terrain spawned across 5 floors with `generatedKit=1`.
- `Saved/StandaloneLogs/TerrainFix_Iteration01_Stage1.log:934-936` - active floor changed to floor 2 and initial enemies spawned, proving the capture entered the gameplay board.

### Doorway Closeup

Path: `Saved/TerrainFix/Iteration_01/doorway_closeup.png`

![Doorway closeup](../../../Saved/TerrainFix/Iteration_01/doorway_closeup.png)

Log evidence:

- `Saved/StandaloneLogs/TerrainFix_Iteration01_Doorway.log:623` - floor 1 spawned with generated kit enabled.
- `Saved/StandaloneLogs/TerrainFix_Iteration01_Doorway.log:628` - full tower terrain spawned across 5 floors with generated kit enabled.
- `Saved/StandaloneLogs/TerrainFix_Iteration01_Doorway.log:932-933` - active floor changed to floor 2 and initial enemies spawned.

## Build And Stage Status

Command:

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\StageStandaloneBuild.ps1" -ClientConfig Development -ResetSavedGames
```

Result: passed.

- UBT compiled `T66TowerMapTerrain.cpp` for `T66Editor Win64 Development` and `T66 Win64 Development`.
- BuildCookRun completed with `BUILD SUCCESSFUL` and `ExitCode=0`.
- Staged executable verified present: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Project shortcut verified target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Pinned taskbar shortcut verified target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Shortcut arguments preserved: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`.

## Unexpected Findings

- Full `git diff --check` still fails because of pre-existing trailing whitespace in `Gameplay/Movement/MASTER_MOVEMENT.md:3-4`. The scoped diff check for edited files passed.
- The screenshots can still show joins between separate walkable rectangles and authored wall-module joins. Those are not the same as the removed internal floor subdivision or the previous wall-to-ceiling height gap, and they are tracked below for the next pass.

## Pending Issues Created

`Source/T66/Gameplay/pending_issues_Gameplay.md`

- `Source/T66/Gameplay/pending_issues_Gameplay.md:24` - generated wall stack has a mid-height visual join at Z=600 because wall textures/modules are not authored as vertical pairs.
- `Source/T66/Gameplay/pending_issues_Gameplay.md:31` - inter-walkable-box floor seams remain possible where separate visual rectangles meet.
- `Source/T66/Gameplay/pending_issues_Gameplay.md:38` - doorway headers lack source wall-run metadata, so exact surrounding-wall mesh selection cannot be guaranteed from the current `FBox2D`-only header data.
