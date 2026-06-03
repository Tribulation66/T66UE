Verdict: REVISE

## Blockers

- None of the issues below appear to corrupt the proven behavior, so no hard blocker. Promoting to BLOCK only if the file-scoping ambiguity (next section) turns out to hide task-owned edits in `T66PlayerController_Overlays.cpp`.

## Major Issues

- Mixed task/pre-existing edits in claimed file list. `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md` appear in "Files Changed" but are explicitly disclaimed as carrying pre-existing unrelated edits. The packet says the implementation was "integrated without reverting them," which leaves ambiguous whether any task-owned hunks were added on top of the pre-existing diff. For audit cleanliness, the packet should either (a) split the diff per file into task-owned hunks vs pre-existing hunks with line ranges, or (b) confirm those two files contain zero task-owned edits and should be removed from the changed-files list. As written, this defeats the purpose of "diff-scoping clarification."
- Authored `FIdolData::AoeDelay` is silently ignored. The Water proof shows `AoeDelay=0.150 DelayApplied=false Reason=LegacyImmediatePreserved`. The new idol impact branch claims idol-owned damage authority but does not honor a data-driven authoring field. This is logged as "out-of-scope runtime timing issue" but is functionally a regression seam: future authors editing `AoeDelay` will see no effect, and the new structure is the obvious place that contract should land. At minimum this should be called out in `pending_issues_Combat.md` with a follow-up ticket reference, not buried in a doc note.

## Minor Issues

- Scope wording drift. Working Goal says "all-weapons/all-idols impact-source infrastructure," but `UsesImpactPresentationForIdol` is currently gated to `Idol_Water` + AOE. The structure may be general, but the *consumer surface* is Water-only. Recommend the final summary explicitly say "infrastructure landed; Water is the only wired idol; other idols remain legacy until separately ported," which is accurate to the Earth neutral evidence.
- `C4996 FNiagaraEmitterInstance::IsReadyToRun` is noted as pre-existing but lives in `T66Hero1AxeAOEVFXLabActor.cpp`, which is in the same combat VFX area. Worth a one-line pointer to whichever issue tracks the UE 5.7 deprecation cleanup so it doesn't quietly regress into noise.
- Earth neutral control is solid, but the asserted "forbidden patterns absent" list only names `CombatIdolWaterImpactResolved` and `CombatVFXIdolImpactPlaceholderSpawned`. Add `Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water` to the forbidden set so the control proves the *new context branch* didn't fire, not just the legacy-suppression and placeholder spawns.
- The placeholder is described as Water-only but the binding helper is described as general. Confirm the helper's lookup key/path is namespaced (`Idol_<X>`) and won't accidentally resolve a non-Water binding to the blue sphere if a different idol asset is dropped in during authoring.

## Clarifying Questions

- Were any task-owned edits made to `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md`, or are those entries leftover noise that should be dropped from "Files Changed"?
- Is the `AoeDelay` honoring contract intended to land in this structure pass or explicitly punted to the Niagara authoring pass? If punted, where is the follow-up tracked?
- Is `ParentSourceID=<weapon source>` meant to be normative for all idol contexts, or Water-specific for now? Other idols porting to this seam will need that convention codified.
- Does `CombatVFXIdolOverlayArchitecture.md` now document the "compiled and reachable seam, not real-asset spawn proof" status for future-author readers, or is that only in this packet?

## Required Verification

- Re-run `git diff -- Source/T66/Gameplay/T66PlayerController_Overlays.cpp` and `git diff -- Gameplay/Combat/MASTER_COMBAT.md` and post the hunks (or a "no task-owned hunks" confirmation) so the diff-scoping clarification is auditable.
- Add `Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water` to the Earth neutral forbidden-log assertion list inside `RunHero1AxeAOEWaterIdolImpactProof.ps1` and re-run the proof; current Earth `PASS` does not prove the new context branch is skipped, only that the placeholder and resolution log are.
- Confirm staged build's `T66.log` was checked for warnings from the new idol context branch (not just fatal/error patterns) — a one-line evidence quote from the staged log would close this.
- Confirm the binding helper currently has no `Idol_Water` Niagara binding registered (so the helper fall-through to placeholder is the verified path), e.g., by quoting a "no binding found" log entry from the proof run.

## Rationale

The structural work is real, verified, and honestly bounded: weapon vs idol context separation is in place, Water consumes it end-to-end, Earth proves no cross-idol leakage, build/validator/staged smoke all pass, and the placeholder vs final-Niagara split is explicit. What pushes this to REVISE rather than APPROVE is the audit hygiene around two files listed as changed but disclaimed from scope, and the `AoeDelay=false` behavior that the new idol-authority branch arguably owns and currently sidesteps. Both are fixable with documentation/diff-scoping rather than re-implementation, so a clean revision pass should land at APPROVE.

