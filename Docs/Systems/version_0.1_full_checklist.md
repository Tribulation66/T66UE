# T66 Version 0.1 Full Release Checklist

**Date revised:** 2026-04-04  
**Target:** First private Steam build (`0.1`) with real Steam identity, real backend leaderboard data, real run-summary/reporting flows, and duo invite/session gameplay.

## 1. Current Status

### Completed in code / live backend

- `DONE` Real Steam AppID is aligned to `4464300` in the UE project config and in Vercel production/development env.
- `DONE` The backend is deployed on Vercel at [https://t66-backend.vercel.app](https://t66-backend.vercel.app).
- `DONE` `GET /api/run-summary/:id` now returns `proof_locked` in production.
- `DONE` UE leaderboard rows no longer fabricate fake backend run summaries when online data is missing.
- `DONE` UE run-summary screen now sends real backend requests for:
  - proof-of-run updates
  - run reports
- `DONE` UE account status now refreshes from backend and appeal submission now calls `/api/submit-appeal`.
- `DONE` UE bug report submission now posts to `/api/bug-report` and no longer shows placeholder `Sentry` wording.
- `DONE` Shipping dev auth fallback is disabled:
  - `DevTicket=`
  - `DevFriendIds=`
  - shipping build no longer falls back to fake Steam auth
- `DONE` Solo backend leaderboard submission is real for:
  - score
  - completed-run speedrun submission
- `DONE` Seeded production leaderboard/profile data has been removed. The public board is now clean and empty until real submissions arrive.
- `DONE` Both `T66Editor Win64 Development` and `T66 Win64 Development` compile successfully after the `0.1` online/reporting changes.

### Intentionally deferred for `0.1`

- `DEFERRED` Full co-op leaderboard submission is not implemented yet.
- `DEFERRED` Backend leaderboard submission for co-op parties is now intentionally skipped instead of silently submitting host-only data.
- `DEFERRED` Account Status "reviewed run" deep-link is not yet backend-driven end to end.

### Remaining blockers before packaging

1. `BLOCKER` Production KV is still unhealthy.
   - `/api/health` currently returns:
   - `postgres: ok`
   - `kv: error: internal error`

2. `BLOCKER` Steamworks build delivery is not finished.
   - No packaged upload yet
   - No private branch build live yet
   - No release override key issued yet

3. `BLOCKER` Steam rich presence strings are not yet set up in Steamworks.
   - `#Status_MainMenu`
   - `#Status_InLobby`

4. `BLOCKER` The first real two-player Steam invite test has not happened yet.

## 2. What `0.1` Must Mean

`0.1` is ready when all of this is true:

1. The game launches through Steam using AppID `4464300`.
2. Host can invite a Steam friend.
3. Friend receives the invite and joins the host lobby.
4. Both players enter the same game world.
5. The visible leaderboard uses real backend data only.
6. Clicking a leaderboard entry opens a real backend run summary.
7. Proof-of-run links save through the backend.
8. Run reporting hits the backend and is stored.
9. Backend production health is clean enough for external testing.

## 3. Steam / Social Checklist

- `DONE` Real AppID path is in place in the UE project.
- `DONE` Steam session/invite/join/shared-travel foundation is implemented and compiled.
- `REMAINING` Upload a private Steam build.
- `REMAINING` Set that build live on a private branch.
- `REMAINING` Request a very small release override key batch.
- `REMAINING` Add Steam rich presence localization entries.
- `REMAINING` Run the first real two-person invite test.

## 4. Backend Hosting Checklist

- `DONE` Vercel project `t66-backend` is linked locally.
- `DONE` Production deploy succeeded after the route changes.
- `DONE` Production AppID env is corrected to `4464300`.
- `DONE` Development AppID env is corrected to `4464300`.
- `DONE` Preview AppID env is corrected for preview branch `version-0.5`.
- `DONE` `.env.example` now reflects `STEAM_APP_ID=4464300`.
- `REMAINING` Fix the KV service/config problem behind `/api/health`.
- `REMAINING` Verify cron secret/config if you want weekly reset + retention active for `0.1`.

## 5. Leaderboard Checklist

- `DONE` Online leaderboard rows use backend data.
- `DONE` Online row click opens cached backend run summaries only.
- `DONE` Fake/generated run summary fallback for online rows was removed.
- `DONE` Proof lock state is now returned by the live backend route.
- `DONE` Solo score submission is real.
- `DONE` Solo completed-run speedrun submission is real.
- `DEFERRED` Co-op leaderboard submission is disabled until full per-party payload support exists.
- `DONE` Production seed rows were removed from the public environment.

## 6. Reporting / Moderation Checklist

- `DONE` Report run -> `/api/report-run`
- `DONE` Proof of run -> `/api/proof-of-run`
- `DONE` Submit appeal -> `/api/submit-appeal`
- `DONE` Bug report -> `/api/bug-report`
- `DONE` Account status refresh -> `/api/account-status`
- `REMAINING` Decide whether Account Status reviewed-run linking is required for `0.1`

## 7. Packaging / Build Checklist

- `DONE` Project version fallback is set to `0.1` in config for bug-report output.
- `REMAINING` Package the Windows build.
- `REMAINING` Upload through SteamPipe / ContentBuilder.
- `REMAINING` Install through Steam on two machines.
- `REMAINING` Run the first full Steam invite + join + world-entry test.

## 8. True Go / No-Go Gate

Do **not** package the Steam `0.1` build until these are true:

1. KV health is no longer red.
2. Steam build delivery is ready.
3. Real two-player Steam invite flow passes.
4. Any visible online/reporting surface is real, not placeholder.

## 9. Immediate Remaining Order

1. Fix production KV health.
2. Add Steam rich presence strings in Steamworks.
3. Package and upload the private build.
4. Run the first two-player Steam test.
