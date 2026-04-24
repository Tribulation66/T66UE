# T66 Master Backend

**Last updated:** 2026-04-20
**Scope:** Single-source handoff for T66 online state: Steam, Steamworks, Steam AppIDs, Vercel, backend services, live status, and the recommended path to private multiplayer testing under the real Steam AppID.
**Companion policy doc:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`
**Companion Steam ops doc:** `MASTER DOCS/MASTER_STEAMWORKS.md`
**Companion anti-cheat doc:** `ANTI_CHEAT/MASTER_ANTI_CHEAT.md`
**Historical predecessor docs:** `Docs/Archive/Systems/backend_architecture_historical.md`, `Docs/Archive/Systems/backend_0.1_checklist_historical.md`, `Docs/Archive/Systems/version_0.1_full_checklist_historical.md`
**Maintenance rule:** Update this file after every Steam, backend, online, multiplayer, upload, deployment, or other internet-connected systems change. Update `MASTER DOCS/MASTER_STEAMWORKS.md` in the same pass for any Steamworks/build/upload/private-test workflow change. If the change also affects project policy, workflow, or anti-cheat enforcement, update `MASTER DOCS/T66_MASTER_GUIDELINES.md` and `ANTI_CHEAT/MASTER_ANTI_CHEAT.md` in the same pass.
**Status note:** For the current Steamworks operational state, active build ID, upload workflow, and private-testing procedure, prefer `MASTER DOCS/MASTER_STEAMWORKS.md`. This backend document still contains historical transition details from the earlier `480` to `4464300` migration period.

## 1. Primary Source Files

This document consolidates the current state from these checked-in sources plus direct live checks:

- `Config/DefaultEngine.ini`
- `Config/DefaultGame.ini`
- `steam_appid.txt`
- `Source/T66/Core/T66SteamHelper.cpp`
- `Source/T66/Core/T66BackendSubsystem.cpp`
- `Source/T66/Core/T66SessionSubsystem.cpp`
- `Source/T66/UI/Screens/T66PartyInviteModal.cpp`
- `Tools/Steam/UploadToSteam.ps1`
- `Saved/Logs/T66_StagedGameplaySmoke*.log`
- `C:\UE\Backend\src\lib\leaderboard-keys.ts`
- `C:\UE\Backend\src\lib\steam.ts`
- `C:\UE\Backend\src\app\api\party-invite\*\route.ts`
- `C:\UE\Backend\src\app\api\client-diagnostics\route.ts`
- `C:\SteamworksSDK\sdk\tools\ContentBuilder\scripts\app_build_4464300_root.vdf`

Where this document contradicts the archived backend/reference docs, prefer this document. It is based on the currently checked-in config and current live backend response.

## 2. Executive Summary

- The project already has a real Steam multiplayer foundation in code.
- `MASTER DOCS/MASTER_STEAMWORKS.md` is now the primary operator-memory file for Steamworks uploads, current build tracking, key handling, and the local PowerShell workflow.
- The backend is real and deployed at `https://t66-backend.vercel.app`.
- The live backend health check is currently healthy as of 2026-04-20 UTC / 2026-04-20 local time.
- The main blocker is not "Steam is missing"; it is finishing the move from legacy Spacewar testing to the real app path.
- Ranked leaderboard acceptance remains backend-authoritative. Steam trusted leaderboard writes, if ever used for mirroring, do not replace T66's own mod, integrity, or anti-cheat gates.
- The checked-in UE config now defaults to real AppID `4464300`.
- Older staged logs still show previous packaged builds running under `480`, so fresh validation is still required.
- The backend party-invite and diagnostics routes still allow legacy `480` during the transition window.
- Structured multiplayer diagnostics now exist locally and through the backend so host/guest failures can be correlated without asking for full raw log files.
- The runtime leaderboard UI now exposes `solo`, `duo`, `trio`, and `quad` and no longer collapses `trio`/`quad` requests back to `duo`.
- Source now prevents score-only victory submits from seeding bogus `0 ms` speedrun leaderboard rows; backend speedrun keys are only generated for real completed-run submissions with `time_ms > 0`.
- Source now treats an empty Steam friends list as valid for the Friends leaderboard route and returns the requesting player's own row instead of failing the request.
- Source now parses backend speedrun leaderboard values into `TimeSeconds` for UI display, and completed-run local rank handling now uses the backend speedrun rank rather than the score rank.
- Stage-clear leaderboard submission now uses one authoritative completed-run request on the UE side instead of the earlier score-submit plus completed-run-submit pair.
- `/api/submit-run` now accepts optional `submission_id` and caches the final response per `steam_id + submission_id`, so a retry of the same logical submission replays the same result instead of being treated as a second run.
- Accepted leaderboard rows are now append-only per run instead of PB-only per player; `/api/my-rank` now resolves the player's single best row separately for the below-top-10 "you row".
- Production `run_summaries` schema drift was fixed live on 2026-04-19 by adding the missing anti-cheat and integrity columns that `/api/submit-run` already writes.
- Checked-in backend code now models the redesigned progression as Easy/Medium/Hard/VeryHard = 4 local stages and Impossible = 3 local stages, using `local_stage_reached` plus normalization helpers.
- Production was successfully redeployed on 2026-04-20 with deployment `dpl_9qrhYBkYE1Zxv8XoKGUgWAx1QEQR`, and the alias `https://t66-backend.vercel.app` now points at that deployment.
- Steam build `22947092` is now the newest uploaded client payload as of 2026-04-24. It contains the clean Steam leaderboard identity pass, including live Steam persona names and profile avatars in leaderboard rows, and remains not-live until the intended Steamworks branch is switched to that build.
- Suspicion-level anti-cheat restrictions are now backend-authoritative and block new score submissions the same way the frontend does.
- Automatic suspension reasons now distinguish `luck_rating` and `skill_rating` breaches over `100`, and the Account Status flow can open the reviewed backend run summary tied to the restriction.
- All-time proof-of-run data now persists for the active PB leaderboard entry until that PB is replaced or the entry is removed; weekly proof-backed summaries are wiped by the weekly reset.
- Run reports are now treated as a weekly moderation queue and are wiped by the weekly reset, with a 7-day TTL as fallback cleanup.
- Production streamer whitelist now includes SteamID `76561198749633075` (`Tribulation 66 Studios`) so accepted runs appear in the Streamers leaderboard filter for that account.
- A local Steam upload wrapper now exists and now defaults to the absolute root VDF after the relative script failed from SteamCMD path resolution.
- The recommended path remains: use base AppID `4464300`, keep the app unreleased, distribute a small number of Release Override keys, and iterate through SteamPipe uploads.

