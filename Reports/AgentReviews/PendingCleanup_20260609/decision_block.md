# Pending Cleanup Program — Decision Block + Task Contract

Created 2026-06-09 (evening session). Owner: Claude Code session "pending issues full clean".
Status: RUNNING since 2026-06-09 night — user said GO (going to bed; overnight autonomous).
GO amendments (user, 2026-06-09 night): **Phase 1 is SKIPPED for this session — a different
agent owns the deferred-cleanup pass.** This session runs Phases 2 → 3 → 4 and updates the
pending-issue ledgers as fixes land. Multiple agents share the tree: on any unrelated
blocker (locked files, mid-flight edits by the cleanup agent, busy build system), WAIT and
retry — do not stop the run. Precondition met: full commit landed as version 1.2
(c8da91343, tagged `1.2`, pushed).

## Task contract

```text
Working task: Fully solve and clean all pending issues across C:\UE\T66 (cleanup program,
  not feature development), plus one approved add-on: Stats Rework Phase 3b display.
Scope: Phases 1-4 below. Phase 0 (landing the current 1,355-file working tree) is
  EXPLICITLY USER-OWNED — user does a full commit before the program starts; do not
  triage, restore, or re-litigate the ~620 uncommitted deletions.
Stop condition: All phases verified per their owning process (focused compile,
  StageStandaloneBuild.ps1, pre-release smoke, Unreal-owned captures where visual),
  ledgers/audits reconciled, origin/main pushed at each phase milestone.
```

## User decisions (2026-06-09, via in-session question gate)

1. **Working-tree deletions / Phase 0:** "I am going to do a full commit before you start —
   you can ignore those issues." → Phase 0 is user-owned. Program precondition = that
   commit exists (which also satisfies the documented precondition of both deferred
   cleanup specs: in-flight UI pass committed).
2. **Feature add-ons in scope:** Stats Phase 3b display ONLY (+% display language in
   stats panel / power-up / tooltips / item cards; spec §"Remaining" in
   `Reports/AgentReviews/StatsRework/STATS_REWORK_SPEC.md`). Leaderboard texture-lifecycle
   fix and footstep audio explicitly NOT in scope.
3. **Push policy:** push origin/main at phase milestones. No version tags unless the user
   names a version.

## Phase plan

- **Phase 1 — Deferred cleanup execution** (precondition: user's full commit):
  `Reports/AgentReviews/FriendslopMigration/coverage_audit.md` item 2 (plumbing cleanup)
  + `retro_removal_spec.md` R1→R5 in one pass (they overlap). Includes UE5RFX removal
  (101.6 MB), QuadRetro assets/scripts, retro materials/wrappers/pixelation cluster,
  BloodyRetro preset with leaderboard/topbar icon migration FIRST. R5 gate: full stage,
  cook log clean of deleted-path warnings, captures (main menu w/ leaderboard icons, HUD,
  pause), pre-release smoke, grep gate.
- **Phase 2 — Bug-fix pass** (each fix with owning verification): casino gambler tab
  magenta/empty; lab/crate capture modes don't open overlays; DailyDescent + PetSelection
  automation names; smoke case-05 screenshot flake; frontend Settings tag-click smoke
  early exit; durable-save-integrity stale slot; top-bar nav dump anchor flake; packaged
  quit 0xC0000409 (investigation-class); plus quick ledger wins: VendorBoss
  CharacterVisuals row, StatusEffects.csv malformed rows, SC_Music/SC_SFX SoundClasses,
  Alice OST folder key, Item_Headshot sprites, DT_HouseNPCs missing refs, Niagara C4996,
  unreached Bounce branch, gameplay HUD dump target selector.
- **Phase 3 — Stats Phase 3b display** (approved add-on): +% display language across
  stats panel / power-up / tooltips / item cards; save-field review of legacy base-stat
  snapshot fields. Balance playtest stays user-owned.
- **Phase 4 — Ledger + audit reconciliation, then final gate**: close stale/mooted
  entries across all 24 `pending_issues_*.md` ledgers (mark resolved with evidence);
  re-triage the 6 docs in `Audit/Pending/` against the post-cleanup tree (fix real
  findings in Phase 2 style, reclassify/move stale ones to Finished/Reference); fix
  `Audit/README.md` "no active pending" contradiction; consolidate surviving deliberate
  deferrals into one backlog doc. Final gate: StageStandaloneBuild + pre-release smoke
  suite; push.

## Explicitly OUT of scope

UI Reimagine overnight run (own contract: `Reports/AgentReviews/UIReimagine/run_contract.md`,
own approval gate); hero motion rebuild; FriendSlop look rollout to mobs/props; leaderboard
port completion; footstep audio; mob status-effect feature; controller focus contract;
Game Over rank submission service; theme atmosphere authoring; VAT crowd profiling;
WidgetGames module extraction; floor-seam rework; female Hero 1 model.

## Limit handling (user instruction, 2026-06-09)

If the 5-hour usage window is hit during the program, DO NOT end the program — park and
auto-resume:
- Poll the verified OAuth usage endpoint ~hourly during long phases (same pattern as the
  overnight usage guard) so the window reset time is known in advance.
- Near exhaustion: finish the atomic step in flight, write a checkpoint to
  `progress_log.md` in this folder (phase, item, verification state), arm a background
  timer task that exits just after the reset time (its completion re-invokes the session),
  then go quiet.
- On wake: re-check usage, resume from the checkpoint.
- The WEEKLY guard is different: self-stop at 80% weekly remains a hard stop (do not
  sleep-wait days for a weekly reset).

## Continuation rule

On any continuation: read this file first, then `progress_log.md` if it exists. If the
user's full commit has not happened yet, the program has not started — do not begin
Phase 1 against a dirty tree.
