Verdict: REVISE

## Blockers

None. The pass is diagnostic-only, additive, CVar-gated, and out-of-scope correctly excludes routing/balance fixes and B.10 reattempt.

## Major Issues

- **Runner gate semantics are underspecified.** The packet says "For this diagnostic set, `RichSpawns` should not abort the entire investigation… If the current runner cannot do that cleanly, add a reviewed diagnostic mode or preserve rows before halting." This is the most important runner-side change in the pass and it is left conditional. Decide and write down exactly: does the Resume5 runner mode (a) bypass the `RouteValidity` reject for `RichSpawns>0` while still recording the rejection cause, (b) downgrade reject to warn-and-continue, or (c) record the row before continuing the capture set? The current text could be read three ways and that is implementation ambiguity, not Codex latitude.
- **Sample size change unjustified.** Resume4 used 10/10. Resume5 proposes 5 CVar-on captures without stating why 5 is sufficient for attribution. Either justify (e.g., diagnostic-only, not acceptance) or align with the established 10. As written, the runner change is silent on whether this is a new mode or a different size for the existing mode.
- **No behavioral-neutrality check for the new counters.** Risk section asserts "no RNG draws, no data loads beyond existing values," but Verification has no step that proves it. Add a step that compares a CVar-off pass with the new counters compiled in against the Resume4 CVar-off baseline (FPS median, projectile fired/hit totals, manager overhead) to demonstrate the instrumentation did not perturb the spawn or wave loop.
- **`[RouteAttributionSummary]` schema undefined.** Runner parsing depends on a stable key/value format, family bucket names, and reason bucket names. The packet enumerates buckets but does not commit to the line format the runner will regex against. Codex should fix the schema in the packet so review can validate parser-vs-emitter alignment, not after the fact.
- **Reset lifecycle named only by intent.** "Reset route-attribution counters when autocapture resets ranged diagnostics at `enemywaveperf` start" — name the exact reset site (e.g., the function that resets `FT66RangedPressureDiagnostics`) and the terminal emission site (the same call sites as `EmitRangedPressureSummary`). Otherwise reset drift between the two summaries is a real possibility and will silently corrupt attribution.

## Minor Issues

- The `RoutedRich_ElitePromotion` vs `RoutedRich_MiniBossPromotion` bucket choice is deferred to "if live code shows only mini-boss promotion." Resolve this from the code inspection already done (live facts say mini-boss only) before implementation rather than leaving Codex to decide mid-edit.
- "Special/Unknown" family bucket needs a definition. What classifies as Unknown? `MobID==None`? Family lookup failure? If both, then `RoutedRich_FamilyLookupFailed` and the Unknown family bucket overlap and double-count.
- Non-director spawn path audit lists ~9 sites but says "Audit, but do not necessarily instrument every path unless it can run during `enemywaveperf`." Commit to which paths will be instrumented and which are deferred. Otherwise the `RoutedRich_NonDirectorPath` bucket is structurally incomplete and the leak attribution is unfalsifiable.
- Documentation list updates `B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md` but does not state where in that file the Resume5 section appends (top, after Resume4, end). Minor, but the consolidated packet has a structure worth preserving.
- Gating decision (`T66.Ranged.DiagnosticLogging=1` vs new CVar) is openly punted to the reviewer. My read: reuse `T66.Ranged.DiagnosticLogging=1` because the attribution summary is logically the same diagnostic surface as `[RangedDecisionSummary]`, no new command-line surface needed for a one-pass diagnostic.

## Clarifying Questions

1. Will the Resume5 runner mode be a new switch on the existing `run_b101d_projectile_manager_validation.ps1` (e.g., `-Resume5Diagnostic`) or a sibling script? Affects rollback and AGENTS.md compliance.
2. Is the Dungeon `enemywaveperf` capture expected to spawn any mini-boss promotion? If yes, the measurement contract under which "all Dungeon Ranged should route lightweight" is already wrong and the contract itself needs an explicit clarification before attribution data is interpreted.
3. Should the diagnostic pass also run one CVar-off capture as a control to validate that attribution counters report 0 leaked-rich for migrated families under known-clean conditions? This would catch counter bugs.

## Required Verification

The Verification Plan in the packet is broadly correct, but add:

- Behavioral-neutrality control: one CVar-off capture with counters compiled in, compared against Resume4 baseline (FPS median, projectile fired/hit, overhead, hero deaths). Pass criterion: deltas within Resume4's natural per-capture variance.
- Counter sanity: assert that `sum(RoutedLightweight_* + RoutedRich_*) == total observed spawns from director path` for each capture, surfaced in the runner output as a parity check. Without this, missing branch instrumentation will not be detectable.
- Hash provenance per capture is in the plan — keep it, and reject any row with hash drift instead of warn-only.
- Explicit confirmation step that no boss/special is present in the standard Dungeon `enemywaveperf` captures (the packet states this is expected, verification should record the observed special/mini-boss counters per capture, not assume).
- AGENTS.md compliance check: confirm no broad Content-tree `git status` is invoked by the runner or surrounding scripts during the pass.

## Rationale

The diagnostic direction is correct and the live-code inspection already surfaces the two most likely confounds (mini-boss promotion and `AcquireMob`-failure fallback), which is exactly what a diagnostic-only pass should attribute. Scope, rollback, and out-of-scope are clean. The pass is not approvable as-is because three implementation-relevant decisions (runner gate behavior under `RichSpawns>0`, sample-size justification, and `[RouteAttributionSummary]` line schema) are deferred into the implementation phase, and the verification plan lacks a behavioral-neutrality control for the new counters. These are addressable without rescoping; tighten the packet on those points and resubmit for APPROVE.

