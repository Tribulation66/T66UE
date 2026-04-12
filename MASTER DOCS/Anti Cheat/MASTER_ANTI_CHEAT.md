# T66 Master Anti-Cheat

**Last updated:** 2026-04-11  
**Scope:** Single-source handoff for anti-cheat telemetry capture, backend validation, moderation, account standing, and leaderboard eligibility.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_BACKEND.md`  
**Execution checklist:** `MASTER DOCS/Anti Cheat/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`  
**Maintenance rule:** Update this file after every anti-cheat, run-summary telemetry, moderation, restriction, appeal, or leaderboard-eligibility change. If the change affects live backend behavior, update `MASTER DOCS/MASTER_BACKEND.md` in the same pass.

## 1. Executive Summary

- Anti-cheat is currently backend-authoritative for leaderboard eligibility.
- The client gathers telemetry and submits it, but the backend decides whether a run is accepted, quarantined, suspended, or banned.
- The live system now checks hard score/time bounds, inventory/idol limits, duplicate submissions, suspicious luck distributions, internally inconsistent dodge telemetry, and statistically abnormal dodge performance.
- `Skill Rating` is no longer damage-only; it now scores 5-second combat-pressure windows using applied hits, incoming hit checks, observed dodges, and expected dodges, and the backend now enforces a hard upper bound on the submitted rating based on those five-second windows.
- Dodge/skill telemetry now includes aggregate evasion buckets and a compact 5-second pressure-window summary, so the backend can still judge combat shape after sampled hit-check events truncate.
- Gambler minigames now feed the anti-cheat path too: the client records per-game summary totals plus per-round gambler events, the backend can replay coin flip, RPS, lottery, plinko, and box-opening outcomes from the submitted telemetry, and those outcomes now contribute to `Luck Rating` instead of living only in UI state.
- Suspicion-level and certainty-level restrictions are persisted to the backend, surfaced in the account-status flow, and now remove restricted accounts from leaderboard reads and rank calculations.
- Active PB run summaries and quarantined run summaries now persist the submitted anti-cheat context plus the backend verdict metadata used to review the run later.
- Admin review now has a structured evidence surface for quarantine, appeals, and account lookup instead of relying only on raw JSON dumps.
- The system is still client-trusting, but it now includes exact backend replay for the supported replayable luck-event subset.
- It is not yet a full deterministic run replay or server-simulated anti-cheat.

## 2. Current Enforcement Model

### 2.1 Source of truth

- Leaderboard eligibility is determined by the backend in `C:\UE\Backend\src\app\api\submit-run\route.ts`.
- Existing restrictions are stored in `C:\UE\Backend\src\db\schema.ts` under `account_restrictions`.
- Flagged runs are stored in `quarantined_runs` and linked back to a saved `run_summaries` row for review.

### 2.2 Restriction levels

- `suspicion`: account is suspended from new ranked submissions, can appeal, and is hidden from leaderboards while the restriction exists.
- `certainty`: account is banned from new ranked submissions, is hidden from leaderboards, and is treated as confirmed cheating unless manually cleared.

### 2.3 What the backend enforces today

- Hard score caps by difficulty.
- Minimum plausible completion time by difficulty.
- Inventory-size ceiling by difficulty.
- Maximum equipped idols.
- Duplicate-submission rate limiting.
- Luck telemetry heuristics based on sample counts, ratings, and the player's submitted luck stat.
- Auto-dodge heuristics based on expected evasion chance versus observed dodges, dodge streaks, and low-evasion bucket anomalies.
- Internal consistency checks for submitted dodge telemetry, including impossible dodge-plus-damage sampled events and full-sample max-streak consistency.
- Skill-rating consistency checks, including a hard maximum reachable rating based on run length, pressure-window consistency checks, and a conservative high-pressure suspicion rule when near-perfect skill aligns with abnormal dodge outcomes.

## 3. End-to-End Flow

1. `UT66RunStateSubsystem` records run telemetry during gameplay.
2. `UT66SkillRatingSubsystem` computes a coarse 0-100 skill rating from combat windows.
3. `UT66LeaderboardSubsystem` snapshots ratings and telemetry into `UT66LeaderboardRunSummarySaveGame`.
4. `UT66BackendSubsystem` serializes the run summary plus `anti_cheat_context` into the backend request body.
5. `POST /api/submit-run` authenticates the player, checks existing restrictions, and runs `runAntiCheatChecks()` for the host and each submitted co-op member.
6. Clean runs are processed into `leaderboard_entries` and `run_summaries`.
7. Flagged runs are quarantined, linked to an `account_restrictions` row, and returned as `status: "flagged"`.
8. `GET /api/account-status` exposes restriction state, appeal status, and the reviewed run summary ID.
9. The UE leaderboard/account-status flow caches that record and lets the player open the reviewed run from the account screen.
10. Public leaderboard, rank, and run-summary endpoints filter restricted accounts out of visible rankings.

## 4. Anti-Cheat File Hierarchy

```text
C:\UE\T66
├── MASTER DOCS
│   └── Anti Cheat
│       └── MASTER_ANTI_CHEAT.md
└── Source\T66
    ├── Core
    │   ├── T66RunStateSubsystem.h / .cpp
    │   ├── T66SkillRatingSubsystem.h / .cpp
    │   ├── T66RunSaveGame.h
    │   ├── T66LeaderboardRunSummarySaveGame.h
    │   ├── T66LeaderboardSubsystem.h / .cpp
    │   ├── T66SessionSubsystem.h / .cpp
    │   └── T66BackendSubsystem.cpp
    └── Gameplay
        └── T66PlayerController.h / .cpp
    └── UI
        └── T66GamblerOverlayWidget.h / .cpp
    └── UI\Screens
        └── T66AccountStatusScreen.cpp

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

