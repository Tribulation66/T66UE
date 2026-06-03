# Phase 1A Recommended Scope

## Recommendation

Use the ten-asset TEST room as the Phase 1A acceptance target, but split implementation into gated sub-phases. Pablo should approve or modify this sequencing before Claude drafts the implementation prompts.

## Phase 1A.0 - TEST Route And Empty Room

Goal: make the review loop exist before generating assets.

Scope:

- Add a `TEST` button below `MODS` on the hero selection screen.
- Route it to `GameplayLevel`, not a new map.
- Add a dedicated test-room branch or state, modeled after Lab but not reusing The Collector flow.
- Spawn a small empty cuboid room from code using simple cube actors.
- Spawn the player at a stable position.
- Ensure the room is leaderboard-ineligible.
- Disable retro master and real low-resolution presentation for this test flow.
- Do not import or bind new generated assets yet.

Exit criteria:

- Pablo can click TEST from hero selection and land in a neutral empty room.
- The normal ENTER, TUTORIAL, CHALLENGES, MODS, and LAB flows still behave as before.
- The TEST room does not run normal tower generation or spawn The Collector.

## Phase 1A.1 - Room Surface Proof

Goal: prove the "2D texture on simple geometry" environment idea before building the whole room art set.

Scope:

- Import or create one test wall texture and one test floor texture.
- Bind them to the cuboid room material path.
- Verify scale, orientation, UV stretching, texture filtering, and crispness in the TEST room.
- Keep the material simple and ToonStyle-friendly. Avoid QuadRetro pixelation for these textures.

Exit criteria:

- One wall and one floor look correct at runtime.
- Texture stretching/tiling is understood.
- The implementation path is ready for six final textures.

## Phase 1A.2 - First Full Pipeline Pair

Goal: prove one character-like asset and one world prop through image generation, Pixal3D, import, binding, and TEST-room display.

Recommended pair:

- Lu Bu display mesh as the humanoid/hero proxy.
- Idol altar as the prop.

Reasoning:

- Lu Bu is important to Pablo's target but can initially be judged as a static display mesh without solving skeletal production integration.
- Idol altar already has Pixal3D/default generated-asset seams and is the current display anchor for Pixal test models.

Exit criteria:

- Both assets appear in fixed positions in the TEST room.
- Textures are clean enough to judge the ToonStyle direction.
- Any grain is traced to Pixal3D generation, QuadRetro processing, import/material handling, or runtime effects.

## Phase 1A.3 - Stage 1 Enemy Batch

Goal: cover the first live enemy category.

Assets:

- `Slime`
- `CaveBat` for Pablo's "Bat"
- `TombSpider` for Pablo's "Spider"

Scope:

- Generate and import static display meshes first.
- Bind them into the TEST room lineup.
- Do not require VAT integration in this sub-phase unless Pablo explicitly makes animated enemy runtime equivalence a gate.

Exit criteria:

- The three Stage 1 enemy models are visible and comparable in the room.
- Pablo can judge prompt consistency across small, flying, and multi-legged enemy shapes.
- The report for this sub-phase states whether VAT integration remains pending.

## Phase 1A.4 - Companion And NPC Batch

Goal: cover character-like non-hero actors.

Assets:

- ARIA (`Companion_01`)
- Gambler

Scope:

- Generate/import static display meshes for review.
- For Gambler, optionally wire through the existing static NPC visual path after the display mesh passes visual review.
- For ARIA, keep production skeletal integration out of scope unless Pablo explicitly asks for it in this phase.

Exit criteria:

- ARIA and Gambler are visible in the TEST room.
- The team has a clear decision on whether ARIA's final production path needs rigging before Phase 1B material work.

## Phase 1A.5 - World Interactable Batch

Goal: cover the object/interactable classes after the visual pipeline is proven.

Assets:

- Arcade machine
- Loot chest
- Loot bag, one chosen rarity first
- Loot crate, meaning the world `Crate` actor unless Pablo says otherwise
- Idol altar if it was not finalized in Phase 1A.2

Scope:

- Bind each as a static display object first.
- Only wire full gameplay interaction when the static visual passes.
- For chest/crate, use showcase-reusable behavior or a display actor so eyeball testing does not destroy the object.
- For loot bag, pick one rarity for first validation, then decide whether all four rarity colors need separate generated visuals.

Exit criteria:

- All requested world objects are represented in the TEST room.
- Each object has a documented binding path back to its production actor or data row.

## Phase 1A.6 - Full Acceptance Lineup

Goal: make the room match Pablo's full stated success target.

Scope:

- Six final room textures: floor, ceiling, north wall, south wall, east wall, west wall.
- Ten final assets:
  - Lu Bu
  - ARIA
  - Slime
  - CaveBat
  - TombSpider
  - Idol altar
  - Arcade machine
  - Loot chest
  - Loot bag
  - Loot crate
  - Gambler

Note: Pablo's list says "ten" but names eleven entries if "four world objects" includes idol altar, arcade machine, loot chest, loot bag, and loot crate. The implementation prompt should resolve this count explicitly. My recommendation is to include all eleven named entries unless Pablo removes one.

Exit criteria:

- Pablo can click TEST and review the complete lineup under stable room conditions.
- Retro low-resolution and master retro effects are off for the test flow.
- The remaining gaps are clearly separated into Phase 1B character material, Phase 1C environment material, VAT/skeletal integration, and Phase 6 retro overlay.

## Gate Before Drafting Phase 1A Implementation

Pablo should decide:

- Whether `TEST` should be a new `ET66RunCategory::TestRoom` or a transient test flag.
- Whether Lu Bu and ARIA can be static display meshes for Phase 1A visual approval.
- Which loot bag rarity to generate first.
- Whether the final acceptance room should contain all eleven named entries despite the "ten assets" wording.

Without these decisions, Claude can still draft 1A.0, but should not draft the full asset-generation prompt.