## 3. Steamworks App Inventory

### 3.1 Target app

- **Primary target app:** `CHADPOCALYPSE` (`4464300`)
- This should be treated as the real shipping AppID and the final multiplayer target.
- This is the AppID already wired into the local SteamPipe script at:
  - `C:\SteamworksSDK\sdk\tools\ContentBuilder\scripts\CHADPOCALYPSE\app_build_4464300.vdf`

### 3.2 Secondary app

- **Secondary app:** `Chadpocalypse` (`4438060`)
- This app exists in Steamworks per user-provided screenshots on 2026-04-07.
- It can be used as a disposable QA app if absolutely needed.
- It is **not** the recommended main path for solving the real multiplayer issue, because it adds another AppID to an already mixed setup.

### 3.3 Spacewar / legacy compatibility

- **Legacy test AppID:** `480`
- This is no longer the checked-in runtime default.
- The current checked-in runtime defaults are:
  - `Config/DefaultEngine.ini`
    - `SteamDevAppId=4464300`
    - `SteamAppId=4464300`
  - root `steam_appid.txt` is `4464300`
- Old staged logs from 2026-04-07 still show earlier packaged builds running as:
  - `STEAM: [AppId: 480] Client API initialized 1`
  - `STEAM: [AppId: 480] Game Server API initialized 1`

