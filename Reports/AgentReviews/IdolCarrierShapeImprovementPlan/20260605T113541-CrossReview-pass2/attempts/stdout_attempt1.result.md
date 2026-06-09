Result: OK

## Summary
Codex's draft correctly diagnoses the architecture gap (20 profile IDs collapsed to 4 category meshes, one upload row per traveler) and proposes the right fix: a visual-only multi-part carrier recipe layer keyed by `IdolID + Rarity`, expanded into N upload rows in `UploadLiveState` while gameplay simulation stays single-traveler. The independent answer verified these core claims against live source. The plan is sound and faithful to the user's stated direction; it just needs two technical caveats added before it goes out.

## Suggested Answer Patch
Append to Codex's "Recommended improvement" section (after step 4):

> **Capacity scaling (must size for this):** Upload arrays size to `LiveCount = DenseSlots.Num()`, and the pool is bounded by `MaxOutgoingTravelers` / `User.TravelerPoolCapacity`. Multi-part expansion multiplies *upload rows* by parts-per-recipe even though logical traveler count is unchanged. Size the Niagara array params and pool capacity for `maxLiveTravelers x maxPartsPerRecipe`, or high-part idols at swarm density will clip. Verify the `NS_OutgoingTravelerPool` mesh-renderer array binding handles the inflated row count before approving recipes.

Patch step 5 wording (it implies rarity is an existing hook):

> 5. **Note this is net-new plumbing:** `FT66OutgoingTravelerFireParams` has no `Rarity` field today (only `ProfileID`, `TravelerVisualProfileID`, `Color`, `ScaleMultiplier`). Lower-risk start: fold rarity into an expanded `TravelerVisualProfileID` namespace rather than adding a parallel field; promote to an explicit `Rarity` field only if needed.

## Issues To Fix
- Capacity/perf scaling under multi-part expansion is unaddressed — the single biggest technical risk. Add it (patch above).
- Step 5 presents rarity as a near-existing toggle; it's net-new. Flag it and prefer the profile-ID-namespace start.
- Plan should explicitly say to confirm the Niagara mesh renderer supports the per-recipe part count before recipes are approved.

## Question For User
None. The user has already decided the direction (basic shapes, per-projectile carrier, future 20/80 systems); this is a planning answer the models can deliver.

## Evidence Or Verification Gaps
- The 20 silhouette descriptions are subjective faithfulness calls — only the user can judge art-vision fidelity; the proposed contact-sheet capture is the right validation step.
- `NS_OutgoingTravelerPool` internals were not opened to confirm the mesh-index renderer tolerates the inflated row count; verify before committing recipes.
- Swarm-density perf is not quantified — call it the primary open risk.

## Notes
Codex's draft and the independent answer agree on substance; no conflicting claims. With the two caveats folded in, Codex can finalize and send.
