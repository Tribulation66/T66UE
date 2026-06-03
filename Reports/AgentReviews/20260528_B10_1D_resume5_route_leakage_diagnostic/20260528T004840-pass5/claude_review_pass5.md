Verdict: APPROVE

## Blockers

None.

## Major Issues

None. The amendment is narrow, opt-in, and preserves every structural/hygiene gate. The default-off switch and the explicit "diagnostic-only escape hatch" comment correctly box in the blast radius.

## Minor Issues

- **Threshold equality.** Section 4 says "Treat only `AvgFPS > 167.76` as advisory." Confirm the existing strict gate uses the same comparator (strict `>` vs `>=`) so the boundary case `AvgFPS == 167.76` is handled identically with and without the switch — otherwise the switch silently changes a boundary that was previously passing/failing for unrelated reasons.
- **Source of `167.76` and `146.89`.** Packet doesn't say whether these come from a config/envelope file or are inlined in the runner. If they live in two places, this is the moment they will drift. A single source of truth for the envelope bounds (even a `$script:` constant) is worth a sentence before Codex starts.
- **Advisory payload — units/source.** Spell out the units and provenance for `PerfSystemOverheadMaxUs` (matches existing summary field?) and `OvershootPercentOfMax` (e.g., `(AvgFPS - EnvelopeMaxFPS) / EnvelopeMaxFPS * 100`, rounded to how many places). Pinning these now avoids a follow-up packet to fix advisory ambiguity.
- **"Exactly once" semantics.** Gate 4's "fires exactly once on the CVar-off control row" is fine for the single control row in this mode, but worth a one-line guard in the runner so a future second control row (or rerun-in-place) cannot multi-emit.

## Clarifying Questions

1. The discarded `172.04 FPS` row had stale `git status --porcelain` workers. Are those workers' identity and clear-time captured in provenance, or is "discarded as hygiene-unclean" the only artifact? If the diagnostic later wants to revisit whether worker contention biases FPS, the worker IDs/timings matter.
2. The "run five, extend to ten only if no leak in first five; if leak appears, still complete five" extension rule — is this consistent with the prior B.10.1D Resume packets, or is it introduced by this amendment? If introduced here, that should be called out as a procedural delta, not just a scope item.
3. Is the `-AllowHighFpsControlAdvisory` switch intended to be removed after this single Resume5 pass, gated behind a follow-up envelope refresh, or left dormant pending re-review? The runner comment says "removed or re-reviewed" — pick one as the default expectation so it doesn't quietly persist.

## Required Verification

- PowerShell parser check on the amended runner.
- Synthetic assertion suite covering all three matrix points: switch-absent + high-FPS → block; switch-present + high-FPS + clean structural → advisory + continue; switch-present + low-FPS → block. Include one case proving advisory does not fire on CVar-on rows.
- Confirm `RouteControlHighFpsAdvisory` is written only to capture-progress provenance, not into any acceptance-grade aggregate or comparison table.
- Capture and archive: discarded `172.04` row, clean `168.93` advisory row, the five (or ten) CVar-on diagnostic rows, binary SHA stability across all rows, zero non-zero exits / HeroDeath / overhead rejects / dropped fires / missing summaries / route-counter mismatches.
- Confirm in the post-run write-up that no acceptance claim, FPS median, or leak-rate estimate is derived from this pass — only existence / family-scope / root-cause statements about route attribution.

## Rationale

The amendment correctly isolates the only failing dimension (high-side FPS on a counter-only diagnostic) behind a default-off switch while keeping every gate that could mask a real defect (low-FPS, fired/hit ranges, overhead, route validity, counter mismatch, hash drift) strict. The diagnostic objective is categorical route-leak identification, not FPS acceptance, so a `0.70%` overshoot on the high bound is not load-bearing evidence. Rationale explicitly disclaims acceptance-grade interpretation of the resulting rows, and scope explicitly excludes refreshing the envelope or any production change — both are the right boundaries. Implementation is one runner-local change with documented verification and an explicit removal/re-review note, so it is safe to present at the AGENTS.md go-ahead gate.

