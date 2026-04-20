# T66 Master Anti-Cheat

**Last updated:** 2026-04-19  
**Scope:** Single-source handoff for anti-cheat policy, ranked eligibility, integrity attestation, provenance validation, moderation, and leaderboard authority.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_BACKEND.md`  
**Execution checklist:** `ANTI_CHEAT/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`  
**Maintenance rule:** Update this file after every anti-cheat, run-summary telemetry, restriction, appeal, integrity, run-modifier, or leaderboard-eligibility change. If the change affects live backend behavior, update `MASTER DOCS/MASTER_BACKEND.md` in the same pass.

## 1. Executive Summary

- Mainline T66 ranked leaderboard authority belongs to the backend, not to Steam leaderboards and not to client-local PB state.
- Ranked mode now has a strict policy target: pristine build only. Challenges, internal mods, Workshop mods, and other runtime modifiers are intentionally unranked.
- Steam trusted leaderboard writes are not a mod detector. If official Steam leaderboard mirroring is ever added, the T66 backend should remain the only caller to Valve's score-write API after T66's own validation passes.
- Local leaderboard state should be treated as a cache of backend-accepted ranked results only. Recent-run history can still be stored locally, but ranked PB/rank state should not advance before the backend returns `accepted`.
- Anti-cheat is moving to a layered model:
  - existing backend heuristics and moderation
  - integrity attestation based on a launch/run-start baseline plus run-end compare
  - compact event/provenance validation for items, stats, currency, and progression
- The design goal is still low runtime overhead. No per-tick anti-cheat scanning is planned. Integrity checks should be boundary-driven and event-driven.
- Replayable luck-event metadata must only be attached when the submitted event maps directly to the underlying RNG draw. Derived success booleans such as "avoided" / "win" outcomes and draws taken from non-primary RNG streams should stay in aggregate telemetry but omit replay seeds so the backend does not replay the wrong predicate.

## 2. Ranked Eligibility Policy

### 2.1 Source of truth

- Ranked eligibility is determined by the backend in `C:\UE\Backend\src\app\api\submit-run\route.ts`.
- The client may pre-mark a run as ineligible or unranked, but the client is not the final authority for ranked acceptance.
- `leaderboard_entries` should only be written for backend-accepted ranked runs.

### 2.2 Ranked-only requirements

A run is ranked only if all of the following are true:

- the player is not restricted
- the run is submitted to the backend successfully
- the build/integrity attestation matches an allowlisted pristine build
- no challenge or mod run modifier is active
- no extra mounted content or runtime mutation marks the run as modded/unranked
- anti-cheat heuristics and provenance checks do not flag the run

### 2.3 Explicitly unranked cases

These runs are expected to remain playable but unranked:

- challenge runs
- internal mod-tab runs
- future Workshop or loose-file mod runs
- integrity-mismatch runs
- offline runs or runs that never receive backend acceptance

### 2.4 Restriction levels

- `suspicion`: account is suspended from new ranked submissions, can appeal, and is hidden from leaderboards while the restriction exists.
- `certainty`: account is banned from new ranked submissions, is hidden from leaderboards, and is treated as confirmed cheating unless manually cleared.

## 3. Authority Model

### 3.1 T66 leaderboard authority

- The client gathers evidence and submits candidate runs.
- The backend authenticates the Steam ticket, checks account standing, evaluates anti-cheat and integrity, and decides the verdict.
- Only backend-accepted ranked runs should advance durable leaderboard PB/rank state.

### 3.2 Steam trusted writes

- Steam trusted leaderboard writes only mean clients cannot directly write official Steam leaderboard entries.
- They do not inspect Workshop state, mounted content, DLL injection, or changed game rules.
- If T66 later mirrors official scores to Steam leaderboards, the backend should be the only caller to `SetLeaderboardScore`, and only after T66 has already accepted the run as ranked.

### 3.3 Target verdict model

The target submit-run verdicts are:

- `accepted`: clean ranked run; backend may write leaderboard rows and the client may update ranked PB/rank cache
- `unranked`: intentionally non-ranked run, such as `challenge`, `run_modifier`, `modded`, `integrity_mismatch`, or `offline`
- `flagged`: suspicious run; quarantine and review path
- `suspended`: account-level ranked-submit suspension
- `banned`: account-level ranked-submit ban

## 4. Anti-Cheat Layers

### 4.1 Existing heuristic layer

The live backend already enforces:

- hard score and time plausibility bounds
- inventory/idol ceilings
- duplicate-submit rate limiting
- luck telemetry checks and exact replay for supported luck events
- dodge telemetry, bucket, and pressure-window checks
- skill-rating consistency checks tied to combat pressure

This layer remains active. The integrity and provenance work below is additive, not a replacement.

Replayability rule:

- Exact replay is only valid for events where the client can submit the real pre-draw seed for the exact sampled predicate.
- If the event records a derived outcome instead of the direct roll result, the client should not send replay-seed metadata for that event.
- If the event uses a secondary/local RNG stream instead of the primary run stream, the client should not send stale primary-stream metadata.

### 4.2 Integrity attestation layer

The next ranked gate is a lightweight integrity layer built around boundary snapshots rather than continuous scanning.

Policy:

- capture a pristine baseline before the run starts
- mark the run dirty immediately when known ranked-sensitive state changes
- compare the final state against the baseline at run end
- submit the compact integrity evidence to the backend for ranked acceptance

The target integrity context should include:

- `steam_app_id`
- `steam_build_id`
- `beta_name` if relevant
- `manifest_id`
- `manifest_root_hash`
- `module_list_hash`
- `mounted_content_hash`
- `modded`
- `mod_reasons`

Design notes:

- No per-tick hashing or file scanning.
- Hashing should happen at launch/run start and at run end.
- Runtime dirty bits should flip on controlled events such as enabling a run modifier, mounting content, or detecting unexpected module/content changes.
- This layer is intended to catch ordinary mods and many build-tamper paths, not to prove that a hostile client is impossible.

### 4.3 Event/provenance validation layer

The second new ranked gate is compact provenance accounting.

Instead of trying to replay the full run at frame granularity, the client should record only state mutations that matter to ranked plausibility:

- spawn baseline
- item added/removed
- gold added/spent
- stat gained/lost
- chest or loot source consumed
- vendor purchase or sell
- stage entered/cleared
- boss reward granted

The backend should then validate compact invariants such as:

- ending inventory equals legal additions minus removals
- ending gold equals legal gains minus spends
- ending stats reconcile with base values plus legal modifiers
- progression transitions follow legal stage flow
- important rewards can be traced to a valid source event

This layer should stay cheap and event-driven. It exists to make common cheat patterns harder to hide inside otherwise plausible aggregate telemetry.

## 5. End-to-End Ranked Flow

1. The client starts with a baseline integrity snapshot and the current run-modifier state.
2. `UT66RunStateSubsystem` records gameplay telemetry and compact provenance during the run.
3. `UT66SkillRatingSubsystem` computes the combat-pressure skill signal.
4. `UT66LeaderboardSubsystem` snapshots run data into `UT66LeaderboardRunSummarySaveGame`.
5. `UT66BackendSubsystem` serializes the run summary, `anti_cheat_context`, and the future `integrity_context`.
6. `POST /api/submit-run` authenticates the player, checks account restrictions, evaluates heuristics, integrity, and provenance, then returns a verdict.
7. Only `accepted` should advance ranked leaderboard rows and the local ranked PB/rank cache.
8. `unranked` runs may still keep recent-run history or audit records, but must not become ranked PBs.
9. `flagged` runs continue through quarantine and restriction review.

## 6. Anti-Cheat File Hierarchy

```text
C:\UE\T66
├── ANTI_CHEAT
│   ├── MASTER_ANTI_CHEAT.md
│   └── ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md
├── MASTER DOCS
│   ├── T66_MASTER_GUIDELINES.md
│   └── MASTER_BACKEND.md
└── Source\T66
    ├── Core
    │   ├── T66RunStateSubsystem.h / .cpp
    │   ├── T66SkillRatingSubsystem.h / .cpp
    │   ├── T66RunSaveGame.h
    │   ├── T66LeaderboardRunSummarySaveGame.h
    │   ├── T66LeaderboardSubsystem.h / .cpp
    │   ├── T66SessionSubsystem.h / .cpp
    │   ├── T66BackendSubsystem.cpp
    │   └── T66SteamHelper.cpp
    ├── Gameplay
    │   ├── T66GameMode.cpp
    │   └── T66PlayerController.h / .cpp
    └── UI
        ├── T66GamblerOverlayWidget.h / .cpp
        └── Screens
            ├── T66AccountStatusScreen.cpp
            ├── T66ChallengesScreen.cpp
            └── T66RunSummaryScreen.cpp

