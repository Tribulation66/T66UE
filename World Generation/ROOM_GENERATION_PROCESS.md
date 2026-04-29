# T66 Room Generation Process

This is the current process for the first `T66` generated-room experiment.

Prototype target:

- Generate a dungeon-room visual seed with imagegen.
- Use HY-World / WorldMirror paths on RunPod to produce room reconstruction assets.
- Bring the result through Blender QA and Unreal import.
- Change the initial runtime test so the player starts in the generated room, clicks a portal, and lands at the current normal tower start spawn.

## Hard Rules

- Keep generated-room work offline and asset-pipeline based until a room is proven usable.
- Do not replace `T66TowerMapTerrain` during the first prototype.
- Do not claim single-image HY-World room generation is available from the public repo until Tencent releases the full world-generation inference code.
- Keep raw pod outputs, Blender conversions, QA renders, and Unreal imports together in the run folder.
- Generated room collision must be tested separately from visual quality.
- The first gameplay test must be reversible behind a dev flag or compile-time branch.

## Current T66 Runtime Seams

Current flow:

1. `UT66GameInstance::TransitionToGameplayLevel()` opens `GameplayLevel`.
2. `AT66GameMode::EnsureGameplayStartupInitialized()` schedules deferred gameplay spawn.
3. `AT66GameMode::SpawnLevelContentAfterLandscapeReady()` calls `SpawnMainMapTerrain()`.
4. `SpawnMainMapTerrain()` builds the active tower layout through `T66TowerMapTerrain::BuildLayout(...)`.
5. The tower path caches:
   - `MainMapSpawnSurfaceLocation`
   - `MainMapStartAnchorSurfaceLocation`
   - `MainMapStartPathSurfaceLocation`
   - `MainMapStartAreaCenterSurfaceLocation`
   - boss and rescue anchors
6. Once tower collision is ready, `RestartPlayersMissingPawns()` spawns missing pawns.
7. Tower combat starts only after the player reaches a gameplay floor.
8. Tower floor traversal currently uses `AT66TowerDescentHole`.
9. Stage-to-stage travel uses `AT66StageGate`.

Best first integration:

- Add a generated-room entry layer after normal terrain has built and cached its real spawn location.
- Cache the normal `MainMapSpawnSurfaceLocation` as the portal destination.
- Temporarily override the player start/spawn location to the generated room's entry anchor.
- Spawn a click-to-use portal inside the generated room that teleports the pawn to the cached normal start location.

This preserves the existing tower generator, start floor, combat-start rules, minimap behavior, and stage progression while letting us test a generated room at run start.

## Run Folder Contract

Use this folder shape for the first dungeon room:

```text
C:\UE\T66\World Generation\Runs\DungeonRoom01\
  Inputs\
    imagegen_prompt.txt
    seed_image.png
    input_views\
  PodOutput\
  BlenderQA\
    qa_render_front.png
    qa_render_top.png
    scale_notes.md
  UnrealImport\
    DungeonRoom01_visual.glb
    DungeonRoom01_collision.glb
    import_notes.md
  Notes\
    decision_log.md
```

Do not scatter dungeon-room images or meshes under `tmp`, `Saved`, or `SourceAssets/Import` until a candidate is accepted.

## Step 1: Generate The Dungeon Room Image

The first imagegen target should be a clear playable room, not concept art that hides structure.

Prompt requirements:

- one compact square dungeon room
- readable floor plane
- clear wall/floor boundary
- one obvious portal alcove or doorway
- no player character
- no enemies
- no text or UI
- no extreme fisheye
- no darkness that hides collision-relevant edges
- stylized fantasy dungeon compatible with T66's current dungeon/tower theme

Starter prompt:

```text
Stylized fantasy dungeon room for an action roguelite game, square stone chamber, clear flat walkable floor, chunky stone walls, one glowing green exit portal alcove on the far wall, readable wall-floor corners, simple torch props, no characters, no text, no UI, game-ready environment reference, front-facing wide angle, bright enough to see collision boundaries.
```

Save the accepted image as:

```text
World Generation/Runs/DungeonRoom01/Inputs/seed_image.png
```

## Step 2: Decide Which HY-World Path Is Actually Available

Use this decision gate before running the pod.

### Path A: Full HY-World worldgen is public or product/API access is available

Use the generated `seed_image.png` as the single-image room seed.

Expected target:

- generated room mesh or 3DGS output
- camera/render preview
- exportable geometry for Blender inspection

Record exact command, product export path, or API request in:

```text
World Generation/Runs/DungeonRoom01/Notes/decision_log.md
```

### Path B: Public repo only

Use `WorldMirror 2.0`, which reconstructs from multi-view images or video.

Because `WorldMirror 2.0` is reconstruction, not true single-image world generation, prepare either:

- a short video of a controlled room mockup, or
- multiple consistent room views

For imagegen-only experiments, generate a small view set from the same room description and label it experimental:

```text
input_views/
  view_000_front.png
  view_001_front_left.png
  view_002_left.png
  view_003_back_left.png
  view_004_back.png
  view_005_back_right.png
  view_006_right.png
  view_007_front_right.png
```

This may fail consistency checks. If it fails, the failure is useful evidence, not a reason to hand-edit undocumented steps.

