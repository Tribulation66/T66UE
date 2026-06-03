Verdict: APPROVE

Blockers
- None.

Major Issues
- None blocking. The plan is correctly scoped as logging-only diagnostic instrumentation plus proof-harness tightening, with no damage/targeting/VFX behavior change.

Minor Issues
- The expected-context formula `ImpactPresentationIdolSlots * EligibleWeaponImpactContexts` assumes a full cross-product. Since the current Water branch consumes only `PrimaryWeaponImpactContext` (one context), if `EligibleWeaponImpactContexts > 1` the "expected" count will exceed what the single-primary branch can emit, which would make `WaterIdolContextParity=FAIL` on a correct build. Confirm the parity computation accounts for current single-primary behavior (i.e., expected should reflect what the present branch actually intends to emit, not the future all-contexts design), otherwise the new `WaterIdolContextParity=PASS` proof assertion may fail legitimately.
- The Earth-neutral forbidden-pattern addition relies on the summary line being gated so it never fires when Water is not equipped. Verify the gating condition ("Water impact-presentation slot active or Water diagnostics touched") cannot be satisfied incidentally in the Earth-neutral case, or the forbidden pattern will produce false BLOCKs.
- `CVarT66CombatImpactSourceVerbose` must already be set/enabled in the proof wrapper's run path; if the wrapper does not force it on, the new required patterns will never appear and the proof will fail. Confirm the wrapper enables the cvar.

Clarifying Questions
- Does the current Water attack path emit exactly one weapon impact context in the proof scenario, or can it emit more? The parity logic and the regex `[1-9][0-9]*` choices depend on this.

Required Verification
- Compile `T66Editor` per the listed `Build.bat` command.
- Run `Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1` and confirm: MP4 for Water case, Water log excerpt with weapon context + idol context + placeholder spawn + idol damage + new diagnostic summary, `WaterIdolContextParity=PASS`, and all `*=0` skip-counter assertions.
- Confirm Earth-neutral case still passes with the new forbidden `CombatIdolImpactDiagnostic SourceID=Idol_Water` pattern.
- Run `ffprobe`/wrapper video checks.
- Stage standalone Development build only after compile + proof pass.

Rationale
The change is diagnostic-only, gated behind an existing verbose cvar, with explicit out-of-scope guards against architectural refactor and a clean rollback (remove counters/logs/patterns; no asset or data migration). It uses the repo's existing compile + proof + video verification process and correctly prefers regex parity assertions over brittle literal counts. PPF is reasonably marked N/A since no new visual carrier is authored. The only real risk is the parity formula producing a false FAIL under current single-primary behavior — resolvable by Codex during implementation/verification, so this does not warrant REVISE or human decision.

