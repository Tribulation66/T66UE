# Rigging And Animation Agents

## Owns

Blender source-of-truth mob animation, vertex animation texture source generation, mob preview renders, mob contact sheets, VAT import manifests, and verification evidence before Unreal promotion.

## Out Of Scope

Automated hero and humanoid companion rigging research is out of scope for this folder. The user is handling manual rigging decisions. Do not revive the retired humanoid bakeoff process unless the user explicitly asks to restart that research. The narrow Animated ToonStyle bridge for approved demo hero/companion meshes is still in scope as import/export infrastructure.

## Trigger Words

Mob animation, enemy animation, VAT, vertex animation texture, AnimToTexture, Slime movement, Stage 1 enemies, Easy mobs, Blender preview, contact sheet, mob movement, non-humanoid animation.

## Read First

- `README.md`
- `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
- `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`
- Then the specific numbered instruction file for the task.
- For playable imports, also read `../Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
- For crowd/performance questions, also read `../../PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`.

## Hard Rules

- Blender `.blend` scenes are the source of truth for editable mob animation.
- Runtime playback for regular mobs should be static-mesh/VAT-driven unless a specific mob is documented as an exception.
- Keep third-party packages in `External/`; do not commit extracted vendor files.
- Do not treat "it moves" as accepted animation. Timing, silhouette, contact, deformation, facing, and gameplay-camera readability must pass visual QA.
- Keep actor translation gameplay-owned. VAT should sell local body motion unless a specific clip intentionally bakes local displacement for a reviewed reason.
- Do not wire new animation assets into playable content until Blender QA and Unreal import validation are complete.
- Record tool failures and workarounds in `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`.

## Verification

Report the exact Blender version, source files used, exported files, preview video or contact-sheet evidence, Unreal import evidence, DataTable/runtime wiring evidence, crowd/performance evidence when relevant, and staged standalone evidence when gameplay-visible content changes.
