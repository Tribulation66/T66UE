Verdict: REVISE

## Blockers
None.

## Major Issues
- **Gate change persistence is ambiguous.** The packet says "for this diagnostic pass" but the action is to "modify `run_b101d_projectile_manager_validation.ps1`" — a permanent script edit will outlive the pass and silently relax future runs. Either gate the advisory behind an opt-in switch (e.g., `-AllowHighFpsAdvisory` parameter) or specify an explicit revert step in scope.
- **No root-cause hypothesis for the high-FPS drift.** Two consecutive controls overshooting the historical max (168.93 after 172.04) is a trend, not noise. The amendment treats this as benign because structural counters match, but a higher FPS with identical projectile/spawn counts can also mean reduced background work (thermals, scheduling, scene cost) — exactly the kind of measurement perturbation the control was designed to catch. The packet should explicitly justify why structural-counter parity is sufficient to rule out perturbation, or note that root cause is being deferred.
- **Advisory event payload is unspecified.** `RouteControlHighFpsAdvisory` is named but the packet doesn't say what fields it must carry (AvgFPS, overshoot delta vs. 167.76, envelope reference, control-row index). Without that, "documented" in the plan doc is unenforceable.

## Minor Issues
- The 0.70% framing minimizes the deviation; absolute delta (1.17 FPS over a 167.76 max) and envelope width should both be cited so future readers can judge severity without re-deriving.
- "Five CVar-on captures, extended to ten only if no leak appears in the first five" inverts intuition — typically you extend on inconclusive, not on clean. Confirm this is the intended trigger and not a wording slip.
- Hygiene-unclean discard of the first row (172.04) is reasonable but should be recorded in the plan doc, not just the packet, so the audit trail survives.

## Clarifying Questions
- Is the runner edit intended to be reverted after this pass, or to become the new permanent gate behavior?
- Has the underlying reason for the FPS climb (hardware state, background load, build differences vs. when the 146.89–167.76 envelope was set) been investigated, or is this being treated as envelope staleness?
- Does the advisory event need to be surfaced in the combined packet summary section, or only in the per-row log?

## Required Verification
The packet's listed verification gates are appropriate but incomplete. Add:
- Confirm the runner edit either (a) is reverted at end of pass, or (b) is feature-flagged off by default.
- Confirm `RouteControlHighFpsAdvisory` fires exactly once on the CVar-off control row and is absent from CVar-on rows.
- Confirm the plan doc captures the discarded 172.04 row, the kept 168.93 control row, and the advisory rationale before any acceptance discussion resumes.
- PowerShell parser check should be on the amended script, with a dry-run that exercises both the high-FPS path (advisory) and a synthetic low-FPS path (still blocking) to prove only the high side was relaxed.

## Rationale
The diagnostic objective — explaining CVar-on route leakage via per-spawn aggregate counters, not FPS medians — does support treating a clean-structural high-FPS control as non-fatal. The reasoning in the packet is directionally correct and the structural/hygiene gates are preserved. However, this is a control-gate softening based on observed data, which is the exact pattern AGENTS.md-style discipline is designed to slow down. Approval is appropriate only once the change is scoped so it cannot quietly affect future runs, the advisory event is specified, and the high-FPS drift is at least acknowledged rather than waved off as 0.70% noise.

