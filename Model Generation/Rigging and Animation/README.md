# T66 Rigging And Animation

This folder owns the repo-native process for regular enemy/mob animation source work and the animated ToonStyle hero import bridge.

## Scope

In scope:

- regular enemies and mobs
- Blender source scenes used to author mob motion
- VAT source generation, import, verification, and preview videos
- batch guidance for Stage 1 / Easy mobs
- animated ToonStyle skeletal import/export for approved demo hero and companion visual rows

Out of scope:

- manual hero or companion rigging
- buying or authoring new humanoid animation packs outside the accepted Quaternius source

Manual hero and companion rigging remains outside this automation path. The only active humanoid automation here is the reusable Animated ToonStyle bridge that turns approved Pixal3D humanoid meshes into skeletal ToonStyle game assets with `Idle/Walk/Jump/Roll` rows.

## Primary Docs

- `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`: which file to read for a mob animation task
- `01_TOOL_SETUP_INSTRUCTIONS.md`: current Blender and VAT tool setup
- `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`: repo-proven findings and caveats
- `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`: full mob VAT pipeline contract
- `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`: Easy / Stage 1 enemy roster directions
- `06_MOB_ANIMATION_GUIDELINES.md`: running creative and technical guidelines for future mob animation passes
- `07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`: skeletal ToonStyle import bridge for demo-scope heroes and companions

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
- runtime-ready movement also needs an Unreal map preview so cadence can be judged against actual mob travel speed

## Kept Tools

Active mob tools live under `Tools/`:

- `create_easy_mob_vat_sources.py`
- `render_easy_mob_movement_preview.py`
- `make_easy_mob_contact_sheets.py`
- `import_easy_mob_vat_to_unreal.py`
- `verify_easy_mob_vat_in_unreal.py`

Active animated hero tools live under `Tools/`:

- `create_animated_toonstyle_hero_sources.py`
- `import_animated_toonstyle_heroes_to_unreal.py`

## Humanoid Automation Boundary

The old exploratory humanoid bakeoff workflow remains retired. The current humanoid path is narrower: use the accepted Hero 1 Rigify/Quaternius template and the production ToonStyle materials to generate importable skeletal assets for approved demo hero and companion visual rows.

## Runtime Boundary

A mob animation is not playable just because Blender preview looks good. The production path is:

1. approve the Blender preview
2. bake or import VAT assets
3. verify material/data-table playback
4. test many live instances with desynchronized timing
5. promote the data only after visual and performance acceptance

For movement-readiness checks, use `Scripts\CaptureT66EnemyAnimationPreview.ps1` after the VAT row is baked/imported. It captures one configured `AT66MobBase` moving under `UT66MobManagerSubsystem`, so the proof covers both local VAT motion and real map-speed match. Set warm-up with `-PostCaptureDelaySeconds`, not `-DelaySeconds`, because the latter delays preview setup rather than giving the spawned mob and camera time to settle.

If playable content changes, follow the root standalone shortcut rule and refresh the staged build.
