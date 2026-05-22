# Mob Animation Guidelines

This is the running guide for future chats that need to replicate the current mob animation direction.

## Scope

Use this guide for regular enemies and mobs. Do not use it for player heroes or humanoid companions; those are manually rigged outside this automation path.

## Core Direction

The source asset can use whatever Blender authoring method fits the mob: bones, shape keys, lattices, object controls, constraints, or procedural keys. The runtime target for regular mobs is still static mesh plus VAT material playback.

The goal is not "smooth animation." The current T66 mob style should feel deliberately readable and gamey:

- every authored frame should have a visible movement or silhouette change
- no dead frames unless the pause is an intentional anticipation or impact beat
- stepped or constant interpolation is preferred for the first pass
- 15 fps preview is the current review target
- motion should read clearly from gameplay camera distance
- actor translation is gameplay-owned
- VAT should sell local bounce, drag, flap, recoil, jitter, squash, stretch, or impact

## Preview Rules

- Render native Blender MP4 files.
- Use unlit/emissive preview materials unless the user is reviewing lighting.
- Show the mob from its true front first.
- For source meshes that face `+Y`, put the front camera on `+Y`.
- Include actual travel in review videos when the user asks to see movement, even if runtime translation will be gameplay-owned.
- Prefer a longer preview than a tiny loop. The current Slime accepted pass used 72 frames at 15 fps.
- Keep the camera zoomed in enough to judge contact and deformation.

## Slime Baseline

Current accepted Slime movement preview:

```text
C:\UE\T66\Model Generation\Rigging and Animation\Runs\Slime_MoveTowardBouncyStutterPreview_V2_20260521\Slime_MoveTowardCamera_preview.mp4
```

Command used for the accepted style baseline:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python "C:\UE\T66\Model Generation\Rigging and Animation\Tools\render_easy_mob_movement_preview.py" -- --enemy-id Slime --out-root "C:\UE\T66\Model Generation\Rigging and Animation\Runs\Slime_MoveTowardBouncyStutterPreview_V2_20260521" --frames 72 --fps 15 --width 1280 --height 720
```

Accepted traits:

- front faces camera
- face is evenly visible, not shadowed
- every frame changes the pose or travel/contact state
- root travel moves toward camera in preview
- bounce is visible and slightly exaggerated
- the body still reads as a ground blob, not a hidden biped

Reject a Slime pass if:

- it moves sideways relative to the face
- the front face is dark because of preview lighting
- it glides smoothly with no body change
- it has long static holds
- it bounces so high that it stops feeling like a sticky blob

## Per-Mob Process

1. Read the enemy row and source map in `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`.
2. Confirm front axis, origin, scale, texture assignment, and live mesh match.
3. Author only the smallest clip needed for the current review. Start with `Move` unless the user asks otherwise.
4. Render a native Blender MP4 and contact sheet.
5. Inspect the video before handing it off:
   - is it facing the right direction?
   - does every frame move?
   - does the motion fit that creature's body?
   - is actor travel separated from local deformation?
   - does the gameplay camera read the intended threat?
6. Fix obvious issues before asking the user for visual review.
7. Document accepted settings and caveats in this file or the batch doc.

## VAT Performance Position

VAT is the right runtime direction for mob crowds because it avoids normal per-enemy skeletal animation ticking. That does not automatically prove hundreds of enemies will be free.

The current Easy VAT path still needs crowd profiling because total cost can move to:

- draw calls
- per-enemy dynamic material instances
- material parameter updates
- overdraw
- collision and AI
- pooling/spawning
- bounds and culling

For true hundreds, expect a later optimization pass to test instancing, per-instance custom data, phase randomization, culling, pooling, reduced texture sizes, and lower clip frame counts.

## Next Recommended Work

1. Lock `Slime Move` as the visual baseline.
2. Create `Slime Idle` using the same every-frame-change principle.
3. Add `Slime AttackCue` only after movement and idle are accepted.
4. Move through Stage 1 mobs in this order: `CaveBat`, `BoneWalker`, `RatPack`, `TombSpider`, `HexSlinger`, `StoneSentinel`.
5. After each accepted Blender preview, bake/import temporary VAT assets and verify runtime playback.
6. Run a focused crowd performance pass before promoting the VAT rows as final.
