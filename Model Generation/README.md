# Model Generation Workspace

This folder is the main local workspace for `T66` 3D model generation and Blender-side postprocess work.

If an agent needs to work on TRELLIS, Blender cleanup, retopo, or mesh QA, start here first.

## Start Here

Read these in order:

1. [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md)
2. [ENVIRONMENT_LOCK.md](C:/UE/T66/Model%20Generation/ENVIRONMENT_LOCK.md)
3. [RUN_HISTORY.md](C:/UE/T66/Model%20Generation/RUN_HISTORY.md)
4. [KNOWN_ISSUES.md](C:/UE/T66/Model%20Generation/KNOWN_ISSUES.md)
5. [NEXT_STEPS.md](C:/UE/T66/Model%20Generation/NEXT_STEPS.md)

Use [CURRENT_HANDOFF_PROMPT.md](C:/UE/T66/Model%20Generation/CURRENT_HANDOFF_PROMPT.md) only when continuing a handoff-heavy Codex session.

## What Lives Here

- `MASTER_WORKFLOW.md`
  - source of truth for the current production workflow
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
- `Tools/`
  - Trellis server and Blender MCP recovery helpers
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
- [start_blender_mcp.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/start_blender_mcp.py)
  - local Blender MCP recovery helper

One legacy redirect file remains outside this folder only so old links still resolve:

- [TRELLIS2_RunPod_Character_Workflow.md](C:/UE/T66/Docs/UI/TRELLIS2_RunPod_Character_Workflow.md)
  - redirect only, not a second source of workflow truth

## Blender Stack

Current local workstation baseline:

- Blender `5.1.1`
- Blender MCP add-on installed locally
- RetopoFlow `4.1.5` installed locally
- Headless Blender QA helper: [blender_glb_qa.py](C:/UE/T66/Model%20Generation/Scripts/blender_glb_qa.py)

If Blender MCP appears unavailable:

1. Reopen Blender.
2. Rerun [start_blender_mcp.py](C:/UE/T66/Model%20Generation/Tools/Trellis2/start_blender_mcp.py).
3. Fall back to headless Blender only if the live session still does not attach.

## Tool Roles

Use these tools for different jobs. They are not interchangeable.

### Blender MCP

Use Blender MCP as the agent-controlled automation layer:

- import, export, and inspect models
- run scripted scene edits
- capture deterministic screenshots
- perform repeatable QA and verification work
- drive asset import helpers like PolyHaven or Sketchfab when available

This is the canonical Codex-to-Blender bridge.

### RetopoFlow

Use RetopoFlow as the manual topology-authoring tool inside Blender:

- clean hero topology when decimate is not good enough
- fix deformation-critical areas like hands, face, shoulders, elbows, and knees
- prepare meshes that may need rigging, posing, or bake-friendly topology

RetopoFlow is the retopology tool itself. MCP should orchestrate around it, not pretend to replace it.

### Blender Buddy

If you install Blender Buddy locally, use it as an in-Blender interactive copilot:

- tool discovery and workflow suggestions while a human is driving Blender
- one-off modeling or material questions
- quick local context help while already inside Blender

Do not treat Blender Buddy as the source of truth for the repo workflow. It is a local assistant, not the canonical automation backbone.

## MCP / Session Capabilities

The available Blender-side integrations in this Codex environment are broader than the repo-local docs currently reflect:

- direct Blender Python execution through the Blender MCP bridge
- scene and object inspection
- viewport screenshots
- PolyHaven asset download/import
- Sketchfab model download/import
- Hunyuan3D generation/import
- Hyper3D generation/import

There is no separate repo-local `polytools` package here. If "polytools" means the current asset integrations, the relevant live connectors are PolyHaven and Sketchfab.

## Cleanup Notes

This workspace is already mostly in the right place, but these are the current rough edges:

- `CURRENT_HANDOFF_PROMPT.md` and `SECOND_ATTEMPT_PROMPT.md` are operational prompts, not evergreen docs. They are useful, but they are secondary to the main workflow files.
- RetopoFlow usage expectations were previously implicit and are now documented here instead of being scattered through older notes.

Recommended policy:

- keep `MASTER_WORKFLOW.md` as the single authoritative workflow
- keep exact environment and failure-state details in `ENVIRONMENT_LOCK.md` and `KNOWN_ISSUES.md`
- keep run evidence in `RUN_HISTORY.md`
- avoid creating new workflow docs outside `Model Generation` unless they are clearly cross-domain