C:\UE\Backend
└── src
    ├── app\api
    │   ├── submit-run\route.ts
    │   ├── account-status\route.ts
    │   ├── leaderboard\route.ts
    │   ├── leaderboard\friends\route.ts
    │   ├── my-rank\route.ts
    │   ├── run-summary\[id]\route.ts
    │   └── admin\action\route.ts
    ├── db
    │   └── schema.ts
    └── lib
        ├── anti-cheat.ts
        └── schemas.ts
```

## 7. Client Responsibilities

### 7.1 `Source\T66\UI\Screens\T66ChallengesScreen.cpp`

- Owns player-facing selection of challenge and internal mod run modifiers.
- These selections are part of ranked eligibility policy, not just UI state.
- Current state: fresh runs with selected modifiers are already intended to be leaderboard-ineligible.

### 7.2 `Source\T66\Core\T66GameInstance.h` / `Source\T66\Gameplay\T66GameMode.cpp`

- Freeze the run-modifier verdict at run start.
- Preserve ranked ineligibility across save/load and resumed-run paths.
- Keep ranked and unranked policy decisions explicit instead of silently recomputing them later from partial UI state.

### 7.3 `Source\T66\Core\T66RunStateSubsystem.h` / `Source\T66\Core\T66RunStateSubsystem.cpp`

Current responsibility:

- live luck, dodge, pressure-window, and gambler telemetry

Expanded responsibility:

- compact provenance accounting for inventory, stats, currency, and progression
- cheap dirty-bit integration when ranked-sensitive runtime changes happen

### 7.4 `Source\T66\Core\T66RunSaveGame.h`

- Persist anti-cheat telemetry and any ranked-integrity/provenance state needed to preserve verdict continuity across mid-run save/load.

### 7.5 `Source\T66\Core\T66LeaderboardRunSummarySaveGame.h`

- Continue storing the finalized anti-cheat summary fields.
- Expand to carry compact integrity/provenance summary data that the backend can validate at submit time.

### 7.6 `Source\T66\Core\T66LeaderboardSubsystem.h` / `Source\T66\Core\T66LeaderboardSubsystem.cpp`

- Bridge runtime telemetry into the submission snapshot.
- Treat local ranked PB/rank state as a cache of backend-accepted results only.
- Recent run-history snapshots may still be stored locally, but ranked progression should not commit before backend acceptance.

### 7.7 `Source\T66\Core\T66BackendSubsystem.cpp`

- Serialize `anti_cheat_context` today.
- Add `integrity_context` and future provenance summaries to the submit payload.
- Treat `unranked` as an expected submit outcome instead of a generic failure.

### 7.8 `Source\T66\Core\T66SessionSubsystem.h` / `Source\T66\Core\T66SessionSubsystem.cpp`

- Preserve member-owned co-op run summaries on the host.
- Carry integrity/provenance evidence per member instead of cloning one host snapshot across the party.

### 7.9 `Source\T66\Core\T66SteamHelper.cpp`

- Remains the natural source for Steam app/build identity used by integrity attestation.
- If Steam Workshop is added later, this helper layer or a dedicated UGC wrapper can surface subscribed/mounted content information, but ranked authority still remains with the backend.

### 7.10 `Source\T66\UI\Screens\T66AccountStatusScreen.cpp`

- Continue surfacing restriction state, appeal state, and reviewed evidence.
- Future UX should also distinguish expected `unranked` outcomes from punitive enforcement.

## 8. Backend Responsibilities

### 8.1 `C:\UE\Backend\src\lib\schemas.ts`

- Validate the current `anti_cheat_context`.
- Expand the run contract to accept integrity and provenance context.

### 8.2 `C:\UE\Backend\src\lib\anti-cheat.ts`

- Keep the existing heuristic checks.
- Add integrity attestation checks and provenance/invariant checks.
- Separate intentional `unranked` verdicts from suspicious `flagged` verdicts.

### 8.3 `C:\UE\Backend\src\app\api\submit-run\route.ts`

- Authenticate the Steam ticket.
- Block already-restricted hosts and co-op members.
- Evaluate anti-cheat, integrity, and provenance for the host and each submitted co-op member.
- Write ranked leaderboard rows only for `accepted`.
- Keep `unranked` runs out of `leaderboard_entries` while still allowing history or audit persistence where useful.

### 8.4 `C:\UE\Backend\src\db\schema.ts`

- Continue storing `run_summaries`, `account_restrictions`, `quarantined_runs`, `audit_log`, and `run_reports`.
- Expand persistence so accepted, unranked, and flagged verdicts all keep the evidence needed for later review.

### 8.5 Account and admin surfaces

- `account-status` remains the player-facing restriction read path.
- Admin surfaces should keep rendering structured evidence, and later add direct integrity/provenance summaries alongside the existing luck and dodge evidence.

## 9. Current Implementation Snapshot

Already in place:

- backend-authoritative restriction and quarantine flow
- leaderboard-read filtering for restricted accounts
- luck, dodge, pressure-window, and gambler telemetry
- exact replay for the currently supported replayable luck-event subset
- structured admin evidence surfaces
- run-modifier-driven leaderboard ineligibility on the fresh-run path
- replayable luck events now clear stale primary-stream metadata when a secondary RNG stream is used, and derived boolean outcomes such as `ChestMimicAvoided` / gambler win-state telemetry now omit replay seeds to avoid false-positive `luck_replay` quarantines

Still missing or still being changed:

- no submit-time integrity context yet
- no baseline-versus-final integrity compare yet
- no compact provenance validator yet
- ranked PB/rank state is still too local-first and must move behind backend `accepted`
- resumed runs need explicit preservation of ranked ineligibility and integrity state, not just fresh-run setup

## 10. Implementation Priorities

1. Move the anti-cheat docs and align project policy references.
2. Invert the leaderboard flow so ranked PB/rank state updates only after backend `accepted`.
3. Add the integrity attestation layer with launch/run-start baseline, dirty bits, and run-end compare.
4. Extend backend submit contracts and verdict handling to support `unranked` cleanly.
5. Add compact provenance validation for inventory, stats, currency, and progression.
6. Expand admin/account evidence surfaces once the new layers exist.
