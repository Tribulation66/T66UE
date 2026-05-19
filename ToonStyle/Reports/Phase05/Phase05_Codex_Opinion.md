# Phase 0.5 Codex Opinion

## Direct Answer

Claude's updated read is basically correct: the DCC pipeline is where most of the ToonStyle work has to happen.

There is a UE5-heavy prototype path, and it is worth using for fast A/B checks. But there is no credible UE-only path that turns the current QuadRetro output into a HI-FI Rush or Guilty Gear-style foundation. The current assets are flat-shaded, nearest-sampled, pixel-atlas GLBs with no vertex color payload. A material can band lighting, add outlines, and hide some roughness, but it cannot recover intentional normals, clean color regions, or vertex-authored shade control that never made it into the mesh.

## What UE5 Can Do Without Blender Changes

UE5 can give Pablo a quick proof of direction:

- Use `M_GLB_ViewSpaceLit_Character` as a reference or temporary prototype.
- Add smoothstep AA in material code.
- Use view-space/world-space normal bands.
- Use a controlled light vector instead of scene lights.
- Add post-process outlines for environments.
- Add per-material tint/shadow parameters.

That path can answer "do we like a cel ramp on T66 silhouettes?" quickly. It cannot answer "can T66 achieve a clean high-fidelity toon identity?" because the current source atlases already carry texture noise and the current meshes do not carry toon-authoring data.

## Why Blender/DCC Work Is Not Optional

The current QuadRetro script directly conflicts with the target look:

- It defaults to recalculated normals and flat shading.
- It hardcodes final pixel atlas generation.
- It binds the final GLB to a nearest-filter unlit pixel material.
- It writes no `COLOR_0` attribute.
- It uses Quad Remesher with `UseVertexColorMap=False`.
- It relies on a diffuse bake of generated source texture data rather than clean flat color regions.

Those are asset properties. Shader code can work around them only superficially.

Guilty Gear's clean bands come from controlled normals and vertex color thresholds. HI-FI Rush's clean environment look comes from controlled materials, crisp resolution, and deliberately authored shade/comic passes. T66 currently has almost none of that in the DCC output.

## Phase 1 Should Not Start With .ush Files

The original Phase 1 plan starts with Unreal material foundation and shader plumbing. That is premature.

The next real phase should start with a pipeline proof:

1. Add a clean output mode to QuadRetro or a sibling ToonStyle DCC script.
2. Produce a non-pixel, non-dither, non-quantized final texture output.
3. Preserve smooth/custom normals instead of forcing flat shading.
4. Add a first vertex color payload with deterministic defaults.
5. Import one static character test mesh and verify Unreal sees the intended texture, normals, and vertex colors.
6. Only then wire a cel material.

Doing material-first will create false negatives. Pablo will see rough bands and noisy fills and think the shader failed, when the source asset is the real source of the damage.

## The Better Sequencing

I would restructure the upcoming work:

- Phase 1A: DCC clean branch and import verification.
- Phase 1B: Minimal character cel material using the verified clean branch.
- Phase 1C: Environment material prototype on one generated wall/floor.
- Phase 2: Outlines, after normal and scale behavior are known.
- Phase 3: AO-to-vertex-color and outline-channel authoring.
- Phase 4: Halftone/hatching only after clean source images exist.
- Phase 6: Retro overlay remains last.

This still preserves the six-phase intent, but it moves proof of asset data before shader ambition.

## M_GLB_ViewSpaceLit_Character

Do not discard this material yet. Binary inspection shows a real unlit view-space cel ramp with tint and rim parameters. It is likely a useful reference for Phase 1B.

But do not make it the architecture. It is material-contained, not `.ush`-owned, and it does not solve the DCC data problem. Use it as a prototype or comparison baseline, not as the final ToonStyle foundation.

## Clean Output Mode

The right framing is "clean flat-color output with optional pixelation." Do not frame the change as "turn dither off." Dither is already off in current wrapper reports.

Clean mode should:

- Use the bake texture or a same-resolution processed texture as the final output.
- Avoid palette quantization by default.
- Avoid downsampling by default.
- Avoid `Closest` texture filtering by default.
- Avoid `Pixelated_512` naming for clean outputs.
- Keep the old retro output as a separate compatibility mode.

This avoids breaking existing assets and makes the pipeline honest: retro pixelation becomes an opt-in presentation layer, not the baked source identity.

## Runtime FX Defaults

The fresh runtime settings are not clean. PS1 dither and pixelation strengths default to zero, but `bEnableRetroFXMaster=true`, `bUseRealLowResolution=true`, and `TargetResolutionHeightPercent=40`. That means a fresh/default path can still lower scene resolution.

For ToonStyle validation, Pablo should test with real low resolution disabled or master off. Otherwise the team will keep confusing source asset grain with runtime presentation.

## Character Test Surface

Use a static QuadRetro NPC first, preferably Gambler. Do not start with `Hero_1_Chad`.

`Hero_1_Chad` is now skeletal/animated and carries extra risk from UAL retargeting, skeletal import settings, animation material paths, and the "do not assign normalized static atlas" guard documented in the rigging notes. Gambler is simpler and still production-relevant.

## Environment Test Surface

Do not use a handpicked visual-only showcase for the first environment test. Use one generated wall/floor module from the Coherent Theme Kit path or a procedural dungeon module. The point is not to make one object pretty. The point is to confirm the procgen environment asset path can carry clean toon material assumptions.

## Hidden Risk

The biggest hidden risk is topology/normal quality after Quad Remesher. Proxy normal transfer can improve face/body shading, but generated meshes have inconsistent forms. One proxy strategy will not work for every body, robe, hair, monster, and prop. This is solvable, but it argues for a narrow first test pair and a measured quality bar.

The second hidden risk is data-table and importer assumptions around `PixelatedTextureAssetPath`. That name is now semantically wrong for ToonStyle but operationally embedded. The first clean branch should avoid broad data model churn by adding a parallel binding path for the test rather than renaming the existing field everywhere.

## Bottom Line

The Blender-heavy approach is correct. Use UE5 material work for fast feedback, but do not commit to Phase 1 shader architecture until a clean DCC branch proves that T66 can import and render a mesh with:

- clean texture output,
- preserved or transferred normals,
- a defined vertex color layout,
- no forced pixel atlas,
- no runtime low-resolution masking during validation.

That is the real foundation. The shader is the renderer over that data.
