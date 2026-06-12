Verdict: REVISE

## Blockers

None that prevent the work conceptually, but see Major Issues below for items that should be tightened before Codex implements.

## Major Issues

- **AGENTS.md "updated Claude approval rule" claim is unverifiable from the packet.** The packet asserts the root `AGENTS.md` was updated so a `Verdict: APPROVE` alone authorizes implementation without manual user go-ahead. The user's standing operating discipline historically requires explicit user confirmation before implementation. Quote the exact AGENTS.md clause (and its commit/date) in the packet so the user can confirm the rule change is real, current, and applies to multi-workstream packets — otherwise the orchestration step 2 ("proceeds without a separate manual go-ahead") is unsafe.
- **Manager subsystem tick is implicit but never explicitly affirmed.** The "at most one VAT advancement per active mob per manager tick" and "MID parameter application from the manager loop" mechanisms require `UT66MobManagerSubsystem` to tick. The runtime-tick proof acceptance criteria only enumerate `AT66MobBase` actor tick and lightweight component tick. Add an explicit statement that the manager subsystem tick is intentionally enabled, document its tick group/interval, and exclude it from the "no tick" proof so the proof doesn't accidentally fail itself or hide a regression that disables advancement.
- **Original Stage 0a failure is being reclassified, not fixed.** The intermittent rich Ranged delivery problem is now closed as "known limitation of the deprecated path." That is consistent with the new direction, but B.10 closure language in `pending_issues_Gameplay.md` should explicitly state: (a) the rich basic-mob path is no longer the supported route for basic mobs, (b) the original symptom remains reproducible on the deprecated path, and (c) cleanup-pass removal cancels the limitation. Otherwise a future reader sees "B.10 closed" and assumes the underlying bug was fixed.
- **"Cleanup pass" is referenced as the deletion trigger without scheduling or scope.** Every deprecated marker points at an undefined future pass. Define at least the trigger (e.g., next basic-mob touch, end of B.12 series, dated milestone) and the owning agents.md/pending-issues entry that tracks it, or this becomes silent debt with stale `2026-05-28` markers.

## Minor Issues

- The Phase 1 rerun uses three captures with a relatively tight `stdev <= 20.0 FPS` guard against a known sample at 179/190/203 (rough stdev ≈ 12). The escalation-to-ten rule covers borderline failures, but consider documenting that the three-row median (not just any row) must be ≥170.0 to avoid ambiguity when one row spikes high.
- The kill-mid-flight proof spec is deterministic, but doesn't define how the boss source is "killed or unregistered before the next projectile-manager tick" without invoking the normal damage path (which can trigger other state). Specify whether the proof uses a direct destroy/unregister API or the damage-to-zero path; mixing them could change what `DroppedInvalidSource` actually proves.
- VAT proof says "force a safe non-shipping sample or document why that family lacks the clip" for unreachable clips. Define which clips per family are expected to require forced sampling so the completion packet can't trivially satisfy the rule by declaring everything documented-as-absent.
- Source manifest format is referenced ("write a source manifest for all routing, VAT, projectile manager, autocapture HP, and capture-profile files") but not specified. Either point at an existing manifest convention or define columns (path, SHA256, mtime) so the Phase 1 gate is reproducible.
- Per-row sanity floor (`ProjectileManagerFired >= 10`, `ProjectilesHitHero >= 10`) is a useful regression guard but is also a behavioral check on the lightweight Ranged path; consider adding a brief rationale for the `10` constant so a future tuner doesn't lower it without thinking.
- Gambler cleanup removes the widget's direct `SpawnActor<AT66GamblerBoss>` fallback. Specify what "reports failure" means at the widget layer (toast? log? UI state?) — otherwise a player-controller failure becomes a silent no-spawn from the casino UI.
- MINOR-CLEANUP "demote trap projectile fire/impact logs to VeryVerbose" is fine, but the packet does not list which exact log categories/channels are in scope. A workstream agent could over- or under-reach. Name the categories.
- Deprecation marker strings are deterministic, which is good, but the date `2026-05-28` will rot if implementation slips into a later day. Either anchor on a non-dated tag (e.g., `// DEPRECATED-B11B12:`) or commit to updating the date on implementation day.

