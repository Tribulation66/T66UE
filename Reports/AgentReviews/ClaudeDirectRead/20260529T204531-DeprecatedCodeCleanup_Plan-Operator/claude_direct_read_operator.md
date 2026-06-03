The plan packet is fully drafted and ready to write to the required stop-condition path; the write is pending your permission approval.

Summary of what the packet contains (all anchored to current source/config):

- **Live anchors confirmed** for all six targets — rich-basic routing collapse points in `T66EnemyDirector.cpp` (the shared rich miniboss/special/boss spawn branch is flagged to preserve), the 5 inert CVars, the two deprecated projectile classes (no spawn sites; only the `T66GameMode_Backrooms.cpp:151-152` filter + includes reference them), the GamblerToken enum/alias/field sites, and the `DefaultEngine.ini:16-21` redirects.
- **CoreRedirect verification plan** that does not assume safety: text + binary `.uasset` scan for the old names, per-redirect keep/remove rule, leave-in-place on any hit.
- **Phase plan (0–5)** routing the proof-bearing work to Codex-approved FullOperator runs, plus exact files to edit/delete and the grep/build/stage-SHA/`enemywaveperf`-capture verification suite.
- **Three escalations flagged**: (1) **Blocker/Pablo decision** — `ActiveGamblersTokenLevel` / `GamblersTokenUnlockedLevel` are *live save-game persistence under a legacy name*, not dead code, so "delete, no migration" would drop vendor-token persistence; (2) mid-enum `GamblerToken` removal is name-serialized (no reindex) but legacy saves fall back to `None` without an EnumRedirect; (3) projectile classes are `Blueprintable` so deletion is gated on the binary asset scan.
- **Two capability limits stated plainly**: this read-only profile cannot reach `C:\UE\T66_B13_Worktree` (outside sandbox — existence/evidence must be verified in a FullOperator phase), and cannot build/stage/capture/scan binary assets, so it plans but cannot satisfy the proof requirements.

Please approve the file write to `C:\UE\T66\Reports\AgentReviews\20260529_DeprecatedCodeCleanup\operator_plan_packet.md` to complete the stop condition.

**Codex Token Spent:**
Unavailable (read-only Operator profile; helper not run this phase)

**Claude Tokens Spent:**
Unavailable (host did not expose a per-run count to this profile)
