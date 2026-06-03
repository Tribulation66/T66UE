Verified Codex's key claims against live code — all accurate.

Result: OK

## Summary
Codex's draft is accurate and, where I spot-checked, better-evidenced than the independent answer. I confirmed: `GetIdolColor` returns Fire red-orange, Ice blue `(0.28,0.70,1.0)`, Electricity **yellow** `(0.85,0.72,0.12)`, Nature green (T66IdolManagerSubsystem.cpp:204-207); `CombatVFXBindings.csv` has 20 `WeaponBase` rows and 0 `IdolModifier` rows; `T66TemporaryProjectileSystem` and `T66OutgoingTravelerPoolSubsystem` exist. The draft correctly confirms the 4-weapon / 16-idol / 64-visual-variant structure, flags the lightning-purple-vs-yellow color conflict, and recommends a data-driven temp visual registry. Models can finalize internally — no user gate required.

## Suggested Answer Patch
- Add the **Ice color mismatch** alongside the lightning one: the user wants "very light blue" but code ice is medium blue `(0.28,0.70,1.0)`. Same "new convention needed" point, but be explicit it's two element colors diverging, not just Electricity.
- One-line note on the **legacy idol alias layer**: `NormalizeLegacyIdolID` maps ~28 legacy IDs (`Idol_Water`, `Idol_Storm`, `Idol_Lightning`, etc.) onto the 16 canonical `Idol_<Element>_<Category>` rows (T66IdolManagerSubsystem.cpp:138-167). This reinforces "16 canonical rows" but means any temp-registry keyed by `IdolID` must normalize first, or it will miss runtime aliases. Worth stating since the registry keying recommendation depends on it.

## Issues To Fix
- None blocking. The two patches above are additive precision, not corrections.

## Question For User
None required for the read-only deliverable. The three decisions Codex surfaces (temp-shape data home; idol-owned vs weapon-overlay attachment; weapons-black / idols-element-color split) are correctly framed as recommendations to raise in the answer, not as blockers — keep them in the body, not as a NEEDS_USER gate.

## Evidence Or Verification Gaps
- Codex's claim that `FT66TemporaryProjectileSystem` "forces hero/idol overlay profiles to the hardcoded hero blue" and that the traveler pool "has 16 `TravelerVisual.<Element>.<Category>` slots with no rarity dimension" — files exist and are plausible, but I did not read the specific lines. These drive the "not sufficient for the matrix" conclusion; if the answer leans on them, cite line numbers.
- The independent answer's "all rows `SourceType=WeaponBase`" is consistent with my count (20/20); resolved.

## Notes
Codex inspected more C++ seams than the independent pass and its claims held up under spot-check. Treat the draft as the stronger of the two and merge in the Ice-color and legacy-alias precision.