## Clarifying Questions

- Has the root `AGENTS.md` actually been amended to let a Claude `APPROVE` packet skip the manual go-ahead gate for multi-workstream changes? If yes, what is the exact clause and when did it land? If no, the orchestration must pause for user confirmation after the review.
- For the `UT66MobManagerSubsystem` VAT loop: are you targeting `TG_PrePhysics` or a different tick group, and is this the same tick used for existing manager work, or a new tick entry point?
- What is the user-visible behavior when Gambler boss spawn fails after removing the widget fallback — silent, toast, button stays available, button disables?
- For the B.13 custom-data-ready layout: which fields/order has B.13 already committed to, and where is that defined? The packet says "laid out for B.13 per-instance custom data" but no reference is given.
- Is `T66.Ranged.DiagnosticLogging=1` aggregate-only also enforced in Phase 3, or only Phase 1? The Phase 3 section reuses "the same profile as Phase 1" but doesn't restate the diagnostic CVar.

## Required Verification

- Phase 1 lightweight three-row captures on a re-staged content-complete Win64 Development standalone with recorded source manifest and staged exe SHA256, meeting the documented median/dispersion/projectile-floor rules.
- Focused compile of the four workstream file sets plus serialized cleanup files, followed by a full staged build of the integrated binary with recorded SHA256/mtime/length.
- Phase 3 three-row captures on the integrated binary, with neutrality (≥95% of Phase 1 median), dispersion guard, and projectile sanity floor; escalate to ten rows on borderline failures per the documented rule.
- Runtime tick proof using `IsActorTickEnabled()` and component tick state at runtime (not header defaults) for `AT66MobBase` and lightweight components, with manager subsystem tick explicitly excluded as intentionally enabled.
- Multi-frame VAT proof: ≥3 samples ≥0.10s apart per lightweight family for idle/move/attack/death where reachable, with ≥1.0 frame delta on active clips, plus pool-reuse reset proof (`DuplicateVatAdvanceCount=0`, same-tick post-acquire frame within 0.5 of clip start unless explicit override).
- `MinibossTraversalProof` autocapture row with `BlockedWhileAlive=1` / `UnblockedAfterDeath=1` per floor 2→3→4.
- `BossProjectileKillMidFlightProof` autocapture row with `DroppedInvalidSource >= 1`, no `ProjectilesHitHero` increase post-source-death, no hero HP decrease post-source-death.
- Capture hygiene: no `git-lfs.exe` overlapping any accepted Unreal run window; bare `git.exe` overlap recorded only if PerformanceSystem overhead row was accepted.
- Pending issues updates: B.10 closure cites the Phase 1 staged SHA inline; rich basic-mob deprecation and intermittent-delivery limitation explicitly documented; floors 3/4 and kill-mid-flight gaps closed or bug recorded.

## Rationale

The packet is well-structured, respects the disjoint-file rule, preserves user constraints (no destructive worktree action, no tick re-enable, no B.13 HISM, Mini scope excluded, aggregate counters only, Gambler serialized to main), and brings the right anti-lookalike discriminators for VAT (multi-frame + runtime tick proof + pool reuse). Numeric floors and escalation paths are concrete. However, three things keep it short of `APPROVE`: (1) the load-bearing claim that AGENTS.md now lets a Claude approval skip the user confirmation gate is asserted but not quoted, and that policy change directly affects the orchestration's safety; (2) the manager-subsystem tick — which the entire VAT-STATE workstream depends on — is never explicitly carved out of the "no tick while mobs are live" proof, risking a self-failing proof or a hidden regression; (3) B.10 closure language and the open-ended "cleanup pass" need tightening so a future reader doesn't misread a deferred deprecation as a fix. None of these require redesign; they require the packet to state the missing facts and tighten a few rules before Codex implements.

