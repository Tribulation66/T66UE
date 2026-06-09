You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\IdolCarrierShapeImprovementPlan\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\IdolCarrierShapeImprovementPlan\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\IdolCarrierShapeImprovementPlan\20260605T113336-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Ok how can it be improved while still using basic shapes? So that its more faithful to our vision, and I do want each single projectile to have its own carrier shape because eventually we will need 20 different niagra systems for each idol maybe 80 since we have 4 rarities for each one.

Working task:
Operator: Codex
Validator: Claude
Scope: Propose how to improve idol projectile carriers so each of the 20 idols has a distinct basic-shape in-flight silhouette, while keeping the temporary/basic-shape approach and preparing for future 20 or 80 Niagara systems.
Stop condition: Provide a repo-grounded implementation plan and call out the current architecture gap clearly.

</original_prompt>

<validator_independent_answer>
I've verified the architecture against the live source. Codex's core claims hold.

Result: OK

## Independent Answer

The improvement path is sound and the architecture gap is real and correctly diagnosed. Concretely:

**The gap (verified):** `T66OutgoingTravelerPoolSubsystem.cpp:58-80` maps all 20 `TravelerVisual.*` profile IDs onto only **4** primitive mesh indices by category (AOE=0/Sphere, Pierce=1/Cone, DOT=2/Cylinder, Bounce=3/Cube — see lines 60-79 and `GetMeshIndexForTemporaryProjectileShape` at line 276). `UploadLiveState` (line 902) pushes exactly **one** position/rotation/scale/color/meshIndex row per live traveler. So today every Fire/Ice/Electricity/Nature/Wind idol of the same category is literally the same shape, differentiated only by color. There is no per-idol silhouette.

**The fix that keeps basic shapes:** Add a visual-only **carrier recipe** layer keyed by profile ID (later `IdolID + Rarity`), where each recipe is a list of primitive parts (mesh index + local offset/rotation/scale/color). In `UploadLiveState`, expand each logical traveler into N upload rows — one per part. Because the Niagara pool already renders an arbitrary count of indexed primitive instances per frame, multi-part composites need **no new gameplay simulation**: collision, arrival callback, and damage stay bound to the single parent traveler. This is the only way to get 20 distinct silhouettes from 4 meshes without standing up per-idol Niagara systems now, and the same `IdolID + Rarity` keys later resolve to the 20/80 real systems. Codex's 20-silhouette catalog and rarity ramp (Black=cleanest → White=full recipe) are reasonable and directionally faithful.

**Two things Codex under-weighted, fix before answering (still OK):**
1. **Capacity scaling.** Upload arrays size to `LiveCount = DenseSlots.Num()` and the pool is bounded by `MaxOutgoingTravelers` / `User.TravelerPoolCapacity`. Multi-part expansion multiplies *upload rows* by parts-per-recipe even though logical travelers are unchanged. The Niagara array params and pool capacity must be sized for `maxLiveTravelers × maxPartsPerRecipe`, or high-recipe idols at swarm density will clip. Codex's plan should state this explicitly.
2. **No `Rarity` field exists yet** in `FT66OutgoingTravelerFireParams` (`.h:57-101` — only `ProfileID`, `TravelerVisualProfileID`, `Color`, `ScaleMultiplier`). Codex's step 5 ("add rarity to fire params or fold into the profile ID") is correct but should be flagged as net-new plumbing, not an existing hook. Folding rarity into an expanded `TravelerVisualProfileID` namespace is the lower-risk start.

## Evidence Checked
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp:50-191` — 20→4 profile/mesh table, fire path.
- `:276-318` — shape→mesh fallback (4 primitives) and profile lookup.
- `:902-961` — `UploadLiveState`, one row per traveler, 5 Niagara array params.
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.h:57-101` — `FireParams` has no `Rarity`.
- `codex_draft.md` and `Content/VFX/Foundation/OutgoingTravelers/NS_OutgoingTravelerPool.uasset` (single shared pool confirmed).

## Questions Or Blockers
None requiring the user — this is a design/planning answer the models can deliver. The user has already decided the direction (basic shapes, per-projectile carrier, future 20/80 Niagara).