## 4. Current Observed Steam State

### 4.1 What is already implemented

- `OnlineSubsystemSteam` is enabled in `T66.uproject`.
- `UT66SteamHelper` wraps Steam identity, friends, avatars, and Web API tickets.
- `UT66SessionSubsystem` implements:
  - session creation
  - session join
  - invite acceptance delegate handling
  - friend session lookup
  - rich presence updates
  - `ClientTravel` on resolved join connect string
- The local Steam helper also bridges Steam rich presence join callbacks.

### 4.2 What is confirmed working locally

- Steam SDK loads successfully in recent staged runs.
- Older staged builds initialized successfully under `480`.
- Web API tickets are being requested and obtained successfully in recent staged runs.
- Real Steam identity and friends collection succeeded in earlier logs when Steam was initialized normally.
- The checked-in config now targets `4464300`, but that path still needs a fresh validation run.

### 4.3 What is not yet confirmed

- No captured local log in this repo currently proves a successful full two-player join under the real AppID.
- No captured local log currently proves the exact join failure point after invite acceptance in the newest setup.
- No local evidence yet shows a run launched under `4464300`.

## 5. Current Observed Backend State

### 5.1 Deployment and hosting

- **Backend repo:** `C:\UE\Backend`
- **Framework:** Next.js 15
- **Hosting:** Vercel
- **Production URL:** `https://t66-backend.vercel.app`
- **Database:** Vercel Postgres / Neon
- **KV / cache / rate limit:** Upstash Redis / Vercel KV
- **Steam auth:** Steam Web API ticket validation

### 5.2 Live health check

Live check performed on 2026-04-20:

```json
{
  "status": "ok",
  "timestamp": "2026-04-20T07:22:23.361Z",
  "postgres": "ok",
  "postgres_counts": {
    "player_profiles": 1,
    "leaderboard_entries": 1,
    "run_summaries": 4,
    "run_reports": 0,
    "bug_reports": 405,
    "account_restrictions": 0
  },
  "kv": "ok"
}
```

This means the earlier checklist note about production KV being unhealthy is stale, and the production alias was confirmed healthy again after the later 2026-04-19 leaderboard hotfix redeploy.

### 5.3 Backend auth behavior

- `src/lib/steam.ts` validates tickets against `env.STEAM_APP_ID` by default.
- It can also allow additional AppIDs on a per-route basis through `allowedAppIds`.
- This matters because the invite routes currently still allow `480`.

### 5.4 Current route reality

These backend routes still explicitly authenticate against `allowedAppIds: ["480"]`:

- `C:\UE\Backend\src\app\api\party-invite\send\route.ts`
- `C:\UE\Backend\src\app\api\party-invite\pending\route.ts`
- `C:\UE\Backend\src\app\api\party-invite\respond\route.ts`
- `C:\UE\Backend\src\app\api\client-diagnostics\route.ts`

Because `authenticateRequest()` also appends `env.STEAM_APP_ID`, these routes currently accept both:

- legacy `480`
- real app `4464300` (assuming the environment stays set to `4464300`)

So the backend is currently in deliberate dual-AppID transition mode.

### 5.5 Leaderboard moderation / proof retention reality

- `/api/submit-run` now hard-blocks any active `account_restrictions` row:
  - `certainty` returns `status: "banned"`
  - `suspicion` returns `status: "suspended"`
- Automatic anti-cheat suspension records now carry explicit `flag_category` values for:
  - `luck_rating`
  - `skill_rating`
