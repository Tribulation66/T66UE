You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\gate_amendment_packet.md
- Output scope: review of the packet below only.

<review_packet>
# B.10.1D Resume5 Route Diagnostic Gate Amendment

## Working Goal

Continue the approved B.10.1D Resume5 route-leakage diagnostic after the CVar-off control gate halted twice only because FPS was above the historical Resume4 max.

## Current Evidence

- Build/stage succeeded on staged `T66.exe` SHA `D1E3235ED789C2596626BF6748F3DE49018B883D99F941B6160D860C535192FF`.
- First control row: `AvgFPS=172.04`, route-valid, no capture reject, but stale `git status --porcelain` workers were active at launch. I discarded it as hygiene-unclean.
- Second control row after clearing stale Git workers: `AvgFPS=168.93`, route-valid, no reject reasons, no Git/LFS overlap at launch or post-capture, overhead `724.8 us`, binary hash stable, one each of `RangedDecisionSummary`, `RouteAttributionSummary`, and `ProjectileManagerSummary`.
- Structural counters for the clean row stayed inside the Resume4 control envelope:
  - `ProjectileManagerFired=135` within `108-595`
  - `ProjectileManagerHitHero=135` within `107-591`
  - `RouteCounterMismatch=0`
  - `TotalObservedSpawns=90`, `DirectorObservedSpawns=90`, `NonDirectorObservedSpawns=0`
  - `UseLightweight=0`, `RichSpawns=24`, `LightweightSpawns=0`, `RouteValid=true`
- The only clean-control miss is the high side of the FPS envelope: `168.93` versus prior max `167.76`, a `1.17 FPS` / about `0.70%` overshoot.

## Proposed Amendment

Modify `Saved/Codex/Performance/LightweightActorB10_1D/run_b101d_projectile_manager_validation.ps1` control-gate behavior behind an opt-in switch:

1. Keep the CVar-off control row mandatory before CVar-on route diagnostic captures.
2. Add `-AllowHighFpsControlAdvisory`, default `false`. Without this switch, the runner preserves the current strict high-FPS halt behavior. This pass invokes the switch only for the Resume5 route diagnostic rerun.
3. Keep all structural/hygiene checks blocking:
   - capture rejected
   - `AvgFPS < 146.89`
   - projectiles fired outside `108-595`
   - hero hits outside `107-591`
   - `PerfSystemOverheadMaxUs > 10000`
   - route summary missing or `RouteCounterMismatch != 0`
   - route invalid on CVar-off
   - binary hash drift
4. Treat only `AvgFPS > 167.76` as advisory when `-AllowHighFpsControlAdvisory` is present and every structural/hygiene gate is clean. Emit one `RouteControlHighFpsAdvisory` progress event, then continue.
5. `RouteControlHighFpsAdvisory` payload fields must include:
   - `Set`
   - `Run`
   - `DiagnosticScope=B10.1DResume5RouteDiagnostic`
   - `AllowHighFpsControlAdvisory=1`
   - `AvgFPS`
   - `EnvelopeMinFPS`
   - `EnvelopeMaxFPS`
   - `OvershootFPS`
   - `OvershootPercentOfMax`
   - `TerminalWorldTime`
   - `RangedTotal`
   - `RichSpawns`
   - `ProjectileManagerFired`
   - `ProjectileManagerHitHero`
   - `FiredPerWorldSecond`
   - `HitHeroPerWorldSecond`
   - `PerfSystemOverheadMaxUs`
   - `ExeSHA256`
6. Add a runner comment next to `-AllowHighFpsControlAdvisory`: this is a B.10.1D Resume5 diagnostic-only escape hatch and should be removed or re-reviewed once the CVar-off envelope is refreshed.

## Rationale

The control was intended to catch measurement perturbation before route attribution. A slower-than-envelope CVar-off row would still be a real performance concern. A faster-than-envelope row with clean structural counters does not invalidate the diagnostic objective because Resume5 is not accepting FPS medians; it is trying to explain CVar-on route leakage using per-spawn aggregate counters. Blocking on a `1.17 FPS` high-side overshoot (`168.93` vs `167.76`, about `0.70%` of the high bound) prevents collecting the actual route evidence while preserving no additional safety for this counter-only diagnostic.

The high-FPS drift root cause is not proven. Plausible causes include normal hardware/scheduler variance, thermal state, lower background load, or stale historical envelope bounds. This amendment does not claim the FPS surface is acceptance-grade; it only says a structurally clean faster row is adequate to proceed to route-attribution diagnostics. The high row and the discarded unclean `172.04 FPS` row will be documented as measurement observations, not used as acceptance evidence.

The clean high-FPS row is also low-pressure inside the accepted structural envelope: `ProjectileManagerFired=135` and `ProjectileManagerHitHero=135`, near the low end of the Resume4 ranges, with `RangedTotal=24`. This is a secondary measurement concern for FPS interpretation, not for the categorical route-leak question. The CVar-on diagnostic will therefore be interpreted as existence/family-scope/root-cause evidence, not as a leak-rate estimate or FPS comparison. If a leak appears only in a similarly low-pressure row, it still proves the route path can leak; if no leak appears, the runner extends to ten because the negative result is inconclusive for an intermittent bug.

The cheap differential check for the rerun is:

- Record `RangedTotal`, `RichSpawns`/`LightweightSpawns`, `ProjectileManagerFired`, `ProjectileManagerHitHero`, and per-world-second fired/hit rates in the high-FPS advisory.
- For CVar-on rows, rely on `RouteAttributionSummary` categories and `CounterMismatch=0` to identify which spawn route produced leakage. Do not use those rows for acceptance-grade FPS claims.

The extension rule is intentional: run five CVar-on captures first; extend to ten only if the leak does not appear in those five, because a no-leak result is inconclusive for an intermittent bug.
If a leak appears in the first five, still complete the five-capture diagnostic set unless a hard reject occurs; do not stop at the first positive row.

## Scope

In scope:

- One runner gate amendment guarded by `-AllowHighFpsControlAdvisory`.
- Rerun `-Mode RouteDiagnostic -HeroHPOverride 20000 -AllowHighFpsControlAdvisory` from the start.
- Document the control advisory and final route-attribution captures in the plan doc and combined packet.

Out of scope:

- Any routing fix.
- Any acceptance reattempt.
- Any production behavior change.
- Any broad Git/LFS workflow change.
- Refreshing the CVar-off FPS envelope; if Pablo wants acceptance-grade FPS comparison after this pass, that is a separate reviewed packet.

## Verification Gates

- PowerShell parser check for the runner.
- Synthetic assertion check proving:
  - high-FPS still blocks when `-AllowHighFpsControlAdvisory` is absent
  - high-FPS emits the advisory when the switch is present and structural gates are clean
  - low-FPS still blocks even when the switch is present
- CVar-off control row recorded and structurally clean.
- `RouteControlHighFpsAdvisory` fires exactly once on the CVar-off control row if the high side is exceeded, and does not fire on CVar-on rows.
- Five CVar-on route diagnostic captures, extended to ten only if no leak appears in the first five.
- Binary hash stable across the pass.
- No non-zero exits, HeroDeath, overhead rejects, dropped fires, missing summaries, or route-counter mismatches.
- Plan documentation records the discarded `172.04 FPS` row, the structurally clean high-FPS control row, and the advisory rationale.
- Confirm the advisory is written only to capture-progress provenance and does not alter acceptance-grade counters or future acceptance gates unless the switch is explicitly supplied.

</review_packet>
