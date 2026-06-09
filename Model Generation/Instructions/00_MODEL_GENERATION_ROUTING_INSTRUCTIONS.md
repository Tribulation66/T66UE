# Model Generation Master

This workspace owns model-generation process, not runtime game content. A model is not part of the playable build until it is imported through the Unreal scripts, validated, cooked, staged, and the standalone shortcut points at the staged executable when the change affects playable standalone output.

## Decision Tree

1. For a new source image or character direction, start with `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`.
   - For humanoid heroes, companions, or manually rigged characters, continue with `10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md` before writing the live prompt.
2. For TRELLIS RunPod setup, use `01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md`.
3. For Blender cleanup, rigging, or retopo policy, use `04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md`.
   - For editable character rigs, retargeting, Rigodotify, Quaternius, or authored hero/enemy animation sets, route into `../Rigging and Animation/RIGGING_ANIMATION_AGENTS.md`.
4. For Unreal import, DataTable reloads, material checks, or standalone verification, use `05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`.
5. For known failure modes and historical lessons, use `06_RUN_HISTORY_AND_KNOWN_ISSUES_REFERENCE.md`.
6. For Pixal3D production replacement assets, read `11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` first, then `../Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md`, `07_PIXAL3D_RUNPOD_SETUP_INSTRUCTIONS.md`, and `08_PIXAL3D_TROUBLESHOOTING_INSTRUCTIONS.md` for CuMesh/export/remesh failures.
7. For FriendSlop raw Pixal3D replacements, read `11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`; these imports intentionally skip retired ToonStyle/QuadRetro processing but must explicitly import and bind the generated GLB base-color texture.
8. For raw FriendSlop humanoid rigs intended for later skeletal import, PhysicsAsset, ragdoll, or PAC work, read `13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`. Do not route that work through the archived AccuRig / Animated ToonStyle bridge unless the user explicitly revives that legacy path.

## Current Source Rule

Current FriendSlop model work uses the raw Pixal3D FriendSlop source runs under `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532` and `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415`.

Do not route new FriendSlop work through archived AccuRig / Animated ToonStyle hero demo lineups. `HeroDemoLineup_20260522_AccuRig` is archived historical provenance, not an active source. If a raw FriendSlop export report contains `AccuRig_Textured`, treat that as a legacy exporter name only.

Retired ToonStyle, Animated ToonStyle, and QuadRetro process docs are archived under root `Archive/ToonStyle/` and `Archive/RetroFX/`. They are historical evidence, not active routing, unless the user explicitly asks to revive them.

## Folder Rules

- `Scripts/Core`: reusable helpers only.
- `Scripts/Batches`: named batch drivers; delete after the batch is done.
- `Scripts/Legacy`: old prototype helpers; keep only while they document an unresolved workflow.
- `Tools`: server and Blender helper assets.
- Generated `Runs`, `Scenes`, `Archive`, `Reference`, and local access files are cleanup targets, not durable source.

## Script Lifecycle

Do not add a new one-off script when a manifest can drive an existing master. If a one-off is needed, delete it after the task succeeds or fails conclusively, and move the durable lesson into a core script, README, or instruction doc.
