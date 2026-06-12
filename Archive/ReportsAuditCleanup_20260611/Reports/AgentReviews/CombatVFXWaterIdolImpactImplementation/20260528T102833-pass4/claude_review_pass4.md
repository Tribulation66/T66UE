Verdict: REVISE

## Blockers

- None that block the structural claim outright, but see Major Issues — at least one apparent log/claim contradiction must be reconciled before presenting this as greenlit.

## Major Issues

- **`Applied=false` vs. `DamageBySource Idol_Water TotalDamage=36` is unresolved on its face.** The packet states "Water applies damage under `SourceID=Idol_Water`" and lists a binding new idol-owned target query, but the proof log line `CombatIdolWaterImpactResolved ... Applied=false Reason=LegacyImmediatePreserved` says the new path explicitly did not apply damage. If the `Idol_Water` 36 damage is actually coming from the legacy immediate path that was relabeled with the new SourceID, then the "idol-owned damage source" claim is *structural only* (context published, query run, source tagged) and the application is still routed through the legacy path. That nuance must be stated plainly in the final summary, or the contradiction reads as a discrepancy between claim and evidence.
- **Pre-existing unrelated edits in `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md` were integrated, not isolated.** The packet flags this as a "known caveat" but does not show that the impact-context diff is separable from those pre-existing changes. For a structural foundation that other idols will key off, the commit hygiene matters — reviewers cannot cleanly attribute later regressions to this change vs. the carried-over edits.
- **Idol-owned chaining seam is announced but not exercised.** The packet says "Water owns its own impact point for future chaining" and that an idol Niagara binding helper "currently falls through to placeholder." Neither chaining nor binding-resolution-to-real-asset is proven in this pass. That is acceptable as deferred scope, but the final summary should not imply the seam has been validated end-to-end — only that it compiles and is reachable via the placeholder branch.

## Minor Issues

- `AoeDelay=0.150` is logged but the runtime timing issue is explicitly out of scope. Confirm the pending-issues entry names the symptom, the file/log line, and the deferral owner so it does not get lost.
- `T66Hero1AxeAOEVFXLabActor.cpp(353,3) C4996 FNiagaraEmitterInstance::IsReadyToRun` is described as pre-existing; worth a one-line cross-reference to where it is already tracked, so a future reader doesn't relitigate it under this change.
- "Earth neutral proves no leakage" rests on the *absence* of two log tokens (`CombatIdolWaterImpactResolved`, `CombatVFXIdolImpactPlaceholderSpawned`). That is a reasonable negative test but is only as strong as the test scenario coverage — confirm Earth was actually triggered through an analogous AOE path so the absence is meaningful, not vacuous.
- The blue sphere is described as Water-only and structural. Make sure the temporary placeholder is gated behind an explicit feature/branch that is trivially removable when Water Niagara is authored, not woven into the impact-context plumbing itself.

## Clarifying Questions

- For Water: is the `Idol_Water` 36 damage applied by the **new** impact-context path or by the **legacy immediate** path with relabeled source ID? Which code path actually called `ApplyDamage`?
- Is `Applied=false Reason=LegacyImmediatePreserved` intended as the steady-state for this pass, or is it a transitional log that will flip to `Applied=true` in a follow-up once legacy is retired? If transitional, where is that follow-up tracked?
- Were the pre-existing edits to `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md` reviewed/owned elsewhere, or are they entangled with this change such that reverting this work would also revert them?
- Does the idol Niagara binding helper have a test (even a unit-level one) confirming it would resolve a real binding if one existed, or is its correctness only inferred from the placeholder fallthrough?

## Required Verification

- Show the code path (file:line) where Water's `Idol_Water` damage is applied, so the proof log and the claim agree.
- Show the diff scoped to this task (e.g., `git diff` filtered to the new impact-context additions) demonstrating that the pre-existing edits in the two carried-over files are separable from the structural change.
- Confirm Earth's neutral proof exercised the same Hero 1 AOE attack class as Water (same code branch), so the negative-token assertion is non-vacuous.
- Confirm the placeholder spawn is reachable only under an explicit Water+no-binding condition and cannot leak to other idols even if a future idol context is added without authored Niagara.

## Rationale

The structural work (per-source impact context, weapon-vs-idol source ID, idol-owned impact point, idol-owned target query, Niagara binding seam, Water-only placeholder, Earth neutral negative test, staged build refreshed and smoke-checked) is well-scoped and matches the declared process. Build, validator, proof captures, staged shortcut verification, and log highlights are all present and consistent at the artifact level. The reason for `REVISE` rather than `APPROVE` is narrow: the `Applied=false` log line directly conflicts with the natural reading of "Water applies damage under `SourceID=Idol_Water`," and the carried-over edits in two files muddle attribution for a change that other idols will build on. Both can likely be resolved by clarifying language in the final summary plus a small diff-scoping demonstration — no rework of the implementation appears necessary.

