# T66 Backend 0.1 Infrastructure Checklist

**Date revised:** 2026-04-04  
**Scope:** Current live backend state for the first private Steam `0.1` build.

## 1. Current Live Stack

- `DONE` Hosting: Vercel
- `DONE` Backend repo: `C:\UE\Backend`
- `DONE` Framework: Next.js 15
- `DONE` ORM: Drizzle
- `DONE` Persistent storage: Neon / Vercel Postgres
- `DONE` Temporary cache / rate limit layer: Upstash Redis / Vercel KV
- `DONE` Identity layer: Steam Web API ticket auth

## 2. Verified Live Status

- `DONE` Production alias is live at [https://t66-backend.vercel.app](https://t66-backend.vercel.app)
- `DONE` Production deploy succeeded from the current backend repo on 2026-04-04
- `DONE` Production leaderboard reads are live
- `DONE` Production run-summary route is live and now returns `proof_locked`
- `DONE` Production and development `STEAM_APP_ID` are set to `4464300`
- `DONE` Preview `STEAM_APP_ID` is set to `4464300` for preview branch `version-0.5`
- `DONE` Seeded production leaderboard/profile data has been removed
- `BLOCKER` Production health is still degraded:
  - `postgres: ok`
  - `kv: error: internal error`

## 3. Environment Inventory

### Required and present in Vercel

- `POSTGRES_URL`
- `KV_REST_API_URL`
- `KV_REST_API_TOKEN`
- `STEAM_WEB_API_KEY`
- `STEAM_APP_ID`
- `ADMIN_PASSWORD`

### Still to verify explicitly for `0.1`

- `CRON_SECRET`

### Dev-only

- `DEV_STEAM_IDS`

## 4. Database Tables In Scope For 0.1

Verified in `C:\UE\Backend\src\db\schema.ts`:

1. `player_profiles`
2. `leaderboard_entries`
3. `run_summaries`
4. `streamer_whitelist`
5. `account_restrictions`
6. `run_reports`
7. `quarantined_runs`
8. `bug_reports`
9. `audit_log`

## 5. Routes That Are Now Real For 0.1

- `DONE` `GET /api/leaderboard`
- `DONE` `GET /api/leaderboard/friends`
- `DONE` `GET /api/run-summary/:id`
- `DONE` `POST /api/report-run`
- `DONE` `PUT /api/proof-of-run`
- `DONE` `POST /api/submit-appeal`
- `DONE` `POST /api/bug-report`
- `DONE` `GET /api/account-status`

## 6. Data Reality Check

- `DONE` Seeded production leaderboard rows were removed.
- Current live counts after cleanup:
  - `player_profiles`: `0`
  - `leaderboard_entries`: `0`
  - `run_summaries`: `0`
  - `streamer_whitelist`: `0`

## 7. UE Client / Backend Integration State

- `DONE` Proof-of-run is wired from UE to backend.
- `DONE` Run report is wired from UE to backend.
- `DONE` Bug report is wired from UE to backend.
- `DONE` Appeal submission is wired from UE to backend.
- `DONE` Account status refresh is wired from UE to backend.
- `DONE` Online leaderboard row click no longer fabricates fake backend run summaries.
- `DONE` Solo score submission is wired to backend.
- `DONE` Solo completed-run speedrun submission is wired to backend.
- `DEFERRED` Co-op leaderboard submission is intentionally disabled until full party payload support exists.

## 8. Backend Code / Docs Cleanup Completed

- `DONE` `.env.example` now uses `STEAM_APP_ID=4464300`
- `DONE` Proof-of-run route now locks proofs on update
- `DONE` Run-summary route now exposes `proof_locked`
- `DONE` Backend repo builds successfully with `npm run build`

## 9. Remaining True Backend Blockers

1. `BLOCKER` Fix KV health in production.
2. `REMAINING` Confirm cron secret / cron readiness if those jobs are required for `0.1`.
3. `REMAINING` Decide whether Account Status reviewed-run deep-linking is required in the first Steam build.

## 10. Practical Meaning For 0.1

If you package today without further backend work:

- leaderboard/reporting/proof/account-status flows are much more real than before
- the public board is now clean and empty
- and the health check still reports KV as broken

So the backend is now **cleaner and usable**, but not yet **fully healthy** for the first honest Steam `0.1` package.