## 5. Client-Side Responsibilities

### 5.1 `Source\T66\Core\T66RunStateSubsystem.h` / `Source\T66\Core\T66RunStateSubsystem.cpp`

This is the gameplay telemetry source for anti-cheat. It currently owns:

- run-level luck rating aggregation
- luck quantity and quality sample counts
- incoming hit-check count
- applied-damage hit count
- dodge count
- current and maximum consecutive dodge streaks
- accumulated evasion chance across hit checks
- evasion-bucket dodge aggregates
- 5-second combat-pressure aggregates
- save/load persistence of those counters through the run snapshot path

This is also where dodge telemetry is produced inside `ApplyDamage()`. The backend's dodge checks only exist because this subsystem now records the per-hit aggregate counters.

### 5.2 `Source\T66\Core\T66SkillRatingSubsystem.h` / `Source\T66\Core\T66SkillRatingSubsystem.cpp`

This subsystem computes the submitted `skill_rating`.

- It starts from a default rating of 50.
- It tracks five-second combat windows.
- It penalizes windows by how many hits the player takes.
- It does not directly prove cheating. It is a supporting signal that the backend cross-checks against run length, hit pressure, and dodge abnormality.

### 5.3 `Source\T66\Core\T66RunSaveGame.h`

Persists the anti-cheat counters inside the active run save so mid-run save/load does not wipe the telemetry needed for submission review.

### 5.4 `Source\T66\Core\T66LeaderboardRunSummarySaveGame.h`

Stores the finalized run summary fields that get submitted or viewed later:

- `RunSeed`
- `LuckQuantitySampleCount`
- `LuckQualitySampleCount`
- `IncomingHitChecks`
- `DamageTakenHitCount`
- `DodgeCount`
- `MaxConsecutiveDodges`
- `TotalEvasionChance`
- `AntiCheatEvasionBuckets`
- `AntiCheatPressureWindowSummary`
- `LuckRating0To100`
- `LuckRatingQuantity0To100`
- `LuckRatingQuality0To100`
- `SkillRating0To100`

### 5.5 `Source\T66\Core\T66LeaderboardSubsystem.h` / `Source\T66\Core\T66LeaderboardSubsystem.cpp`

