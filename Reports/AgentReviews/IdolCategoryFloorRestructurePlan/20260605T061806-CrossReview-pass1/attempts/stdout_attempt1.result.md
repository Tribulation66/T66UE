Result: OK

## Summary
Codex's draft is a strong, planning-only answer that correctly leads with the three real ambiguities (idol-count math 15 vs 16, Pierce removal scope, altar-interaction proof) and matches my independent findings. The dependency-ordered plan, save-migration handling, and the five user questions are well-formed. No blocking issues; the draft is shippable with a couple of small precision fixes.

## Suggested Answer Patch
- In Lead Finding 3 and Plan step 6, make the floor renumber explicit: current Start=1 / Mob=2-4 / Boss=5 becomes Start=1 / Mob=2-3 / Boss=4 (cutting old floor 3 leaves two mob floors). Codex states this conditionally ("if ... two gameplay floors") — state it as the concrete target so the user can confirm directly.
- Add the stage-entry altar nuance to the interaction-math question: the live tree has a stage-4-only floor-1 stage-entry altar in `T66GameMode_WorldInteractables.cpp` *in addition to* gate-guardian altars, so the raw pre-cut count is not a clean 16. Codex mentions the stage-4 altar in findings but doesn't carry it into Question 5 — fold it in so the user knows the "16" baseline itself needs definition, not just the post-cut "12."

## Issues To Fix
- None blocking. The two patch items above are precision improvements, not corrections.

## Question For User
None required at the validator level — the user-facing questions belong inside Codex's plan (which already lists all five correctly). This stays Result: OK.

## Evidence Or Verification Gaps
- Neither model traced the per-difficulty altar total to a single authoritative number (altar instances vs. `RemainingSelections`/selection budget vs. equipped-slot count). Both correctly defer this to a user definition of "interaction" before committing the floor cut — acceptable for planning-only, but Codex should keep this as an explicit pre-implementation verification gate, not assume the floor change alone proves 12.
- Codex's "16" pre-cut baseline is asserted as inconsistent but not pinned to exact altar sources; my count (12 gate altars + 1 stage-4 stage-entry = ~13, not 16) suggests the baseline itself is unconfirmed. Flag that the "16→12" framing may rest on a count the user has in mind that doesn't match live spawn logic.

## Notes
- Codex's scope-split on Pierce (keep `ET66AttackCategory::Pierce` for Hero 2/Hero 8 weapons vs. global removal) is the right framing and slightly more careful than my independent answer, which leaned toward re-pointing the weapons. Codex's version is safer — keep it.
- Both answers agree the enum/data-type layer must land first to surface dependent compile sites. Good sequencing.
