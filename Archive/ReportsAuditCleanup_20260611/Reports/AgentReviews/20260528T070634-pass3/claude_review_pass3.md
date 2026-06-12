Verdict: REVISE

Spot-checked the foundational live-repo claims. Cited line numbers all confirm: `T66MobBase.cpp:79-80` and `:897` disable actor tick in ctor and pooled reset, `T66MobManagerSubsystem.cpp:678` disables on activation, the manager tick at `:2149+` advances VAT via `TickMobVertexAnimationState`, VAT fields live at `T66MobBase.h:268-276`, and there is no `AT66MobBase::Tick` override. Codex's live-state correction is accurate.

## Blockers

None. The plan is verification-first, defers all source edits to audit-driven triggers and a Pablo go-ahead gate, and refuses to manufacture a fake A/B by reintroducing actor tick. Safe to present at the go-ahead gate after the revisions below.

## Major Issues

- **95% gate provenance is unstated.** The acceptance line "CVar-on median must be at least 95% of current CVar-off median" appears without a citation back to the established B.10 acceptance criterion or the B.10.1D runner. If 95% is the canonical B.10 gate, cite the source (e.g., `pending_issues_Gameplay.md` or the B.10 plan section that defines it). If it is reviewer-introduced, name it as new and align it with the documented gate. Stage 0 closes B.10, so the gate it closes against must be unambiguous.
- **B.11 intent ambiguity not resolved in the plan.** Reviewer Question 2 asks whether B.11 requires physically moving VAT fields off `AT66MobBase`. That answer determines whether Stage 1 has any source-change scope. Leaving it as an open question while declaring "default: no source change" risks Pablo greenlighting the pass and later concluding B.11 is not actually satisfied. Resolve this in the plan (with a referenced B.11 intent statement from `2026-05-23_T66_LightweightActor_Plan.md` or `T66_Mass_Migration_Plan.md`) before Stage 0, not after.
- **Worktree contamination risk for Stage 0 binary baseline is not pre-flighted.** Conversation gitStatus shows many uncommitted modifications (`DT_Weapons.uasset`, `Weapons.csv`, several `MASTER_*.md`, `T66RunStateSubsystem_Combat.cpp`, deleted Cliffs/QuickRevive assets, etc.). The plan's binary-provenance rule will hash a staged build that includes these, which conflates the B.10-closure baseline with unrelated WIP. Add a Task 0 step that enumerates and classifies modified/deleted paths as runtime-affecting vs. inert, and either commits/stashes/reverts the runtime-affecting subset or documents per-path why it is inert before staging the Stage 0 binary.

## Minor Issues

- **Frame-cadence under-specified.** "3+ frames" of VAT Frame scalar proof can pass with three samples inside the same VAT frame at high FPS. Add a minimum inter-sample interval (e.g., ≥ 1/SampleRate of the slowest sampled clip) or require observed `Frame` parameter change between consecutive samples.
- **`DumpTicks` invocability not verified.** The plan correctly proposes `DumpTicks` as the preferred non-source runtime proof and falls back to asking Pablo for a hook. Confirm in Task 0 whether existing automation/command paths can fire `DumpTicks` after mobs spawn in a staged standalone session, so the fallback escalation is only invoked when actually needed.
- **Runner reuse vs. recreate left ambiguous.** "Reuse the B.10.1D runner or create a new runner that preserves equivalent hash fields" is fine guidance but the plan should commit to one path before Stage 0 to keep provenance trivially auditable. Default to reusing the existing runner; only fork if a concrete field is missing.
- **Family-class table is informational only.** The Melee/Rush/Flying/Ranged → `AT66MobBase` mapping should be verified by source audit in Task 0 (not assumed); the plan does require this but could state explicitly that if any family resolves to a subclass, the component-tick table must be expanded *and* the smoke roster updated to spawn that subclass.
- **Doc-sweep scope omits one likely-stale doc.** Consider adding `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md` and `C:\UE\T66\Gameplay\Combat\MOB_SYSTEM_MODEL_PIPELINE_REPORT_2026-05-15.md` to the sweep — both touch mob VAT/tick ownership claims.
- **Stage 0 fail branch is good but does not specify rollback of staged binary.** If Stage 0 fails, state whether the staged binary and runner artifacts are retained as diagnostic evidence or pruned, and where they land. Default to retain.

## Clarifying Questions

1. Is the 95% CVar-on/CVar-off acceptance gate the canonical B.10 gate, and where is it documented? If not canonical, what is the canonical gate?
2. Does Pablo accept that B.11's migration-plan intent is structurally satisfied by manager-owned per-frame VAT advancement alone (given B.13 will retire the per-mob MID path), or does B.11 require VAT state fields to physically move off `AT66MobBase` before the pass can close?
3. Are the in-flight uncommitted worktree changes (`DT_Weapons.uasset`, `Weapons.csv`, master docs, `T66RunStateSubsystem_Combat.cpp`, deleted Cliffs/QuickRevive assets) expected to be present in the Stage 0 baseline binary, or should they be reverted/committed/stashed first?
4. Is the existing B.10.1D runner authoritative for Stage 0 reuse, or is a new runner expected for the B.11+B.12 packet directory?

## Required Verification

Before Stage 0 execution:
- Worktree classification: every modified/deleted path under `Source/`, `Content/`, and runtime configs accounted for as runtime-affecting vs. inert, with action taken.
- Component-tick audit table populated with source evidence for `PrimaryActorTick`, `VisualMesh`, `CapsuleComponent`, `BodyHitZone`, `HeadHitZone`, `LockIndicatorWidget`, and any movement/timeline helper; absences stated with evidence.
- Family→class map confirmed against the live spawn paths for Melee/Rush/Flying/Ranged.
- B.11 intent statement quoted from the migration plan with a yes/no on field-relocation requirement.

During Stage 0:
- Staged `T66.exe` SHA256 + length + mtime recorded at pass start, before/after each capture, and at pass end.
- CVar-off and CVar-on sets each produce ≥ 3 accepted rows with `PerformanceSystemOverheadMaxUs ≤ 10000`, zero `HeroDeath`, and route attribution distinguishing placed Slime guardian from random promotion.
- Escalation to 10 captures triggered if within 2× stdev of the 95% gate; halt on 2+ rejected captures.

Stage 1/Stage 2 verification (no-source default):
- `DumpTicks`-derived evidence that no `AT66MobBase` actor tick function is enabled and that the component-tick map matches Task 0's source claim.
- Multi-timestamp `Frame` scalar samples per family/clip showing `Frame` changes between samples while actor tick stays disabled.
- Pool reuse cycle (acquire → release → reacquire) with animation re-initialized and tick still disabled.

## Rationale

The pass correctly inverts itself once the live source contradicts the original premise: actor tick is already disabled and VAT is already advanced from the manager tick loop, so Stage 1/Stage 2 cannot legitimately produce a new isolated FPS gain and refusing to manufacture one is the right call. Stage 0 as the B.10 closure mechanism is sound and reuses the proven B.10.1D hygiene shape. The remaining gaps are not plan-design defects but tightening items: pin the 95% gate to its source, resolve the B.11 field-relocation question in the plan rather than as a runtime question, pre-flight worktree cleanliness so the staged binary that becomes the B.10 closure artifact is not contaminated by unrelated WIP, and sharpen the multi-frame VAT proof so it cannot pass on three near-identical samples. With those revisions, the plan is safe to present at the Pablo go-ahead gate.