- Suspicion-level restrictions remain appealable through `/api/submit-appeal`, and the reviewed run summary ID returned by `/api/account-status` is now consumed by the UE Account Status screen.
- Proof-of-run URLs are now stored on the active PB run summary for every live leaderboard entry, not just the previous Top-N summary buffer.
- When a player improves the PB attached to an entry, the stored run summary is replaced with the new PB snapshot. Existing proof is therefore replaced by the new run's proof state.
- Production `run_summaries` now has the anti-cheat and integrity columns expected by the deployed backend:
  - `anti_cheat_context`
  - `anti_cheat_verdict`
  - `anti_cheat_flag_category`
  - `anti_cheat_reason`
  - `anti_cheat_supporting_data`
  - `anti_cheat_evaluated_at`
  - `integrity_context`
  - `integrity_verdict`
  - `integrity_reason_code`
  - `integrity_reason`
  - `integrity_evaluated_at`
- The production alias `https://t66-backend.vercel.app` was redeployed on 2026-04-19 after aligning `src/lib/leaderboard-keys.ts` with the real game difficulty ranges so stage-5 clears are treated as completed Easy runs on the backend too.
- The production alias was redeployed again on 2026-04-19 with deployment `dpl_5t8U9kvwstdxDnUQYMX8QHz2DwC9` after:
  - moving duplicate submission handling out of the anti-cheat restriction path
  - keying duplicate detection by a run fingerprint instead of only `steam_id + difficulty`
  - returning a JSON `unranked` verdict for safe duplicate suppression instead of creating a suspicion-level account restriction
  - adding explicit server-side `console.error` logging plus JSON `500` responses for unexpected `/api/submit-run` exceptions
- The production alias was redeployed again on 2026-04-19 with deployment `dpl_H6ejEz35AyLCWECG32LZ2CT1RsAG` after:
  - adding `submission_id` support to `/api/submit-run`
  - caching the terminal response per `steam_id + submission_id` so legitimate retries are idempotent
  - keeping duplicate fingerprint suppression only as a fallback for submissions that do not provide a `submission_id`
  - matching the UE client change that now sends a single completed-run leaderboard request for stage clears
- The production alias was redeployed again on 2026-04-19 with deployment `dpl_DW6wcQ7DqHhVQpPo2rAwjX5zzocw` after:
  - switching `leaderboard_entries` from PB-only upserts to append-only accepted-run inserts
  - making `/api/my-rank` resolve the player's single best row separately from the Top 10 list
  - keeping run summaries for every accepted leaderboard row instead of only the active PB row
  - aligning the backend with the UE weekly-rank display and difficulty-clear quit-button flow
- On 2026-04-19 after the 4-stage progression redesign landed in source, repeated production deploy attempts from `C:\UE\Backend` initially failed before a deployment ID was created:
  - `npx vercel deploy --prod --yes` failed twice with `read ECONNRESET`
  - a later retry failed once with `Invalid JSON response`
  - `npx vercel build --prod --yes` also exposed a local Vercel packaging issue (`Unable to find lambda for route: /admin/login`) even though `npm run build` succeeded
