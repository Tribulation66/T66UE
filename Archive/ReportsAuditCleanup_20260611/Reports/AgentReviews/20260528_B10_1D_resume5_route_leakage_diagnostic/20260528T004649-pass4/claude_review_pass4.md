Verdict: REVISE

## Blockers

None that prevent the amendment from being safe in scope. The change is opt-in, narrow, and keeps all structural/hygiene gates blocking. However, the issues below should be resolved before the rerun is presented as greenlit.

## Major Issues

- **Unexplained FPS drift is left as a known-unknown and may confound the diagnostic itself.** The packet enumerates plausible causes (hardware/scheduler variance, thermal, background load, stale envelope) and then proceeds anyway. If "whatever is making CVar-off faster" also perturbs spawn cadence, the route-attribution counters collected under the advisory could be just as suspect as the FPS surface the amendment chose to ignore. The amendment should either (a) include at least one cheap differential check (e.g., confirm `ProjectileManagerFired` per second of wallclock is stable vs Resume4 control medians, not just inside the absolute 108–595 range), or (b) explicitly state in the rationale that the diagnostic's interpretive weight is reduced if the leak signal happens to correlate with the same hardware regime that produced the high FPS.
- **`ProjectileManagerFired=135` is at the low end of the 108–595 envelope while FPS is at the high end.** That correlation deserves an explicit one-line note in the packet — either "expected because spawns are time-gated, not frame-gated" or "flagged as a secondary measurement concern." Right now it's unaddressed.
- **The `-AllowHighFpsControlAdvisory` switch becomes a permanent runner capability with no per-pass scoping.** The packet says "This pass invokes the switch only for the Resume5 route diagnostic rerun," but nothing in the runner enforces that. Add either: a comment header in the runner naming the pass it was added for, a sunset condition (e.g., "remove once Resume4 envelope is refreshed"), or a log-line reminder so future operators see it's diagnostic-only.

## Minor Issues

- The advisory fires only on the CVar-off control row, not on CVar-on rows. For a diagnostic pass, capturing the same advisory payload on any CVar-on row that also exceeds 167.76 FPS would be cheap and would help interpret the route counters. Worth either adding or explicitly justifying the asymmetry.
- The `RouteControlHighFpsAdvisory` payload is well-specified but omits anything identifying which `-AllowHighFpsControlAdvisory` invocation produced it (e.g., a run-tag or argv echo). Useful for future audits.
- The extension rule ("five captures, extend to ten only if no leak appears in five") is sound, but the stop condition on a positive find should be stated: stop at the capture that produces the leak, or run the full five regardless?

## Clarifying Questions

- Has the Resume4 FPS envelope been re-baselined recently, or is `167.76` still from the original Resume4 control set? If the envelope is stale, the cleaner fix is an envelope refresh rather than an advisory switch.
- Was anything intentionally changed between the discarded `172.04` row and the `168.93` row besides clearing the stale `git status --porcelain` workers? If so, that belongs in the packet so the FPS difference is attributable.
- Does AGENTS.md require explicit user go-ahead before a runner gate is amended for a diagnostic? If yes, this packet is the proposal, not the authorization.

## Required Verification

The packet's verification list is mostly adequate but needs:

- Explicit negative-path assertion: with `-AllowHighFpsControlAdvisory` present, an `AvgFPS < 146.89` row still hard-halts (the packet implies this but doesn't list it as a discrete synthetic case).
- Explicit assertion that with the switch **absent**, a `>167.76` FPS row still hard-halts (regression guard against the switch's default flipping).
- Confirm the advisory does not increment any acceptance-grade counter or get written to any file consumed by Resume6 acceptance gates.
- Record the actual `-AllowHighFpsControlAdvisory` argv in the run log so the diagnostic provenance is auditable.

## Rationale

The amendment correctly identifies that a high-side FPS overshoot with otherwise clean structural counters is not a reason to halt a counter-based diagnostic, and it keeps every safety-relevant gate blocking. The scoping (opt-in, structurally bound, diagnostic-only, no acceptance reuse) is appropriate. The reason this is REVISE rather than APPROVE is that the packet acknowledges the FPS drift root cause is unknown without addressing whether that same unknown could distort the per-spawn aggregates the diagnostic is about to rely on, and the advisory switch is being added as a permanent runner feature with no per-pass enforcement or sunset. Both are fixable with small additions to the packet rather than a redesign.