Bridges runtime systems into a submission snapshot and owns the client-side account restriction cache.

It currently:

- copies anti-cheat telemetry from `UT66RunStateSubsystem`
- copies skill rating from `UT66SkillRatingSubsystem`
- blocks new local submissions when the cached backend account restriction says the player is suspended or banned
- caches the reviewed backend run summary used by the account-status screen

### 5.6 `Source\T66\Core\T66BackendSubsystem.cpp`

Serializes the anti-cheat fields into backend JSON under `anti_cheat_context`.

Current submitted telemetry fields:

- `run_seed`
- `luck_quantity_sample_count`
- `luck_quality_sample_count`
- `luck_quantity_categories`
- `luck_quality_categories`
- `luck_events`
- `luck_events_truncated`
- `hit_check_events`
- `hit_check_events_truncated`
- `incoming_hit_checks`
- `damage_taken_hit_count`
- `dodge_count`
- `max_consecutive_dodges`
- `total_evasion_chance`
- `evasion_buckets`
- `pressure_window_summary`
- `gambler_summary`
- `gambler_events`
- `gambler_events_truncated`

Important caveat:

- This file still has a fallback path that fabricates placeholder luck/skill values for dummy leaderboard summaries when backend data is missing. Those placeholder values are UI filler only and must not be treated as anti-cheat evidence.
- In co-op, the host no longer clones its own summary for every member. Each client now builds its own finalized run JSON locally, forwards it to the host keyed by the backend request key, and the host retries briefly until all member payloads are available before submitting.

### 5.7 `Source\T66\Core\T66SessionSubsystem.h` / `Source\T66\Core\T66SessionSubsystem.cpp`

This subsystem now caches per-request co-op run summaries on the host.

It currently:

- clears stale cached member summaries when a new run starts or the party session ends
- accepts local player run-summary handoff calls from the backend submit path
- stores member-owned run JSON by `RequestKey` and `SteamId`
- exposes the cached member summary back to the host backend submission path

### 5.8 `Source\T66\Gameplay\T66PlayerController.h` / `Source\T66\Gameplay\T66PlayerController.cpp`

This is the transport bridge for co-op provenance.

It currently:

- forwards non-host finalized run summaries to the server via a reliable RPC
- stores host-local summaries directly when authority already owns the player state
- ties the stored summary to the sender's `AT66SessionPlayerState` Steam identity on the host
### 5.9 `Source\T66\UI\Screens\T66AccountStatusScreen.cpp`

This is the player-facing restriction surface.

It currently:

- shows whether the account is in good standing
- explains that suspended accounts are not leaderboard-eligible
- surfaces backend restriction reasons
- exposes the appeal action for suspicion-level cases
- lets the player open the reviewed run summary that triggered the restriction

### 5.10 `Source\T66\UI\T66GamblerOverlayWidget.h` / `Source\T66\UI\T66GamblerOverlayWidget.cpp`

This widget is now part of the anti-cheat capture surface, not just presentation.

It currently:

- routes gambler minigame RNG through the shared run RNG where the outcome matters for anti-cheat
- records gambler cheat-attempt success rolls into `Luck Rating`
- records coin-flip, RPS, lottery, plinko, box-opening, and blackjack outcomes into the anti-cheat summary path
- captures per-round gambler evidence such as bet size, payout, cheat attempt/success, lottery masks, plinko path bits, and blackjack action sequence

## 6. Backend Responsibilities

### 6.1 `C:\UE\Backend\src\lib\schemas.ts`

Defines the accepted anti-cheat payload shape through `runDataSchema`.

The backend currently accepts:

- ratings: `luck_rating`, `luck_quantity`, `luck_quality`, `skill_rating`
- anti-cheat context: seed, luck sample counts, hit checks, damage hits, dodge count, max dodge streak, total evasion chance, evasion buckets, and pressure-window summaries

### 6.2 `C:\UE\Backend\src\lib\anti-cheat.ts`

This is the core rules engine.

Current detection categories:

- `score`
- `speed`
- `items`
- `rate_limit`
- `luck_rating`
- `skill_rating`
- `luck_probability`
- `dodge_telemetry`
- `dodge_bucket_telemetry`
- `auto_dodge`
- `auto_dodge_bucket`
- `skill_pressure_telemetry`
- `skill_pressure`

Current dodge/skill model:

- The backend computes expected dodges from submitted `total_evasion_chance`.
- It compares expected versus observed dodges over the submitted hit-check count.
- It cross-checks full-run dodge counters against submitted evasion buckets and pressure-window summaries.
- It can flag abnormal dodge behavior inside low-evasion buckets even when aggregate averages look less extreme.
- It calculates a conservative z-score and excess-dodge count.
- `skill_rating` is still not a standalone cheat verdict, but it is now cross-checked against run length and pressure-window shape.

Current luck model:

- The backend uses submitted sample counts plus `luck_rating`, `luck_quantity`, and `luck_quality`.
- It derives a conservative expected luck envelope from the player's submitted luck stat.
- It flags runs whose observed aggregate luck is implausibly high for the number of observed samples.

### 6.3 `C:\UE\Backend\src\app\api\submit-run\route.ts`

This is the enforcement entry point.

It currently:

- authenticates the Steam ticket
- blocks already-restricted hosts and co-op members
- runs anti-cheat on the host payload
- runs anti-cheat on each submitted co-op member payload
- quarantines flagged runs
- creates or updates `account_restrictions`
- prevents flagged runs from being processed into leaderboard PB rows

### 6.4 `C:\UE\Backend\src\db\schema.ts`

Defines the anti-cheat persistence model:

- `run_summaries`
- `account_restrictions`
- `quarantined_runs`
- `audit_log`
- `run_reports`

`run_summaries` now also persist:

- submitted `anti_cheat_context`
- backend verdict (`clean`, `suspicion`, `certainty`)
- backend flag category and reason
- backend `supportingData`
- backend evaluation timestamp

### 6.5 `C:\UE\Backend\src\app\api\account-status\route.ts`

Returns the current backend standing record used by the UE account screen:

- restriction level
- reason
- flag category
- appeal status
- reviewed `run_summary_id`

### 6.6 Leaderboard visibility routes

The following routes now filter restricted accounts out of visible rankings:

- `C:\UE\Backend\src\app\api\leaderboard\route.ts`
- `C:\UE\Backend\src\app\api\leaderboard\friends\route.ts`
- `C:\UE\Backend\src\app\api\my-rank\route.ts`

This means restricted players stop appearing in leaderboard reads and total-entry counts without deleting the underlying data.

### 6.7 `C:\UE\Backend\src\app\api\admin\action\route.ts`

Supports manual moderation actions:

- approve appeal
- deny appeal
- suspend
- ban
- unsuspend
- clear quarantine
- confirm cheat

### 6.8 `C:\UE\Backend\src\app\admin\quarantine\page.tsx` / `C:\UE\Backend\src\app\admin\appeals\page.tsx` / `C:\UE\Backend\src\app\admin\accounts\page.tsx`

These admin pages now render reviewer-oriented anti-cheat evidence cards.

They currently surface:

- run overview values such as score, time, difficulty, hero, and ratings
- luck evidence such as sample counts and replay mismatches
- dodge evidence such as hit checks, expected dodges, z-score, and suspicious low-evasion buckets
- pressure-window evidence such as pressured-window counts and near-perfect high-pressure windows
- raw JSON as a fallback expansion panel

### 6.9 `C:\UE\Backend\src\app\api\admin\account\route.ts`

Account lookup now also returns the reviewed linked run-summary row for a restriction so admins can inspect the stored verdict and supporting evidence directly.

## 7. What Is In Place Right Now