- A later retry on 2026-04-20 succeeded and produced deployment `dpl_DyRiTW2v9o8AZQuhpVQkr5AHKCfL`, which is now the production alias target.
- Keep the client compatibility shim in `Source/T66/Core/T66BackendSubsystem.cpp` for now; it is safe with the redeployed backend and avoids introducing another client/backend mismatch before the next validation pass.
- A false-positive production `account_restrictions` row for SteamID `76561198749633075` with `flag_category = "rate_limit"` and reason `Duplicate submission within 30s window` was cleared live on 2026-04-19 along with its matching `quarantined_runs` row.
- A false-positive production `account_restrictions` row for SteamID `76561198749633075` with `flag_category = "luck_replay"` was also cleared live on 2026-04-19 after the UE replay-metadata fix landed; the matching quarantine row was marked `resolved_false_positive`.
- A false-positive production `account_restrictions` row for SteamID `76561198749633075` with `flag_category = "dodge_telemetry"` and reason `Submitted hit-check event telemetry does not match aggregate dodge counters` was cleared live on 2026-04-20 after fixing the UE-side invulnerability hit-check streak reset. The matching quarantine row `376774c3-a53c-402b-ad3a-fc8fbb8b684d` was marked `resolved_false_positive`.
- A false-positive production `account_restrictions` row for SteamID `76561199841207421` with `flag_category = "luck_replay"` and reason `Submitted luck telemetry does not reproduce the claimed replayable RNG outcomes` was cleared live on 2026-04-20 after fixing the UE-side `GoblinCountPerWave` replay range and redeploying `/api/run-summary/[id]` to support owner-authenticated restricted run-summary fetches by `run_summaries.id`. Matching pending `quarantined_runs` rows for that SteamID were marked `false_positive`.
- Six accidental zero-score leaderboard rows for SteamID `76561199841207421` were deleted live on 2026-04-20 after tracing them to a client run-summary history-open bug that re-entered the live submit path. The backend now rejects zero-score solo submissions instead of accepting them into `leaderboard_entries`.
- The production database no longer carries the old `uq_lb_key_steam` unique index. That index was dropped live on 2026-04-19 to allow multiple accepted leaderboard rows per player per board.
- Weekly cleanup now deletes:
  - all weekly leaderboard entries
  - all weekly run summaries
  - all run reports
- `/api/report-run` still writes moderation reports immediately to Postgres, but those reports are now a short-lived weekly queue rather than a 30-day archive.
- Admin review surfaces still exist at:
  - `/admin/appeals`
  - `/admin/reports`
  - `/admin/accounts`
- There is still no separate email / Discord / webhook notification layer in this repo. Moderation intake is currently the admin portal plus the database-backed queues above.

## 6. Current Contradictions To Resolve

### 6.1 Docs vs old staged builds

The local docs say the real AppID path was already aligned to `4464300`.

That change is now reflected in the checked-in UE runtime config, but the old staged logs still show earlier packaged runs under `480`.

So the remaining work is no longer "change the config"; it is "validate the new config path end to end under a fresh real-AppID run."

### 6.2 Docs vs live backend health

Older docs say production KV was still failing.

The live backend health check on 2026-04-07 shows:

- `postgres: ok`
- `kv: ok`

So backend health should now be treated as healthy until proven otherwise.

### 6.3 Real backend AppID vs invite compatibility route

- General backend docs/checklists point to `4464300`.
- Invite routes still allow `480`.
- The game currently sends `host_app_id` using whatever AppID Steam thinks is active.

Right now that means the current invite stack is still effectively running in Spacewar mode end-to-end.

## 7. Internet-Connected Systems Inventory

| System | Role | Current State |
|---|---|---|
| Steam client | Runtime ownership, friends, overlay, invite UX | Installed locally and working |
| Steamworks SDK 1.61 | UE Steam integration | Loaded in local logs |
| Steam Web API | Ticket auth + profile fetch | In use by backend |
| Steam avatars CDN | Player avatar images | In use by backend / UE image cache |
| Steam App `4464300` | Intended real shipping app | Present in Steamworks; now the checked-in UE runtime default |
| Steam App `4438060` | Secondary app | Present in Steamworks; optional fallback test app only |
| SteamPipe / ContentBuilder | Build upload | Installed locally and already has script for `4464300` |
| Client diagnostics route | Structured host/guest multiplayer traces | Implemented at `/api/client-diagnostics` |
| Vercel | Hosts backend | Live |
| Vercel Postgres / Neon | Persistent game data | Live and healthy |
| Upstash Redis / Vercel KV | Cache / rate limits / invite temp data | Live and healthy |

## 8. Recommended Multiplayer Testing Path

### 8.1 Recommended target

Use:

- **Base app:** `4464300`
- **Release state:** unreleased
- **Distribution method:** Release Override keys
- **Build delivery:** SteamPipe upload through ContentBuilder
- **Build targeting:** optional private beta branch

