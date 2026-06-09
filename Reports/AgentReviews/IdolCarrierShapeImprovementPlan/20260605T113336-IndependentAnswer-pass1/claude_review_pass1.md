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
