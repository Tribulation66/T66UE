I've completed Phase 0 (read-only) and made **no edits**. Three findings diverge from the plan packet and require Codex rulings. Full evidence is written to `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/operator_phase0_findings.md`.

## Codex Approval Required

**Divergence A — Phase 1+2 CVar footprint spans the verification harness (decision needed).**
The 5 route/touch CVars are confirmed **inert** (director never reads them — `T66IsLightweightMobRoutingEnabled()` returns `true`, `(void)bUseLightweightRouting` at `T66EnemyDirector.cpp:717`, touch CVar `(void)`-ed at manager `:116`). But the plan mis-located them: 4 are defined in `T66EnemyDirector.cpp:51-97` (not the manager), and **all 5 are heavily referenced as test knobs in `T66PlayerController_Overlays.cpp`** — the same file that hosts the `enemywaveperf` capture harness (set-sites, command-line overrides `:2876-2912`, summary log `:2958-2963`). Clean removal touches 3 files incl. the verification instrument, not the 1 file planned. Need a ruling:
- **A1** — remove only the CVar definitions + manager helper/log-fields, leave the (now-no-op) harness string refs. Lowest risk.
- **A2** — full clean removal including the harness set-sites/overrides/summary + dead route-attribution remnants.

**Divergence B — Phase 4 GamblerToken footprint is ~13 files, not the 6 listed** (localization ×3, item-card, economy, backend parser, HUD, combat, etc.). All legacy branches alias to canonical `VendorToken`, so removal is behavior-neutral and mandatory-for-compile. I assess this as in-scope; flagging for confirmation, will proceed.

**Divergence C — Phase 6 B.13 is in unexpected state (stop).** `C:\UE\T66_B13_Worktree` exists at the exact path, but it is **not a registered git worktree and has no `.git`** — it's a plain orphaned full-tree copy. The plan assumed `git worktree remove`; that's N/A. Preserved evidence **is** confirmed in-repo (36 proof logs). Need approval for a raw `Remove-Item -Recurse -Force` of the orphaned non-git directory, since I can't git-verify it holds no unique work and a content diff would be a disallowed broad binary scan.

**Clean and ready to execute on approval:** Phase 3 (`AT66BossProjectile` — scan clean, no spawn/subclass/Blueprint refs), Phase 4 (GamblerToken), Phase 5 (all 5 CoreRedirects — old-names absent from Source, `DT_PlayerExperience.uasset`, and all Content).

I'm holding all edits pending your rulings on A (A1 vs A2), B (confirm proceed), and C (approve raw delete or specify alternate verification).
