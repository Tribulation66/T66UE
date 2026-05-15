# Blender Processing And Rigging

Use Blender for deterministic inspection, cleanup, render QA, and export. Use RetopoFlow only for topology work that needs human-authored low-poly structure.

## Durable Helpers

- `Model Generation/Scripts/Core/Blender/blender_glb_qa.py`
- `Model Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1`
- `Model Generation/Tools/BlenderLabMCP/setup_blender_lab_mcp.ps1`

## Policy

- Do not promote Decimate output as accepted low-poly topology for deformation-critical characters.
- Decimate is acceptable for diagnostics and throwaway prototypes.
- Rigging work must bake mesh placement, keep feet-origin scale consistent, and preserve actions explicitly so exported animation assets survive.
- Equipment grip quality needs multi-angle validation, especially hands, shoulders, face, and weapon overlap.

## Legacy Notes

Older TypeA/Mike, Arthur, and dungeon-kit prototype scripts were deleted during Alpha 0.1 cleanup after their durable lessons were captured in the current instructions. New production work should promote reusable ideas into `Scripts/Core` instead of reviving one-off prototypes.