## Step 3: Run HY-World On RunPod

Follow [SETUP.md](C:/UE/T66/World%20Generation/SETUP.md) for pod bootstrap.

Public WorldMirror CLI shape:

```bash
conda activate hyworld2
cd /workspace/HY-World-2.0

python -m hyworld2.worldrecon.pipeline \
  --input_path /workspace/t66_worldgen/DungeonRoom01/input_views \
  --output_path /workspace/t66_worldgen/DungeonRoom01/out \
  --target_size 952 \
  --save_rendered \
  --no_interactive
```

If multi-GPU is required:

```bash
torchrun --nproc_per_node=<GPU_COUNT> -m hyworld2.worldrecon.pipeline \
  --input_path /workspace/t66_worldgen/DungeonRoom01/input_views \
  --output_path /workspace/t66_worldgen/DungeonRoom01/out \
  --use_fsdp \
  --enable_bf16 \
  --save_rendered \
  --no_interactive
```

Input-view count must be at least `<GPU_COUNT>`.

Download the output into:

```text
World Generation/Runs/DungeonRoom01/PodOutput
```

## Step 4: Pod Output Acceptance Gate

Accept the pod run for Blender QA only if it produces at least one useful 3D artifact:

- `points.ply`
- `gaussians.ply`
- generated mesh from full worldgen/product export
- camera/depth outputs that can support a later mesh extraction pass

Reject or rerun if:

- the floor is missing or tilted beyond easy repair
- walls collapse into floaters
- the room has no readable portal/exit anchor
- scale is impossible to infer
- outputs are only pretty renders with no usable 3D artifact

Log the decision in:

```text
World Generation/Runs/DungeonRoom01/Notes/decision_log.md
```

## Step 5: Blender QA And Conversion

Use Blender for deterministic inspection before Unreal import.

Minimum QA:

- import `points.ply`, `gaussians.ply`, or mesh export
- orient the room so floor is `XY` and up is `Z`
- set approximate T66 scale:
  - playable floor width target: `5000-9000` Unreal units
  - portal interaction clearance: at least one hero capsule plus camera room
  - wall height target: at least `700-1200` Unreal units
- create or verify:
  - visual mesh
  - simplified collision floor
  - simplified collision walls
  - portal anchor transform
  - player spawn transform
  - exit destination note

Export candidates:

```text
World Generation/Runs/DungeonRoom01/UnrealImport/DungeonRoom01_visual.glb
World Generation/Runs/DungeonRoom01/UnrealImport/DungeonRoom01_collision.glb
```

Save QA renders:

```text
World Generation/Runs/DungeonRoom01/BlenderQA/qa_render_front.png
World Generation/Runs/DungeonRoom01/BlenderQA/qa_render_top.png
```

## Step 6: Unreal Import

Only after Blender QA passes:

1. Copy accepted GLBs to:

```text
SourceAssets/Import/WorldGen/DungeonRoom01/
```

2. Import through the project static-mesh import workflow.
3. Target Unreal content path:

```text
/Game/World/Generated/DungeonRoom01/
```

4. Keep visual mesh and collision proxy separate if the visual mesh is complex.
5. Verify material parent and texture binding. World GLB imports should follow the current project import rules and end up on approved unlit world material paths.

## Step 7: First Runtime Prototype Design

Add a dev-only generated-room entry mode.

Recommended code shape:

- Add a small feature flag such as `bUseGeneratedRoomEntryPrototype`.
- After `SpawnMainMapTerrain()` builds the normal tower and caches `MainMapSpawnSurfaceLocation`, store:

```text
NormalTowerStartDestination = MainMapSpawnSurfaceLocation
```

- Spawn the generated room at a safe isolated world offset.
- Set the runtime player spawn point to the generated room's player anchor.
- Spawn a new explicit-destination portal actor in the generated room.
- On interaction, teleport the player to `NormalTowerStartDestination`.

Do not reuse `AT66TeleportPadInteractable` directly for this first test because it chooses a random destination pad. Either add explicit-destination support behind a new mode or create a small `AT66GeneratedRoomPortal` actor.

Combat and pacing requirements:

- timer remains frozen in generated room
- enemy director remains inactive in generated room
- miasma remains inactive in generated room
- portal use lands at the existing normal tower start
- after the player proceeds into gameplay floors, current tower combat activation remains unchanged

## Step 8: Runtime Acceptance Gate

The first prototype is accepted only if:

- player starts inside the generated dungeon room
- player cannot fall through the generated room floor
- walls block movement or the room has explicit blockers
- portal prompt appears and is clickable/usable
- portal teleports to the normal tower start spawn
- no combat starts before portal use
- after portal use, current tower flow still works
- restarting the run does not leave stale generated-room actors
- packaged Development standalone test still behaves correctly before any Steam-facing handoff

## Current RunPod Readiness

We are ready to spin up a pod when the user provides the connection details listed in [SETUP.md](C:/UE/T66/World%20Generation/SETUP.md).

First pod goal:

1. install HY-World public repo
2. smoke-test `WorldMirrorPipeline`
3. run one reconstruction test with controlled dungeon-room inputs
4. download outputs into `World Generation/Runs/DungeonRoom01/PodOutput`
5. decide whether the output is good enough for Blender QA