## Caveats
- Did not open `NS_OutgoingTravelerPool` internals to confirm the mesh-index renderer supports the full part count per recipe; plan should verify the Niagara mesh-renderer array binding handles the inflated row count before approving recipes.
- The 20 silhouette descriptions are subjective "faithfulness" calls the user should still eyeball via the proposed contact-sheet capture; validator can't judge art-vision fidelity from code.
- Capacity/perf at swarm density is the main technical risk and is not yet quantified.

</validator_independent_answer>

<codex_draft>
Codex draft answer for cross-review:

The current four-slot profile map should be replaced by a per-idol carrier recipe layer.

Current gap:
- `T66OutgoingTravelerPoolSubsystem.cpp` maps 20 visual profile IDs to 4 mesh slots by category.
- The existing Niagara pool uploads one position, rotation, scale, color, and mesh index per logical traveler.
- This can only produce one primitive mesh per traveler, so it cannot faithfully represent 20 unique carrier silhouettes if the carriers need to look different by idol, not just color/category.

Recommended improvement:
1. Keep the outgoing traveler gameplay simulation logical and unchanged: one traveler, one target, one arrival callback, one damage/event path.
2. Add a visual-only carrier recipe layer keyed by `IdolID + Rarity`, with fallback to `IdolID`.
3. Each recipe is a compact list of primitive parts: mesh shape, local offset, local rotation, local scale, color multiplier, and optional orbit/spin hint.
4. During `UploadLiveState`, expand each logical traveler into N visual upload rows. The Niagara pool still renders primitive mesh indices, but it renders multiple parts per logical traveler. Collision and arrival stay bound to the parent traveler only.
5. Add rarity to the traveler fire params or fold it into the visual profile ID so Black/Red/Yellow/White can use different recipes later. Start with 20 base recipes and a rarity multiplier/count curve, then split to 80 explicit recipes when the shapes are approved.
6. Keep activation/impact VFX separate from carrier VFX. Carrier is the thing flying. Activation is the explosion/burn/freeze/shock/root/tornado effect at impact.

Suggested 20 basic carrier silhouettes:
- Fire AOE: compact fire bomb, center sphere plus upward cone crown and small ember satellites.
- Fire DOT: flame seed, vertical cone flame plus two small body-orbit sparks.
- Fire Pierce: fire lance, long cone/cylinder spear with ember nodes along the shaft.
- Fire Bounce: ember cluster, three small spheres/cubes staggered like snapping sparks.
- Ice AOE: snowflake puck, flat cylinder disc with cone spikes around the edge.
- Ice DOT: frost capsule, sphere plus cube crystal shell.
- Ice Pierce: icicle spear, long thin cylinder with cone point.
- Ice Bounce: shard fan, three mini cones in a V cluster.
- Electricity AOE: storm node, central sphere with three short vertical/diagonal bolt rods.
- Electricity DOT: shock cage, small core with crossing zig rods.
- Electricity Pierce: lightning bolt, segmented zig-zag cylinders/cubes.
- Electricity Bounce: chain spark, diamond cube with short rods at both ends.
- Nature AOE: seed pod burst, center sphere with radial branch/root rods.
- Nature DOT: spore pod, clustered small spheres around a core.
- Nature Pierce: ground root, low horizontal cylinder chain with bulb nodes.
- Nature Bounce: seed dart, oval seed body with small cone point.
- Wind AOE: orbit tornado seed, stacked cones/cylinders in a squat spiral.
- Wind DOT: dust devil seed, small vertical cone stack.
- Wind Pierce: skinny tornado spear, narrow stacked cone spiral stretched forward.
- Wind Bounce: mini whirlwind, tiny stacked cone spiral with side offset.

Rarity handling:
- Black: shortest/cleanest version of the silhouette.
- Red: add one secondary part group or stronger asymmetry.
- Yellow: add more shards/branches/sparks/stack layers.
- White: larger and cleaner silhouette, full recipe, not just more clutter.

Verification plan:
- Update the visual profile gate to assert 20 unique carrier recipe signatures, not just 20 profile IDs.
- Add an isolated carrier-shape capture that shows all 20 in-flight carriers in a contact sheet or short clip.
- Compile editor, refresh staged standalone, run the visual profile gate from staged exe.

Conclusion:
This gives each idol its own basic-shape projectile now, and the exact same `IdolID + Rarity` profile keys can later resolve to 20 or 80 real Niagara systems.

</codex_draft>
