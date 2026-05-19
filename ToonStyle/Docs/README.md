# ToonStyle

ToonStyle is the rendering workspace for moving T66 from a retro-first post-process look to a clean cel-shaded foundation. The intended stack is native-resolution toon rendering first, with retro pixelation/dither reintroduced later as an optional final overlay.

Current phase: Phase 1C foundation and Pixal3D production replacement support.
This workspace now contains production ToonStyle material, shader, Blender
pipeline, import, and documentation assets. Historical Phase 0/1A reports remain
for traceability, but active model replacement work must follow the current
Pixal3D production workflow.

## Folder Layout

- `Shaders/` - ToonStyle shader source and `.ush` support code.
- `Materials/` - material specs and any ToonStyle material asset notes.
- `Source/` - Unreal Python setup/import scripts for ToonStyle materials and Pixal3D production imports.
- `BlenderScripts/` - automated mesh-normal, vertex-color, outline, texture, Tint, and foundation-tool pipeline scripts.
- `Tools/` - local ToonStyle helper scripts, currently including the Codex CLI image-generation wrapper.
- `Docs/` - durable ToonStyle process docs.
- `Reports/` - phase reports and handoff notes, grouped by phase subfolder.

## Workflows

- [Codex CLI Image Generation Workflow](IMAGEGEN_CLI_WORKFLOW.md) - generate images through the local Codex CLI and save paths to disk without embedding images in chat.
- [Pixal3D ToonStyle Production Import](../../Model%20Generation/Instructions/09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md) - canonical path for Pixal3D models entering playable content with the full ToonStyle NPR stack.

## Reports

- [Phase0/Phase0_Situation_Report.md](../Reports/Phase0/Phase0_Situation_Report.md)
- [Phase0/Phase0_Codex_Opinion.md](../Reports/Phase0/Phase0_Codex_Opinion.md)
- [Phase0/Phase0_Pending_Questions.md](../Reports/Phase0/Phase0_Pending_Questions.md)
- [Phase05/Phase05_Blender_Deep_Dive.md](../Reports/Phase05/Phase05_Blender_Deep_Dive.md)
- [Phase05/Phase05_Screenshot_Causality.md](../Reports/Phase05/Phase05_Screenshot_Causality.md)
- [Phase05/Phase05_Codex_Opinion.md](../Reports/Phase05/Phase05_Codex_Opinion.md)
- [Phase05/Phase05_Updated_Pending_Questions.md](../Reports/Phase05/Phase05_Updated_Pending_Questions.md)
- [Phase1A/Phase1A_Preflight_Inventory.md](../Reports/Phase1A/Phase1A_Preflight_Inventory.md)
- [Phase1A/Phase1A_Preflight_Opinion.md](../Reports/Phase1A/Phase1A_Preflight_Opinion.md)
- [Phase1A/Phase1A_Preflight_Recommended_Scope.md](../Reports/Phase1A/Phase1A_Preflight_Recommended_Scope.md)

Historical reports may describe earlier report-only or research-only boundaries.
For active production replacement work, follow the current instruction docs and
folder agent files rather than old phase-report wording.
