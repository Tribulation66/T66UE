# Model Generation Workspace

This folder is the main local workspace for `T66` 3D model generation and Blender-side postprocess work.

If an agent needs to work on TRELLIS, Blender cleanup, retopo, or mesh QA, start here first.

## Start Here

Read these in order:

1. [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md)
2. [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md)
3. [Model Importing.md](C:/UE/T66/Model%20Generation/Model%20Importing.md)
4. [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
5. [RUN_HISTORY.md](C:/UE/T66/Model%20Generation/RUN_HISTORY.md)
6. [KNOWN_ISSUES.md](C:/UE/T66/Model%20Generation/KNOWN_ISSUES.md)
7. [NEXT_STEPS.md](C:/UE/T66/Model%20Generation/NEXT_STEPS.md)

For character work after raw TRELLIS output exists, read [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md) before Blender assembly, rigging, Unreal import, DataTable wiring, or staged visual verification.

For modular dungeon environment pieces, read [MODULAR_DUNGEON_KIT_PROCESS.md](C:/UE/T66/World%20Generation/MODULAR_DUNGEON_KIT_PROCESS.md) and [SHARED_ASSET_PIPELINE.md](C:/UE/T66/World%20Generation/SHARED_ASSET_PIPELINE.md).

Use [CURRENT_HANDOFF_PROMPT.md](C:/UE/T66/Model%20Generation/CURRENT_HANDOFF_PROMPT.md) only when continuing a handoff-heavy Codex session.

## What Lives Here

- `MASTER_WORKFLOW.md`
  - source of truth for the current production workflow
- `Model Processing.md`
  - required post-TRELLIS character setup, scale, material, import, and verification guide
- `Model Importing.md`
  - dedicated Unreal import scripts, DataTable reloads, material repair, and staged verification runbook
- `ENVIRONMENT_LOCK.md`
  - exact known-good pod and workstation baseline
- `RUN_HISTORY.md`
  - what actually worked, failed, or partially worked
- `KNOWN_ISSUES.md`
  - current technical and quality failure modes
- `NEXT_STEPS.md`
  - current recommended work order
- `MESH_APPROVAL_CHECKLIST.md`
  - approval gate before retopo or import
- `Rigging Process.md`
  - focused character rigging, animation, and weapon attachment guide
- `WALLS_FLOORS_CEILINGS.md`
  - redirect to the world-generation modular dungeon kit process
- `Tools/`
  - Trellis server and Blender MCP launch/recovery helpers
- `Scripts/`
  - local helpers for pod bootstrap, Blender QA, and local workflow operations
- `Runs/`
  - all tracked generation artifacts
- `Scenes/`
  - working Blender scenes and review captures
- `Reference/`
  - historical setup references that are not the active source of truth

## Important Files Outside This Folder

The active Trellis helpers now live inside this workspace:

- [trellis_server.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/trellis_server.py)
  - canonical TRELLIS API server template copied to the RunPod workspace
- [launch_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1)
  - local Blender Lab MCP launcher and recovery helper
- [setup_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/setup_blender_lab_mcp.ps1)
  - reinstall/update helper for the official Blender Lab source clone, Python venv, and Blender extension

One legacy redirect file remains outside this folder only so old links still resolve:

- [TRELLIS2_RunPod_Character_Workflow.md](C:/UE/T66/Docs/UI/TRELLIS2_RunPod_Character_Workflow.md)
  - redirect only, not a second source of workflow truth

## Blender Stack

Current local workstation baseline:

- Blender `5.1.1`
- Blender Lab MCP add-on `mcp` version `1.0.0` installed locally from [blender.org/lab/mcp-server](https://www.blender.org/lab/mcp-server/)
- Blender Lab MCP source clone at `C:\Users\DoPra\.codex\tools\blender_mcp_official`
- Codex MCP config points at `C:\Users\DoPra\.codex\tools\blender_mcp_official\mcp\.venv\Scripts\blender-mcp.exe`
- RetopoFlow `4.1.5` installed locally
- RetopoFlow rule: [RETOPOFLOW_4.md](C:/UE/T66/Model%20Generation/RETOPOFLOW_4.md)
- Headless Blender QA helper: [blender_glb_qa.py](C:/UE/T66/Model%20Generation/Scripts/blender_glb_qa.py)

If Blender MCP appears unavailable:

1. Run [launch_blender_lab_mcp.ps1](C:/UE/T66/Model%20Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1).
2. Use `-Visible` if a human needs to work in the Blender window.
3. Fall back to headless Blender only if the live session still does not attach.

## Tool Roles

Use these tools for different jobs. They are not interchangeable.

### Blender MCP

Use Blender MCP as the agent-controlled automation layer:

- import, export, and inspect models
- run scripted scene edits
- capture deterministic screenshots
- perform repeatable QA and verification work
- query bundled Blender Python API and manual documentation

This is the canonical Codex-to-Blender bridge. Use Blender's Lab page and `projects.blender.org/lab/blender_mcp` as the upstream source of truth.

### RetopoFlow

Use RetopoFlow as the manual topology-authoring tool inside Blender:

- clean hero topology and any accepted low-poly topology work
- fix deformation-critical areas like hands, face, shoulders, elbows, and knees
- prepare meshes that may need rigging, posing, or bake-friendly topology

RetopoFlow is the retopology tool itself. MCP should orchestrate around it, not pretend to replace it.
Do not promote Blender Decimate modifier output as accepted low-poly work. Decimate is limited to throwaway diagnostics and failed prototypes.

### Blender Buddy

If you install Blender Buddy locally, use it as an in-Blender interactive copilot:

- tool discovery and workflow suggestions while a human is driving Blender
- one-off modeling or material questions
- quick local context help while already inside Blender

Do not treat Blender Buddy as the source of truth for the repo workflow. It is a local assistant, not the canonical automation backbone.

## MCP / Session Capabilities

The official Blender Lab MCP integration provides:

- direct Blender Python execution through the Blender MCP bridge
- scene, object, and data-block inspection
- viewport screenshots
- small render/thumbnail helpers
- bundled Blender Python API search
- bundled Blender manual search

## Cleanup Notes

This workspace is already mostly in the right place, but these are the current rough edges:

- `CURRENT_HANDOFF_PROMPT.md` and `SECOND_ATTEMPT_PROMPT.md` are operational prompts, not evergreen docs. They are useful, but they are secondary to the main workflow files.
- RetopoFlow usage expectations were previously implicit and are now documented here instead of being scattered through older notes.

Recommended policy:

- keep `MASTER_WORKFLOW.md` as the single authoritative workflow
- keep exact environment and failure-state details in `ENVIRONMENT_LOCK.md` and `KNOWN_ISSUES.md`
- keep run evidence in `RUN_HISTORY.md`
- avoid creating new workflow docs outside `Model Generation` unless they are clearly cross-domain
