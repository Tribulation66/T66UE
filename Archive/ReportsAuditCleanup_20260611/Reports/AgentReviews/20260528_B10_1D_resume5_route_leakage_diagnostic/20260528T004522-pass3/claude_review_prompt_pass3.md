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

Modify `Saved/Codex/Performance/LightweightActorB10_1D/run_b101d_projectile_manager_validation.ps1` control-gate behavior:

1. Keep the CVar-off control row mandatory before CVar-on route diagnostic captures.
2. Keep all structural/hygiene checks blocking:
   - capture rejected
   - `AvgFPS < 146.89`
   - projectiles fired outside `108-595`
   - hero hits outside `107-591`
   - `PerfSystemOverheadMaxUs > 10000`
   - route summary missing or `RouteCounterMismatch != 0`
   - route invalid on CVar-off
   - binary hash drift
3. Treat only `AvgFPS > 167.76` as advisory for this diagnostic pass when every structural/hygiene gate is clean. Emit a progress event such as `RouteControlHighFpsAdvisory`, then continue.

## Rationale

The control was intended to catch measurement perturbation before route attribution. A slower-than-envelope CVar-off row would still be a real performance concern. A faster-than-envelope row with clean structural counters does not invalidate the diagnostic objective because Resume5 is not accepting FPS medians; it is trying to explain CVar-on route leakage using per-spawn aggregate counters. Blocking on a 0.70% high-side overshoot prevents collecting the actual evidence while preserving no additional safety.

## Scope

In scope:

- One runner gate amendment.
- Rerun `-Mode RouteDiagnostic -HeroHPOverride 20000` from the start.
- Document the control advisory and final route-attribution captures in the plan doc and combined packet.

Out of scope:

- Any routing fix.
- Any acceptance reattempt.
- Any production behavior change.
- Any broad Git/LFS workflow change.

## Verification Gates

- PowerShell parser check for the runner.
- CVar-off control row recorded and structurally clean.
- Five CVar-on route diagnostic captures, extended to ten only if no leak appears in the first five.
- Binary hash stable across the pass.
- No non-zero exits, HeroDeath, overhead rejects, dropped fires, missing summaries, or route-counter mismatches.

</review_packet>
