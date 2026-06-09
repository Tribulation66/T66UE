# T66 Rigging And Animation

This folder owns repo-native animation source work. Regular mobs still use the VAT-oriented process here; physics-first raw FriendSlop hero rigs route through the model-generation instruction set and are treated as gameplay-facing hero foundations, not the old Animated ToonStyle bridge.

## Scope

In scope:

- regular enemies and mobs
- Blender source scenes used to author mob motion
- VAT source generation, import, verification, and preview videos
- batch guidance for Stage 1 / Easy mobs
- historical animated ToonStyle skeletal import/export evidence, archived and not active routing
- physics-first raw FriendSlop hero rigging process routing

Out of scope:

- broad manual hero or companion rigging outside an approved physics-first process
- buying or authoring new humanoid animation packs outside the accepted Quaternius source

Manual hero and companion rigging remains outside the mob/VAT automation path. Hero 1 Chad is the approved exception for a physics-first raw FriendSlop process.

Explicit raw FriendSlop hero rigging for skeletal FBX, pose-target clips, later PhysicsAsset, and active-ragdoll readiness is routed through `../Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`, not through the retired humanoid bakeoff or the legacy Animated ToonStyle bridge.

## Primary Docs

- `00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`: which file to read for a mob animation task
- `01_TOOL_SETUP_INSTRUCTIONS.md`: current Blender and VAT tool setup
- `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`: repo-proven findings and caveats
- `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`: full mob VAT pipeline contract
- `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`: Easy / Stage 1 enemy roster directions
- `06_MOB_ANIMATION_GUIDELINES.md`: running creative and technical guidelines for future mob animation passes
- `../Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`: active physics-first raw FriendSlop humanoid rigging route

Archived historical ToonStyle bridge doc:

- `../../Archive/ToonStyle/Model Generation/Rigging and Animation/07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`

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

Legacy animated hero bridge tools live under `Tools/`:

- `create_animated_toonstyle_hero_sources.py`
- `import_animated_toonstyle_heroes_to_unreal.py`

These tools are retained only as historical Animated ToonStyle bridge context. Do not use them as the current FriendSlop model source path. Current FriendSlop raw Pixal3D work starts from the FriendSlop runs documented in `../README.md` and `../Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.

Physics-first hero tools live under `Tools/`:

- `create_physics_first_hero1_chad_sources.py`
- `import_physics_first_hero1_chad_to_unreal.py`

These start from the raw FriendSlop GLB and write fresh Stage 2 outputs under the Hero 1 FriendSlop run. Do not point them at old spike Rigging outputs.

## Humanoid Automation Boundary

The old exploratory humanoid bakeoff workflow remains retired. The Animated ToonStyle bridge is legacy maintenance only. The current Hero 1 Chad path is the physics-first raw FriendSlop standard with `Idle`, `Walk`, `Jump`, and `Leap` pose-target clips.

## Runtime Boundary

A mob animation is not playable just because Blender preview looks good. The production path is:

1. approve the Blender preview
2. bake or import VAT assets
3. verify material/data-table playback
4. test many live instances with desynchronized timing
5. promote the data only after visual and performance acceptance

For movement-readiness checks, use `Scripts\CaptureT66EnemyAnimationPreview.ps1` after the VAT row is baked/imported. It captures one configured `AT66MobBase` moving under `UT66MobManagerSubsystem`, so the proof covers both local VAT motion and real map-speed match. Set warm-up with `-PostCaptureDelaySeconds`, not `-DelaySeconds`, because the latter delays preview setup rather than giving the spawned mob and camera time to settle.

If playable content changes, follow the root standalone shortcut rule and refresh the staged build.
