# T66 Mob Rigging And Animation

This folder owns the repo-native process for regular enemy and mob animation source work. The current runtime target is vertex animation texture playback, not live skeletal animation.

## Scope

In scope:

- regular enemies and mobs
- Blender source scenes used to author mob motion
- VAT source generation, import, verification, and preview videos
- batch guidance for Stage 1 / Easy mobs

Out of scope:

- player heroes
- humanoid companions
- manual hero or companion rigging

Hero and companion rigging is intentionally handled outside this automation path. Do not recreate the retired humanoid bakeoff workflow unless the user explicitly reopens that research track.

## Primary Docs

- `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`: which file to read for a mob animation task
- `01_TOOL_SETUP_INSTRUCTIONS.md`: current Blender and VAT tool setup
- `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`: repo-proven findings and caveats
- `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`: full mob VAT pipeline contract
- `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`: Easy / Stage 1 enemy roster directions
- `06_MOB_ANIMATION_GUIDELINES.md`: running creative and technical guidelines for future mob animation passes

## Current Accepted Baseline

The accepted Slime move preview is the current style baseline for non-humanoid mob movement:

```text
Model Generation/Rigging and Animation/Runs/Slime_MoveTowardBouncyStutterPreview_V2_20260521/Slime_MoveTowardCamera_preview.mp4
```

That pass established the following direction:

- front-facing Slime source is `+Y`
- preview renders should use unlit/emissive materials so lighting does not create false shadow problems
- the mob should visibly change every authored frame
- movement should feel crunchy and slightly stuttered, as if authored for a lower frame rate
- gameplay owns actor translation; VAT sells body deformation, bounce, drag, and contact
- preview videos may add travel/root hop so the user can judge the motion without opening Unreal

## Kept Tools

Active mob tools live under `Tools/`:

- `create_easy_mob_vat_sources.py`
- `render_easy_mob_movement_preview.py`
- `make_easy_mob_contact_sheets.py`
- `import_easy_mob_vat_to_unreal.py`
- `verify_easy_mob_vat_in_unreal.py`

## Removed Humanoid Automation

The old humanoid rigging bakeoff scripts, iteration docs, and generated run folders were removed on 2026-05-21 because heroes and humanoid companions are no longer part of this automated rigging path. Durable lessons that still affect mobs were kept in the mob pipeline and findings docs.

## Runtime Boundary

A mob animation is not playable just because Blender preview looks good. The production path is:

1. approve the Blender preview
2. bake or import VAT assets
3. verify material/data-table playback
4. test many live instances with desynchronized timing
5. promote the data only after visual and performance acceptance

If playable content changes, follow the root standalone shortcut rule and refresh the staged build.
