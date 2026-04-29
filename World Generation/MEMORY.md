# World Generation Memory

Last updated: `2026-04-19`

## 1. Purpose

This file is the rolling implementation memory for `T66` world-generation research.

Update it whenever:

- Tencent releases more of `HY-World 2.0`
- a local experiment is run
- a T66 adoption decision changes
- a blocker or license issue is found
- a new recommended workflow replaces an older one

## 2. Locked Decisions

- `HY-World 2.0` is being tracked as an **offline** world-generation / reconstruction pipeline, not as a direct replacement for T66’s runtime procedural terrain.
- The official GitHub repo and technical report are the source of truth for release status.
- As of `2026-04-19`, the only clearly public runnable component is `WorldMirror 2.0`.
- Claims that the full `HY-World 2.0` pipeline is already fully open-sourced should be treated as overstated unless the official repo changes.
- T66 should not commit production architecture around unreleased Tencent modules.
- Any serious production use needs a license check because the shipped license excludes the `EU`, `UK`, and `South Korea`.

## 3. What Was Learned So Far

### 2026-04-29

- Moved the modular dungeon wall/floor/ceiling kit process into this workspace:
  - [MODULAR_DUNGEON_KIT_PROCESS.md](C:/UE/T66/World%20Generation/MODULAR_DUNGEON_KIT_PROCESS.md)
  - [SHARED_ASSET_PIPELINE.md](C:/UE/T66/World%20Generation/SHARED_ASSET_PIPELINE.md)
- Locked the practical near-term world-generation direction to modular environment-kit generation instead of unreleased full-room HY-World generation.
- Cleaned the runtime map path so the old terrain-variant asset family is no longer a live world-generation dependency.
- Confirmed the existing [World Generation](C:/UE/T66/World%20Generation) folder is the canonical home for this work.
- Refreshed official HY-World release status from the public GitHub repo:
  - `WorldMirror 2.0` code and weights are still the public runnable path
  - full world-generation inference, `HY-Pano 2.0`, `WorldNav`, and `WorldStereo 2.0` are still listed as coming soon
  - `hyworld2/worldgen/README.md` still identifies the world-generation module as not open-sourced in the repository
- Created [SETUP.md](C:/UE/T66/World%20Generation/SETUP.md) for RunPod/HY-World setup.
- Created [ROOM_GENERATION_PROCESS.md](C:/UE/T66/World%20Generation/ROOM_GENERATION_PROCESS.md) for the first dungeon-room prototype.
- Repo seam for the runtime test:
  - keep `T66TowerMapTerrain` as the normal tower generator
  - after `AT66GameMode::SpawnMainMapTerrain()` caches `MainMapSpawnSurfaceLocation`, use that cached value as the portal destination
  - spawn the player in a generated-room entry layer only for the dev prototype
  - use an explicit-destination portal instead of reusing `AT66TeleportPadInteractable` directly, because the current teleport pad chooses a random destination pad

### 2026-04-19

- Created the top-level folder [World Generation](C:/UE/T66/World%20Generation).
- Read existing T66 master-doc patterns and matched this folder to the repo’s current documentation style.
- Pulled the official `HY-World 2.0` repo, documentation, technical report, and license into local research cache under [tmp/worldgen_research](C:/UE/T66/tmp/worldgen_research).
- Confirmed the four-stage architecture:
  - `HY-Pano 2.0`
  - `WorldNav`
  - `WorldStereo 2.0`
  - `WorldMirror 2.0` + world composition / `3DGS`
- Confirmed the public-code boundary:
  - `WorldMirror 2.0` code/weights are public
  - `panogen` and `worldgen` are not meaningfully open yet in the repo
- Confirmed the T66-local boundary:
  - the game already has runtime procedural terrain and map assembly
  - `HY-World 2.0` is more naturally an upstream asset-generation pipeline than a runtime map system
- Confirmed the most immediate T66 experiment path:
  - use `WorldMirror 2.0` first on controlled multi-view or video captures

## 4. Recommended Immediate Next Steps

- Continue `DungeonKit01` as the active production-facing experiment:
  - generate/import more modular wall and floor variants
  - keep generated meshes visual-only
  - keep hidden simple proxies collision-authoritative
  - move broad runtime kit selection toward a data-authored registry after the prototype stabilizes
- Run one local `WorldMirror 2.0` reconstruction test on a controlled scene:
  - either Unreal editor screenshots from a blockout
  - or a short multi-view / video capture of a scene we already understand
- Record whether the outputs are useful as:
  - point-cloud reference
  - Gaussian-splat reference
  - camera/layout reference
  - mesh proxy after cleanup
- If generation experimentation is needed before Tencent opens the full stack, evaluate older preview branches separately and label them as temporary research only.

## 5. Current Risks

- The official full world-generation stack is still incomplete in public code.
- The report’s runtime numbers are on `NVIDIA H20` hardware and do not guarantee easy local operation on consumer GPUs.
- Even if world generation looks strong visually, gameplay-grade collision quality may still require heavy cleanup.
- Multi-stage pipelines can accumulate error across panorama generation, planning, expansion, and reconstruction.
- The license has territory and scale restrictions that could matter later.

## 6. Open Questions

- How useful are `WorldMirror 2.0` outputs for actual Unreal-facing level reference work inside T66?
- Is the mesh extracted from generated / reconstructed data clean enough for collision proxies after a reasonable cleanup pass?
- How much manual repair would generated worlds need before they are better than current authored or procedural terrain workflows?
- Once Tencent releases the full stack, how much of it can realistically run on our own hardware versus hosted infrastructure?
- Does the eventual full release preserve enough camera and geometry control for gameplay-space prototyping rather than just visual demos?

## 7. Rules For Future Updates

- Record exact release dates when Tencent changes public availability.
- Distinguish clearly between:
  - officially released
  - described in the paper
  - inferred from summaries or discussion threads
- Log concrete experiment outcomes, not just intentions.
- If an older assumption becomes wrong, replace it here and note why.
- Keep this file current enough that another agent can resume the work without rereading the full report.
