Result: OK

## Summary
The Codex draft is solid and approval-ready: all 20 idols (5 elements × 4 categories) covered in the accepted table format, with Black→Red→Yellow→White scaling via count/scale/timing/density rather than color, and the preserved concept directions mapped to the primitive kit. Both my independent pass and this draft agree no user decision is required to produce the plan. A few small consistency/wording patches below.

## Suggested Answer Patch
- Fire Bounce hardcodes "Four" ember spheres at every tier, while all other Bounce rows use relative language ("a few", "several"). This silently asserts a fixed hit count. Either drop the literal "Four" to match the others, or apply the build note's caveat that visible count should follow gameplay hit count. Recommend changing "Four small ember spheres" → "small ember spheres (count following gameplay hit count)".
- Bounce rows should state the falloff intent (later hops shrink), which the CSV `FalloffPerHit` supports. Add a short clause, e.g. to each Bounce White cell or once in Build Notes: "later hops scale down per hit falloff."
- Add a one-line note distinguishing the 2D sprite icon columns (inventory icons) from the in-world VFX this plan targets, so the user doesn't confuse the two.

## Issues To Fix
- Minor internal inconsistency (Fire "Four" vs relative counts elsewhere) — Codex can resolve in text.
- Falloff-per-hit not reflected in Bounce scaling notes.

## Question For User
None required to produce the plan. (Optional, non-blocking: whether to lock hard per-tier shape counts now or leave as build-time tuning — does not block approval.)

## Evidence Or Verification Gaps
- CSV-anchored numbers (AoeRadius 300 / AoeDelay 0.2, DotTickInterval 0.5 / DotDuration 3, ProjectileSpeed ~2200–2400, FalloffPerHit on Bounce) were used in my independent pass; Codex's draft does not cite them but its qualitative claims are consistent with them. No contradicting evidence found.

## Notes
Scope respected — planning-only, no asset/imagegen/Unreal edits. Wind AOE movement-circle vs damage-radius distinction is correctly preserved across all tiers. Ice "no crystal spikes" constraint honored. Patches above are wording/consistency only; Codex can apply them without further inspection.
