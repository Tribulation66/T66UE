# Operator Draft: Hero 1 Male Rubber Blender Look-Dev

## Scope

Codex used the confirmed Hero 1 male FriendSlop source and stayed entirely in Blender/Blender CLI. No Unreal work, rigging, jiggle, runtime import, DataTable, or Git operation was performed.

Source assets:
- GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Source image: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Sources\Hero_1_Chad_Male.png`
- Existing review scene preserved: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\Hero_1_Chad_Male_Review.blend`
- Provenance basis: the look-dev scene imports directly from the FriendSlop Pixal3D source GLB above, not from any archived AccuRig asset.

## PPF Close

Process used: Blender look-dev from the raw FriendSlop Pixal3D GLB, fixed soft/shadowless lighting, parameter-swept Principled BSDF rubber/vinyl material, non-destructive bevel/weighted normals, comparison grid, and turntable evidence.

Matches declared process: YES

Evidence:
- New Blender file: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`
- Reproducible build script: `C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\build_hero1_male_rubber_lookdev.py`
- Recipe Markdown: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\rubber_recipe.md`
- Recipe JSON: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\rubber_recipe.json`

## Mechanism Close

Mechanism: Fixed soft flat lighting
- Status: PRESENT
- Evidence: Recipe records white world, Standard view transform, weak AO, and four shadow-disabled area lights.
- Discriminator test: Rendered grid and turntable frames use the same soft rig; no Unreal or desktop screenshot substitute.
- Reported status: FULL

Mechanism: Shader-driven rubber response
- Status: PRESENT
- Evidence: Six named material variations use Principled BSDF parameters for roughness, specular IOR, coat, coat roughness, subsurface, saturation/value, contrast flattening, bevel, and bump.
- Discriminator test: Raw Pixal3D material is present next to rubber variants; the variants are not just relit raw output.
- Reported status: FULL

Mechanism: Soft silhouette/edge catch
- Status: PRESENT
- Evidence: Rubber copies include non-destructive bevel modifiers and weighted normals; values are in the variation table.
- Discriminator test: Edge softness is geometry/normal driven and portable to UE5 as bevelled geometry/weighted normals.
- Reported status: FULL

Mechanism: Comparison under fixed viewing condition
- Status: PRESENT
- Evidence: `Hero_1_Chad_Male_rubber_comparison_grid.png` and `Hero_1_Chad_Male_rubber_comparison_grid_preview_1300.png` show reference image, raw GLB, and six variants in one rig.
- Discriminator test: The final grid is front-facing, readable, and includes the source reference panel after UV/emission correction.
- Reported status: FULL

Mechanism: Multi-angle judgment
- Status: PRESENT
- Evidence: Four top-candidate turntables are encoded as MP4 from 72 rendered Blender frames each.
- Discriminator test: `ffprobe` shows 1280x1280, 72 frames, 3 seconds, 24 fps for all four MP4s.
- Reported status: FULL

## Rendered Artifacts

- Comparison grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_rubber_comparison_grid.png`
- Preview grid: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_rubber_comparison_grid_preview_1300.png`
- V02 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V02_rubber_pop_turntable.mp4`
- V03 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V03_vinyl_bounce_turntable.mp4`
- V04 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V04_candy_rubber_turntable.mp4`
- V06 turntable: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_V06_toy_vinyl_gloss_turntable.mp4`

## Top Candidate

Suggested first candidate for Pablo review: `V04_candy_rubber`.

Reason: It is the softest static read in the matrix, with more subsurface, stronger edge rounding, and broad coat highlights without going as toy-plastic as `V06_toy_vinyl_gloss`.

This is not declared the final locked target until Pablo reviews and either accepts it or gives a critique such as "more rubbery", "less glossy", "too plastic", or "more Fall Guys".

## Verification Performed

- `python -m py_compile` passed for the Blender build script.
- Blender CLI 5.1 imported the raw GLB, rendered the grid, rendered all turntable frame sequences, saved the `.blend`, and wrote recipe files.
- Visual check: final grid preview is readable and includes reference, raw, and six variants.
- Visual check: `V04_candy_rubber_0001.png` shows a front-facing full character under the fixed rig.
- Frame counts: V02/V03/V04/V06 each have 72 PNG frames.
- MP4 validation: all four MP4s are 1280x1280, 72 frames, 3.0 seconds, 24 fps.
- Blender MCP live connection was unavailable, so Blender work used installed local Blender CLI at `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`.