- Backend-authoritative suspension and ban state.
- Automatic quarantine plus restriction writes for flagged runs.
- Account-status integration in the shipped UI flow.
- Reviewed-run retrieval from the account-status screen.
- Leaderboard submission blocking on both the client and backend.
- Leaderboard-read filtering for restricted players.
- Run-summary visibility filtering for restricted entries and restricted co-op member slots.
- Statistical detection for suspicious luck and auto-dodge patterns.
- Sampled per-roll luck events and per-hit dodge-check events now travel in `anti_cheat_context` for post-run review.
- Supported luck events now carry replay metadata (`pre_draw_seed`, optional `draw_index`, expected chance, baseline rarity weights, and float replay bounds where needed) so the backend can exactly re-run those outcomes.
- The backend now exactly replays supported `quantity_roll`, `quantity_bool`, and `quality_rarity` events before it falls back to envelope-based luck suspicion checks.
- Backend verdict metadata is now stored on active PB summaries and quarantined summaries for later review.
- Co-op member submissions are now built from member-owned finalized run summaries instead of the host cloning one snapshot across the party.
- Host co-op submissions now retry briefly until remote member summaries arrive, then submit per-member evidence under the matching backend request key.
- Admin review pages now surface structured evidence for quarantine, appeals, and account lookup.

## 8. What Is Still Missing

### 8.1 Co-op delivery resilience

- Co-op summaries are now member-owned and host-aggregated, but they are still delivered at end-of-run over a live client-to-host RPC path.
- If a remote player never reaches the run-summary submit path or disconnects before handing off their summary, the host waits briefly and then the co-op backend submission fails.
- There is still no signed attestation or relay-backed fallback for missing member summaries.

### 8.2 Deterministic RNG / spawn replay

- The backend now exactly replays the supported luck-event subset when the client submits replay metadata for that event.
- Supported exact replay currently covers the event families we tag with `pre_draw_seed` and the data needed to reproduce the outcome: biased integer rolls including hero level-up gains, rounded float-biased rolls such as wheel gold, Bernoulli chance rolls such as vendor/gambler/lucky alchemy plus spawn checks like goblin-wave, mini-boss-wave, chest-mimic, and tower special-floor outcomes, weighted rarity rolls such as crate reward rarity plus guaranteed-start and catch-up loot bag rarity, and gambler minigame rounds for coin flip, RPS, lottery, plinko, box opening, and blackjack.
- The client still does not submit full run-seed lineage for every shared-stream draw, and some luck paths still only contribute sampled or envelope-based evidence.
- Result: probability tampering is now stronger to detect and prove for supported event types, but not yet for every run-seeded reward/spawn path.

### 8.3 Full event-ledger proof

- The backend now receives capped sampled luck events and capped sampled hit-check events.
- It cross-checks those samples against the submitted aggregate counters when the samples are not truncated.
- The system still does not have a full uncapped event ledger or deterministic replay of every hit check and every run-stream draw.
- A compromised client could still fabricate internally consistent sampled telemetry.

### 8.4 Broader skill-cheat coverage

- There is still no detection for impossible fire rate, scripted attacks, movement macros, impossible DPS cadence, or other automation outside dodge behavior.
- `skill_rating` is now a better combat-pressure support signal, but it is still not a real standalone cheat detector.

### 8.5 Accepted non-PB evidence retention

- `run_summaries` persist anti-cheat evidence for active PB entries and quarantined runs.
- Clean accepted runs that do not become or replace an active PB still do not get their own durable anti-cheat record.
- If full historical review of every clean submission matters, this needs a dedicated accepted-run evidence table or archive path.

### 8.6 Admin evidence surfacing

- The first usable evidence surface now exists.
- The remaining gap is diagnostic polish: threshold highlights, easier cross-run comparison, and direct linking between every admin list and the reviewed run summary.

## 9. Practical Next Steps To Finish The System

1. Expand exact replay coverage to the remaining luck-driven event families that still only submit sampled or envelope telemetry.
2. Upgrade the capped sampled event stream into a fuller replayable ledger where needed.
3. Add heuristics for impossible DPS, attack cadence, and movement scripting.
4. Decide whether clean non-PB runs need durable anti-cheat evidence retention.
5. Improve the admin evidence surface from readable to diagnostic.
6. Decide whether co-op member summaries need stronger delivery guarantees than the current host retry window.
