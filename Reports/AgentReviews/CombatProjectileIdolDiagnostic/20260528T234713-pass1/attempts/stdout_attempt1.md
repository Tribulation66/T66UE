Verdict: REVISE

## Blockers
None.

## Major Issues
- **Hardcoded `=1` expected counts are unverified runtime assumptions.** The proof harness uses literal `SimpleMatch` substrings (`RunHero1AxeAOEWaterIdolImpactProof.ps1:151`), and the packet proposes pinning `WeaponImpactContexts=1`, `EligibleWeaponImpactContexts=1`, `ExpectedWaterIdolImpactContexts=1`. But `PublishWeaponImpactContext` is invoked from multiple paths in `T66CombatComponent.cpp` — slash (`:1795`) and DoT (`:1947`) both append to the same `WeaponImpactContexts` array regardless of `bPrimary`. If the Hero1 Axe AOE scenario emits a DoT (or other) context alongside the primary, `WeaponImpactContexts.Num()` is `>1` and the proof FAILs on a correct build. Codex must derive these counts from an observed proof run before hardcoding them, or assert parity structurally (`ExpectedWaterIdolImpactContexts == WaterIdolImpactContexts`, `WaterIdolContextParity=PASS`) rather than fixed literals.

## Minor Issues
- The EarthNeutral case has `ForbiddenPatterns` (`:59-64`) but the packet does not add `CombatIdolImpactDiagnostic SourceID=Idol_Water` to that forbidden list. The "log only when Water-relevant" gating implies it should never appear in the Earth case; adding it as a forbidden pattern would make the gating mitigation an enforced assertion rather than an unproven claim.
- The diagnostic summary string is shown as `CombatIdolImpactDiagnostic SourceID=Idol_Water ... WaterIdolContextParity=PASS ...`. Confirm the literal emitted token order/spacing matches each `SimpleMatch` substring exactly, since substring checks are brittle to formatting drift.

## Clarifying Questions
None that block Codex-owned revision.

## Required Verification
- Verification plan is sound: focused `T66Editor` compile, proof wrapper run, MP4 + log-excerpt + Earth-neutral artifacts, `ffprobe`, standalone Development stage. Keep all of it.
- Before finalizing the proof patterns, run the proof once and read the actual emitted `CombatIdolImpactDiagnostic` line to confirm real counts; only then commit literal expected values.
- Confirm the capture mode already enables `CVarT66CombatImpactSourceVerbose` (existing gated `CombatImpactContext` patterns at `:31` imply it does, but verify the new gated logs surface under the same path).

## Rationale
Scope is genuinely diagnostic-only, instructions were read, rollback is clean, and the repo claims (CVar, `PublishWeaponImpactContext`, primary-context idol branch, proof structure) all check out against source. The one real defect is the proof asserting fixed `=1` counts that may not match runtime given multiple context-publish sites — a Codex-resolvable issue via an observed run or parity-based assertions, hence REVISE rather than APPROVE or BLOCK.

