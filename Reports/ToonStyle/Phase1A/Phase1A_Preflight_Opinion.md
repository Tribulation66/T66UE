# Phase 1A Preflight Opinion

## Position

Do not implement all ten new assets as the first Phase 1A batch. Keep the ten-asset room as the final Phase 1A acceptance target, but stage the work. The infrastructure and feedback loop must come first.

The reason is not caution for its own sake. The ten requested entries are not ten copies of the same pipeline problem. They hit different systems: hero visual selection, companion skeletal assets, static NPC visuals, VAT mobs, world interactables, pickups, arcade data, crate/chest overlays, and custom room surfaces. If all ten are generated before the TEST room and import/binding path are proven, Pablo will be judging many variables at once and it will be hard to know whether a bad result came from prompt design, Pixal3D, import settings, material assignment, runtime post-process, or the wrong data seam.

## TEST Room Infrastructure Should Come First

The strongest existing seam is Lab, but Lab is a practice room with The Collector, not a ToonStyle review space. I would build a dedicated TEST branch that still opens `GameplayLevel`, similar to Lab, and exits before normal tower generation. That avoids creating a new map while keeping the room deterministic.

I would not use the normal tower or procedural dungeon system for this. The whole purpose is a controlled visual lab: fixed camera-facing lineup, fixed lighting/post-process state, known surface textures, and predictable asset placement. Reusing full tower generation would add noise.

## Scope Size

Ten assets is too much for the first implementation batch. It is the right final coverage set because it spans the major asset classes, but it is a bad first unit of work because quality iteration will be slow and ambiguous.

The first asset loop should prove:

- one room surface material with one texture,
- one humanoid/static character-like model,
- one world prop,
- import/bind into Unreal,
- view in TEST room with retro disabled.

After that, expand in batches.

## Cuboid Room

The cuboid room is the right environment validation target, but I would smoke-test one textured wall or one textured plane before committing to six final room textures. Cube UVs and texture stretching can distort art in ways that look like bad generation when the real issue is material/UV setup. The room should graduate to six textures only after one surface looks correct at runtime.

A full cuboid enclosure is still better than a single plane for acceptance because Pablo wants to judge whether 2D art on simple geometry can replace generated 3D wall modules. But the first proof should be even smaller: one wall and one floor, then the full room.

## Asset Binding Risks

The biggest risk is not Pixal3D alone. It is binding generated assets into the correct runtime seams without invalidating the visual judgment.

Lu Bu and ARIA are especially tricky because production-quality validation wants skeletal assets, while the fastest visual loop wants static display meshes. If Phase 1A tries to solve skeletal generation, animation, clean normals, toon material assignment, and the test room at the same time, it will sprawl. For the first pass, display-only static actors are acceptable for visual style validation, but the report should be honest that this does not prove the final live hero/companion path.

Mobs are similar. Slime, CaveBat, and TombSpider have static visual rows, but production runtime also has VAT rows. Static display is good enough to judge the generated model and ToonStyle surface look. It is not enough to declare the live mob presentation solved.

## Pixal3D Risk

Pixal3D should be used, but not treated as a clean ToonStyle source until proven on a tiny batch. The server exposes texture size, resolution, remesh, decimation, and sampling/guidance headers. It does not expose an obvious "remove texture grain" setting. If texture grain is model-generation noise, the fix is prompt and generation-parameter iteration. If grain is introduced downstream by QuadRetro, the fix is the clean branch. Either way, bulk-generating ten assets before proving clean output is wasteful.

## QuadRetro Clean Branch

If the pipeline still routes Pixal3D assets through QuadRetro for retopo, rebake, or normalization, the clean flat-color output mode must be in place before the ten-asset batch. Otherwise Pablo will keep seeing baked pixelation and the TEST room will fail to answer the actual question.

The right framing is not "non-pixelated" as a one-off hack. It is "clean source output, with optional pixelation later." That preserves Phase 6 as an overlay and gives ToonStyle a clean asset base.

## Highest-Risk Part

The highest-risk part of Phase 1A is not adding the TEST button. It is building a fast, trustworthy visual iteration loop. If Pablo cannot enter the same room, see the same lighting, compare assets in stable positions, and know that retro effects are off, every quality discussion will be contaminated.

The second-highest risk is ARIA/hero skeletal expectations. If Pablo expects generated Lu Bu and ARIA to be fully production-rigged in the first Phase 1A pass, that should be called out as a separate, larger goal. Static visual review and production character integration are different milestones.

## Direct Recommendation

Phase 1A should be staged:

1. Build the TEST entry and empty review room.
2. Prove one textured room surface and one static generated asset.
3. Prove the first character-like asset plus one prop.
4. Add the remaining categories in small batches.
5. Only then do the complete ten-asset lineup pass.

This still honors Pablo's comprehensive vision. It makes the comprehensive room the gate, not the first blind batch.
