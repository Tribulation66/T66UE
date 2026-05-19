# Phase 0.5 Updated Pending Questions

This supersedes the Phase 0 pending list where Phase 0.5 produced codebase evidence. Pablo-decision questions are restated rather than answered by Codex.

## Still For Pablo

### 1. What is the first ToonStyle success target?

Pablo still needs to decide whether Phase 1 success means "one static character and one environment asset look clean in-editor," "one gameplay room looks clean in runtime," or something broader.

New context: the DCC pipeline should be included in that target. A shader-only success target will not prove the real ToonStyle foundation.

### 2. Should the final look prioritize HI-FI Rush environment language or Guilty Gear character language first?

Still a creative decision. The pipeline evidence suggests characters need more DCC work first, while environment materials may be simpler to prototype.

### 3. What is acceptable for generated texture simplification?

Pablo needs to decide how aggressively the pipeline may flatten generated texture detail. A clean toon branch can preserve source colors, posterize/simplify regions, or create very flat color islands. Those are different looks.

### 6. How much runtime retro presentation should remain visible during ToonStyle validation?

Pablo needs to decide the validation default. Evidence says fresh Retro FX settings do not enable PS1 dither/pixelation, but do enable real low resolution while master is true. For clean ToonStyle review, real low resolution should be disabled or master should be off.

### 9. Is the ToonStyle pipeline allowed to create new texture/material fields, or must it reuse `PixelatedTextureAssetPath`?

This is partly technical and partly product/process. Reusing the field is fast but semantically wrong. A new clean texture path is cleaner but touches data model/import code.

### 10. How much quality ceiling is acceptable from deterministic AI/DCC heuristics?

Still a Pablo decision. Proxy normals and vertex-color heuristics can improve the look, but they will not match hand-authored Guilty Gear asset work.

## Resolved Or Partially Resolved By Phase 0.5

### 4. What should be the lowest-risk Phase 1 test pair?

Resolved recommendation: use a static QuadRetro NPC plus a generated environment module.

Recommended character: `Gambler`.

Evidence:

- `Content/Data/CharacterVisuals.csv:77` maps `Gambler` to `/Game/Characters/NPCs/Gambler/QuadRetro/SM_Gambler_QuadRetro` and a `Pixelated_512_Normalized` texture.
- It uses the production `FT66CharacterVisualRow` static mesh path without skeletal animation or VAT.

Recommended environment: one generated Coherent Theme Kit wall/floor module, or a simple generated dungeon wall/floor module already using the static mesh import path.

Reason: it exercises the environment side without mixing in character texture binding assumptions.

### 5. Is `M_GLB_ViewSpaceLit_Character` useful or a dead-end?

Resolved: useful as a prototype/reference, not final architecture.

Evidence:

- `Content/Materials/pending_issues_Materials.md:6` says it is retained for Track 2 A/B testing.
- Binary inspection shows unlit material markers, `MaterialExpressionCustom`, `PixelNormalWS`, view-space light direction, ramp thresholds, shadow/midtone/highlight tints, and rim parameters.

Action: inspect/evaluate it in Phase 1B before writing a new material from scratch. Do not promote it directly without solving `.ush` ownership and DCC data.

### 7. Should QuadRetro get a non-pixelated output mode before material work?

Resolved: yes, but frame it as "clean flat-color output with optional pixelation."

Evidence:

- Current wrapper reports already have `dither_type=none`, so "skip dithering" is insufficient.
- `make_pixelated_image()` is always called at `t66_quad_retro_character_pipeline.py:1215` through `t66_quad_retro_character_pipeline.py:1218`.
- `assign_pixel_material()` sets image interpolation to `Closest` at `t66_quad_retro_character_pipeline.py:1043`.
- Downstream importers prefer `Pixelated_512` texture names.

Action: add a clean branch with its own naming/binding path. Keep retro output backward compatible.

### 8. What vertex color channel layout is technically feasible?

Partially resolved technically, final layout still needs Pablo/design approval.

Recommended first technical layout:

- R: shade threshold offset or AO.
- G: outline width multiplier.
- B: outline depth/push or joint suppression.
- A: outline mask/default.

Evidence:

- QuadRetro currently writes no `COLOR_0` in sampled GLBs.
- Quad Remesher settings hardcode `UseVertexColorMap=False`.
- No existing project utility authors ToonStyle vertex colors.

Action: add deterministic vertex color authoring after retopo import and before GLB export. Verify `COLOR_0` in GLB and in Unreal import before relying on it in materials.

### 11. Is Pixal3D part of the runtime pixelation system?

Resolved: no.

Pixal3D is a separate research model-generation pipeline under `Model Generation/Pixal3D/`. Runtime pixelation is `UT66PixelationSubsystem` plus `UT66RetroFXSubsystem`. The names are similar but the systems are unrelated.

Phase 0.5 adds: Pixal3D GLBs can already carry noisy/generated texture data before QuadRetro runs, so it can contribute to visible grain even when runtime pixelation is off.

### 12. Which docs are stale?

Resolved list for later update:

- `Gameplay/World/T66_LIGHTING_REFERENCE.md`
- `Model Generation/Instructions/03_QUAD_RETRO_PIPELINE_INSTRUCTIONS.md`
- `Model Generation/Instructions/README.md`
- `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
- `Model Generation/Pixal3D/PIXAL3D_PIPELINE_REFERENCE.md` if Pixal3D becomes a ToonStyle source path
- `Content/Materials/pending_issues_Materials.md`
- `ToonStyle/Docs/README.md`

Action: do not update these in Phase 0.5. Update after Pablo decides the Phase 1 branch and test target.

## New Blocking Recommendation Before Phase 1 Prompt

Do not draft Phase 1 as shader-first. Draft it as a DCC clean-branch proof followed by a minimal Unreal material test.

Minimum acceptance criteria before broader ToonStyle work:

- A sample GLB has no `Pixelated_512` final atlas dependency.
- The sample texture is clean, full-resolution or intentionally filtered.
- The sample mesh carries `NORMAL` data that was not forced flat.
- The sample mesh carries a verified `COLOR_0` vertex color layout.
- Runtime validation is done with PS1/pixelation and real low resolution disabled.