### 8.2 Why this is the right path

- It keeps multiplayer testing on the real final AppID.
- Your friend gets Steam-native install and update flow.
- You avoid public release.
- You avoid continuing to depend on Spacewar `480`.
- You avoid adding a third active runtime AppID to debug.

### 8.3 What not to do

- Do not keep using `480` for real invite validation.
- Do not switch to `4438060` unless you intentionally want a disposable QA product and accept that final multiplayer validation still has to be repeated on `4464300`.

## 9. End-To-End Setup For Real-AppID Private Multiplayer

### Phase A. Steamworks manual setup

These actions must be done in Steamworks App Admin by the user:

1. Confirm `4464300` is the target app for the private multiplayer test.
2. Confirm the app has the required Windows depot/package wiring.
3. Create a private test branch if branch isolation is desired.
4. Request a small Release Override key batch for `4464300`.
5. Add Steam rich presence localization entries required by the game:
   - `#Status_MainMenu`
   - `#Status_InLobby`
6. Redeem a key on the tester account.

### Phase B. Repo and backend work Codex can do

These are the code/config tasks needed to finish the migration from `480` to `4464300`:

1. Remove or gate any remaining `480` assumptions in client code and docs.
2. Decide when to end temporary dual-AppID backend acceptance during migration.
3. Add clearer invite/join diagnostic logging around:
   - invite accepted
   - friend session lookup
   - lobby ID matching
   - join result
   - resolved connect string
4. Verify the staged build actually launches under `4464300`.
5. Review `/api/client-diagnostics` and local `Saved/Diagnostics/Multiplayer/` output if a join fails.

### Phase C. Build/upload automation Codex can do

After the one-time Steamworks setup is done, Codex can automate most of the repeatable build workflow:

1. Package the build.
2. Sync staged build output into the Steam ContentBuilder content root.
3. Run `steamcmd` with the existing app build script for `4464300`.
4. Upload the build through SteamPipe.
5. Optionally set it live on a beta branch.
6. Use `Tools/Steam/UploadToSteam.ps1` as the local wrapper for staging and upload.

Important limitation:

- Steam uploads can be automated.
- Changing the **default branch** live build still requires manual action in App Admin.
- Requesting keys and editing packages/branches still requires Steamworks dashboard access.

### Phase D. Verification gates

The migration to the real AppID is only complete when all of these are true:

1. Both test machines launch through Steam as `4464300`, not `480`.
2. The host can create a party session/lobby.
3. The friend receives the invite.
4. Accepting the invite reaches the host's party session.
5. Both players enter the same game world.
6. The backend accepts auth for the real AppID on invite-related routes.
7. No current log or config still identifies the active runtime app as `480`.

## 10. Work Split

### 10.1 Codex can do

- move and reorganize docs
- write and maintain this master backend handoff
- update UE config and code
- update backend invite auth/config
- add richer multiplayer diagnostics
- prepare and run SteamPipe uploads from this PC
- build a repeatable upload script/workflow

### 10.2 User must do manually

- Steamworks App Admin actions
- branch creation and package/license management
- Release Override key requests
- tester account key redemption
- any public release/store page action
- any Steam Guard / account verification required for first upload login

## 11. Open Questions

1. Do we want temporary dual-AppID compatibility during migration, or a hard cut from `480` to `4464300`?
2. Do we want to keep `4438060` completely out of scope, or reserve it as a disposable emergency QA app?
3. Do we want to upload the first real test build to the default unreleased branch or to a named private beta branch?

## 12. Immediate Next Recommended Order

1. Finish migrating client and invite backend auth from `480` to `4464300`.
2. Verify a local staged build logs as `4464300`.
3. Confirm Steamworks branch/package/key setup for `4464300`.
4. Upload the first real private build through SteamPipe.
5. Redeem the Release Override key on the tester account.
6. Run the first true two-player invite/join/world-entry test under the real AppID.
