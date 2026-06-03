Result: OK

## Independent Answer

No Codex draft plan exists in the artifact folder yet (only `original_prompt.md` and the review prompt), so this is my independent baseline for what the plan must contain. The prompt is fully actionable as planning-only — every consequential decision (purple Electricity, ice-blue Ice, normalize-IDs-first, drop legacy-ID dependence, shapes chosen later "one by one") is already settled by the user. No user gate is required to *write the plan*; the per-shape choices come after and are explicitly sequenced by the user. Hence OK.

A correct implementation plan should include, in this order:

1. **ID normalization first (true prerequisite).** Define the canonical idol ID set (Fire/Ice/Electricity/Nature × DOT/AOE/Pierce/Bounce), a single normalization function applied before any temp-visual lookup, and removal of legacy aliases as lookup keys. Sequence this before any visual matrix work so downstream slots key off canonical IDs only.
2. **Color helper changes.** Electricity yellow→purple, Ice→ice-blue, in the existing color helper. Confirm no other system (UI tints, damage-number colors) consumes the same Electricity color and would regress — call out the audit step rather than assuming isolation.
3. **Hero 1 AOE shape differentiation.** Four AOE rows (black/red/yellow/white) need distinct AOE shapes/patterns. Plan should expand the projectile helper beyond its current 4 hero profiles to carry per-row shape data, and define where shape geometry is authored (placeholder simple shapes only — final VFX still governed by PPF/manifest/parity).
4. **Idol visual matrix + the rarity dimension (biggest scope risk).** 16 canonical rows × 4 rarity = 64 conceptual variants, but traveler profiles (slots 4–19) and the single idol overlay profile have **no rarity dimension** today. The plan must explicitly decide: is rarity a new authored slot axis (64 slots, real perf/authoring cost) or a runtime modifier (scale/intensity/tint) layered over the existing 16? This is the one decision most likely to be under-specified — Codex should pick one and justify it, not leave it implicit.
5. **Process compliance.** State that these are infrastructure placeholders only; final Niagara/material VFX stays under the combat-VFX contracts (PPF, artifact parity, mechanism manifest, impact-context proof, visual/damage alignment, Unreal-owned capture, staged verification). Plan should respect `Gameplay/GAMEPLAY_AGENTS.md` and read `CombatVFXAuthoringProcedure.md` before any later asset work.
6. **Sequencing/staging.** Reflect the user's "one by one on the shapes" — plan delivers infrastructure + matrix scaffolding, then each shape lands as a separate reviewed step.

## Evidence Checked
- `original_prompt.md` and the pass-1 review prompt (no draft plan present).
- `Gameplay/GAMEPLAY_AGENTS.md` — combat ownership, read-first/VFX-authoring rules.
- `Gameplay/Combat/` index incl. `OutgoingTravelerVisualProfilesAuthoring.md` — confirms 16 element/delivery slots (4–19), no rarity axis, single Niagara selector, Phase-2 visual-proof standard.
- Grepped `Source/` for Electricity/Ice/color/legacy usages (broad spread across UI + gameplay) — supports the "audit other color consumers" step.

## Questions Or Blockers
None requiring the user. The deferred per-shape selections are by the user's own design ("one by one") and do not block delivering the plan.

## Caveats
- I could not literally review a Codex plan because none is in the folder yet; the above is the standard it should be measured against.
- Rarity dimension (16→up-to-64) is the highest-risk under-specification — verify the plan commits to slot-axis vs. runtime-modifier and notes the traveler-pool perf implication.
- Electricity color change needs a consumer audit (grep showed many color sites); don't assume the helper is the only source of truth.
- Temp shapes must not drift into final-VFX work; confirm the plan keeps the contract boundary explicit.
