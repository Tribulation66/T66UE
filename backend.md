# T66 Backend Architecture — Leaderboard, Integrity & Moderation

**Version:** 2.0  
**Last updated:** 2026-02-17  
**Status:** Phases 1–15 implemented and deployed. Avatars, friends, streamers wired. Dev-mode testing works end-to-end.

This document is the complete implementation spec + operational guide for the T66 online backend. It covers: service stack, database schema, API contracts, data flows, security, anti-cheat, admin portal, phasing plan, file locations, env vars, and continuation guide.

**Companion files:**
- `guidelines.md` — engineering rules and workflows
- `memory.md` — agent context and current state
- `T66_Bible.md` — gameplay/UI design source of truth

---

## 1. Philosophy

- **Steam = Social Layer.** Steam handles identity, friends, invites, avatars, cloud saves, and authentication. Steam does NOT store leaderboard scores.
- **Postgres = Competitive + Moderation Layer.** All leaderboard scores, rankings, run summaries, anti-cheat data, reports, appeals, and moderation live in a single Vercel Postgres database.
- **Single source of truth.** The backend database is the only authority for competitive rankings. No dual-write, no desync, no audit reconciliation needed.
- **Built for 200k concurrent players.** Edge caching for reads, serverless functions for writes, autoscaling database.
- **No Steam leaderboards.** Eliminates Steam Web API rate limit bottleneck (~19M calls/day at 200k concurrent). The backend validates and stores everything. Steam is only called for session ticket authentication (~900k calls/day with caching) and profile resolution (name + avatar).
- **"Score" not "Bounty" or "HighScore".** All code, UI, and docs use "Score" consistently. "Bounty" and "HighScore" are deprecated terms.

---

## 2. Service Stack

| Service | Role | Tier | Est. Cost |
|---|---|---|---|
| **Steam** | Identity, friends, invites, avatars, cloud saves, session ticket auth | Publisher (free) | $0 |
| **Vercel** | Hosts Next.js backend (API routes + admin portal) | Pro ($20/mo) | $20/mo + usage |
| **Vercel Postgres (Neon)** | All leaderboard, run summary, and moderation data | Pro (usage-based) | ~$30-60/mo |
| **Vercel KV (Redis)** | Session ticket validation cache | Pro (usage-based) | ~$10-15/mo |
| **Total at 200k concurrent** | | | **~$60-95/mo** |

---

## 3. Repositories & File Locations

### 3.1 Backend Repository

**Repo:** `https://github.com/Tribulation66/t66-backend` (branch: `main`)  
**Local path:** `C:\UE\Backend`  
**Deployed at:** `https://t66-backend.vercel.app`

| Path | Purpose |
|---|---|
| `src/db/schema.ts` | Drizzle ORM schema (all 9 tables) |
| `src/lib/steam.ts` | Steam ticket validation + dev-mode bypass + KV caching |
| `src/lib/steam-profile.ts` | Fetch Steam display name + avatar via `GetPlayerSummaries/v2` |
| `src/lib/schemas.ts` | Zod validation schemas for all API inputs |
| `src/lib/leaderboard-keys.ts` | Deterministic key builder + sort direction helper |
| `src/lib/anti-cheat.ts` | Phase 1 anti-cheat checks |
| `src/lib/env.ts` | Typed env var access |
| `src/lib/kv.ts` | Upstash Redis (Vercel KV) client |
| `src/app/api/submit-run/route.ts` | `POST /api/submit-run` — full submission pipeline |
| `src/app/api/leaderboard/route.ts` | `GET /api/leaderboard` — public Top 10 |
| `src/app/api/leaderboard/friends/route.ts` | `GET /api/leaderboard/friends` — auth'd friends board |
| `src/app/api/my-rank/route.ts` | `GET /api/my-rank` — player's global rank |
| `src/app/api/run-summary/[id]/route.ts` | `GET /api/run-summary/:id` — full run detail |
| `src/app/api/report-run/route.ts` | `POST /api/report-run` |
| `src/app/api/account-status/route.ts` | `GET /api/account-status` |
| `src/app/api/submit-appeal/route.ts` | `POST /api/submit-appeal` |
| `src/app/api/proof-of-run/route.ts` | `PUT /api/proof-of-run` |
| `src/app/api/bug-report/route.ts` | `POST /api/bug-report` |
| `src/app/api/cron/weekly-reset/route.ts` | Cron: Monday 00:00 UTC weekly reset |
| `src/app/api/cron/data-retention/route.ts` | Cron: daily 03:00 UTC cleanup |
| `src/app/api/admin/*/route.ts` | Admin API routes (login, logout, action, account, streamers) |
| `src/app/admin/*` | Admin portal pages (Next.js App Router) |
| `scripts/seed.mjs` | Database seeder (dummy data + real Steam profile) |
| `drizzle.config.ts` | Drizzle Kit config (for migrations) |
| `vercel.json` | Vercel config (cron schedules) |
| `.env.local` | Local env vars (pulled via `npx vercel env pull`) |

### 3.2 UE5 Game Repository

**Repo:** `https://github.com/Tribulation66/T66UE` (branch: `version-1.1`)  
**Local path:** `C:\UE\T66`

| Path | Purpose |
|---|---|
| `Source/T66/Core/T66BackendSubsystem.h/.cpp` | HTTP client: all backend API calls, response parsing, caching |
| `Source/T66/Core/T66SteamHelper.h/.cpp` | Steamworks integration: tickets, friends list, dev-mode bypass |
| `Source/T66/Core/T66WebImageCache.h/.cpp` | Async web image downloader (Steam avatars → UTexture2D) |
| `Source/T66/Core/T66LeaderboardSubsystem.h/.cpp` | Leaderboard logic: score submission, local save, build entries |
| `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h/.cpp` | Run summary data structure (UObject, serializable) |
| `Source/T66/Data/T66DataTypes.h` | `FLeaderboardEntry` struct (Rank, PlayerName, Score, AvatarUrl, EntryId, etc.) |
| `Source/T66/UI/Components/T66LeaderboardPanel.h/.cpp` | Leaderboard Slate widget: rows, filters, avatar images, click-to-summary |
| `Source/T66/T66.Build.cs` | Module dependencies (HTTP, Json, Steamworks, ImageWrapper, etc.) |
| `Config/DefaultGame.ini` | `[T66.Online]` section: BackendBaseUrl, DevTicket, DevFriendIds |
| `Config/DefaultEngine.ini` | OnlineSubsystem config (Steam + NULL) |
| `T66.uproject` | Plugin config (OnlineSubsystemSteam enabled) |

### 3.3 Config: `DefaultGame.ini` — `[T66.Online]` section

```ini
[T66.Online]
; Backend URL for online leaderboards and moderation.
; MUST be quoted to prevent UE INI parser from truncating at //
BackendBaseUrl="https://t66-backend.vercel.app"

; Dev ticket for local testing without Steam running.
; Must match a ticket in backend's DEV_STEAM_IDS env var.
; Leave empty when shipping with real Steam.
DevTicket=dev_player_1

; Dev friend IDs (comma-separated) for testing Friends tab without Steam.
DevFriendIds=76561198369499700
```

---

## 4. Environment Variables

### 4.1 Vercel (Production)

Set in Vercel dashboard → Settings → Environment Variables.

| Variable | Source | Purpose |
|---|---|---|
| `STEAM_WEB_API_KEY` | Steamworks partner portal | `AuthenticateUserTicket` + `GetPlayerSummaries` calls |
| `STEAM_APP_ID` | Steamworks partner portal | Identifies the game (Chadpocalipse) |
| `POSTGRES_URL` | Auto-set by Vercel Postgres (Neon) | Database connection (pooled) |
| `KV_REST_API_URL` | Auto-set by Vercel KV (Upstash Redis) | Redis endpoint |
| `KV_REST_API_TOKEN` | Auto-set by Vercel KV | Redis auth token |
| `ADMIN_PASSWORD` | You choose | Admin portal login password |
| `CRON_SECRET` | Auto-generated by Vercel | Cron job authentication |
| `DEV_STEAM_IDS` | You set | Dev-mode auth bypass: `dev_player_1:76561100000000001` |

### 4.2 Local Development (`.env.local`)

Pull from Vercel: `npx vercel env pull` (run in `C:\UE\Backend`).

This creates `.env.local` with all the production variables. The seed script and local dev server both read from it.

**Important:** `.env.local` is in `.gitignore` and must never be committed.

---

## 5. Leaderboard Key Convention

**Format:**
```
{type}_{time}_{party}_{difficulty}
```

| Component | Values |
|---|---|
| `type` | `score`, `speedrun` |
| `time` | `weekly`, `alltime` |
| `party` | `solo`, `duo`, `trio` |
| `difficulty` | `easy`, `medium`, `hard`, `veryhard`, `impossible`, `perdition`, `final` |

**Examples:**
- `score_alltime_solo_hard`
- `score_weekly_duo_impossible`
- `speedrun_alltime_trio_final`
- `speedrun_weekly_solo_easy`

**Total: 84 keys** (2 types × 2 times × 3 parties × 7 difficulties).

**Sort direction:**
- **Score:** higher is better → sort `DESC`.
- **SpeedRun:** stored as milliseconds, lower is better → sort `ASC`.

**SpeedRun keys are only generated** when the player completes all stages of a difficulty (10 stages for Easy–Perdition, 6 for Final). Incomplete runs → N/A for SpeedRun.

**SpeedRun split times:** Top 15 entries store `stage_splits_ms` (JSONB array of cumulative ms per stage) for pacing display.

---

## 6. Database Schema

### 6.1 `player_profiles`

Cached Steam identity. Updated on every submission via Steam `GetPlayerSummaries` API (real name + avatar).

```sql
CREATE TABLE player_profiles (
  steam_id        TEXT PRIMARY KEY,
  display_name    TEXT NOT NULL,
  avatar_url      TEXT,               -- Steam CDN avatar (184×184)
  hero_id         TEXT,               -- last-used hero (for display)
  updated_at      TIMESTAMPTZ DEFAULT NOW()
);
```

### 6.2 `leaderboard_entries`

One row per player per leaderboard key. Upserted on submission (keep personal best).

```sql
CREATE TABLE leaderboard_entries (
  id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  steam_id        TEXT NOT NULL REFERENCES player_profiles(steam_id),
  leaderboard_key TEXT NOT NULL,
  score           BIGINT NOT NULL,    -- score value or time in ms
  hero_id         TEXT NOT NULL,
  companion_id    TEXT,
  stage_reached   INT NOT NULL,
  party_size      TEXT NOT NULL,      -- 'solo', 'duo', 'trio'
  difficulty      TEXT NOT NULL,
  time_scope      TEXT NOT NULL,      -- 'weekly', 'alltime'
  run_id          UUID,               -- shared across co-op party members
  submitted_at    TIMESTAMPTZ DEFAULT NOW(),
  UNIQUE(leaderboard_key, steam_id)
);
```

### 6.3 `run_summaries`

Full run data for Top 15 entries + quarantined runs.

```sql
CREATE TABLE run_summaries (
  id                UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  entry_id          UUID REFERENCES leaderboard_entries(id) ON DELETE SET NULL,
  steam_id          TEXT NOT NULL,
  leaderboard_key   TEXT NOT NULL,
  run_id            UUID,
  party_slot        INT DEFAULT 0,
  schema_version    INT DEFAULT 6,
  hero_id           TEXT,
  companion_id      TEXT,
  stage_reached     INT,
  score             INT,
  time_seconds      FLOAT,
  stage_splits_ms   JSONB,            -- cumulative ms per stage (speedrun)
  hero_level        INT,
  stats             JSONB,            -- {damage, attack_speed, ...}
  secondary_stats   JSONB,
  luck_rating       INT,
  luck_quantity     INT,
  luck_quality      INT,
  skill_rating      INT,
  equipped_idols    JSONB,
  inventory         JSONB,
  event_log         JSONB,
  damage_by_source  JSONB,
  proof_of_run_url  TEXT,
  proof_locked      BOOLEAN DEFAULT FALSE,
  display_name      TEXT,
  created_at        TIMESTAMPTZ DEFAULT NOW(),
  expires_at        TIMESTAMPTZ       -- NULL = no expiry (active top entry)
);
```

### 6.4–6.9 Other tables

| Table | Purpose |
|---|---|
| `streamer_whitelist` | SteamIDs for the Streamers tab |
| `account_restrictions` | Suspicion/Certainty records + appeal state |
| `run_reports` | Player "This Run is Cheating" reports |
| `quarantined_runs` | Auto-flagged runs from anti-cheat |
| `bug_reports` | Player-submitted bug reports |
| `audit_log` | Every admin moderation action |

Full SQL for these tables is in `src/db/schema.ts`.

---

## 7. API Contracts

### Base URL

Development: `http://localhost:3000`  
Production: `https://t66-backend.vercel.app`

### 7.1 `POST /api/submit-run`

Submit a run at death or difficulty completion. Auth: Steam ticket.

The backend resolves the player's real Steam name + avatar via `GetPlayerSummaries/v2` and stores them in `player_profiles` (does not trust client-provided name).

**Request:** `X-Steam-Ticket` header + JSON body with `display_name`, `run` object (score, time_ms, hero_id, companion_id, difficulty, party_size, stage_reached, stats, secondary_stats, ratings, equipped_idols, inventory, event_log, damage_by_source, stage_splits_ms).

**Response (success):**
```json
{
  "status": "accepted",
  "score_rank_alltime": 7,
  "score_rank_weekly": 3,
  "speedrun_rank_alltime": null,
  "speedrun_rank_weekly": null,
  "is_new_personal_best_score": true,
  "is_new_personal_best_speedrun": false
}
```

### 7.2 `GET /api/leaderboard`

Public Top 10. Params: `type`, `time`, `party`, `difficulty`, `filter` (global/streamers).

Response includes `entry_id`, `display_name`, `avatar_url`, `hero_id`, `score`, `stage_reached`, `has_run_summary`, etc.

### 7.3 `GET /api/leaderboard/friends`

Auth'd. Params: same + `friend_ids` (comma-separated). Returns entries for friend set + self.

### 7.4 `GET /api/my-rank`

Auth'd. Returns player's rank + score on a specific board. Used for the 11th row.

### 7.5 `GET /api/run-summary/:id`

Public. Returns full run summary for a Top 15 entry. Optional `?slot=N` for co-op.

### 7.6–7.10 Other endpoints

| Endpoint | Auth | Purpose |
|---|---|---|
| `POST /api/report-run` | Steam ticket | Report a cheating run |
| `GET /api/account-status` | Steam ticket | Check own account restrictions |
| `POST /api/submit-appeal` | Steam ticket | Appeal a suspicion flag |
| `PUT /api/proof-of-run` | Steam ticket | Set proof URL on own entry |
| `POST /api/bug-report` | Steam ticket | Submit a bug report |

---

## 8. UE5 Client Integration

### 8.1 Subsystem Architecture

| Subsystem | Role |
|---|---|
| `UT66BackendSubsystem` | HTTP client for all backend calls. Manages auth headers, response parsing, caching. |
| `UT66SteamHelper` | Steamworks SDK wrapper: session tickets, friends list. Dev-mode bypass reads from `DefaultGame.ini`. |
| `UT66WebImageCache` | Downloads web images (Steam avatars), decodes via IImageWrapper, caches as UTexture2D. |
| `UT66LeaderboardSubsystem` | Game logic: builds run snapshots, calls BackendSubsystem to submit, manages local save data. |

### 8.2 Data Flow: Leaderboard Display

1. `ST66LeaderboardPanel::RefreshLeaderboard()` checks `BackendSubsystem` cache
2. If no cache hit → calls `BackendSubsystem::FetchLeaderboard(type, time, party, difficulty, filter)`
3. For `filter == "friends"`: routes to `/api/leaderboard/friends` with auth + friend IDs from `SteamHelper`
4. For `filter == "global"` or `"streamers"`: routes to `/api/leaderboard` (public, no auth)
5. Response parsed into `TArray<FLeaderboardEntry>` including `AvatarUrl`
6. `OnLeaderboardDataReady` delegate fires → panel calls `RebuildEntryList()`
7. Each row shows: Rank | Avatar (32×32, async-downloaded) | Player Name | Score/Time
8. Avatar downloaded via `UT66WebImageCache::RequestImage()` → decoded → `SImage` brush updated

### 8.3 Data Flow: Run Submission

1. Player dies or completes difficulty → `UT66LeaderboardSubsystem::SubmitRunScore()`
2. Builds `UT66LeaderboardRunSummarySaveGame` snapshot (hero, stats, items, idols, damage, ratings)
3. Calls `BackendSubsystem::SubmitRunToBackend()` with snapshot data
4. Backend validates, stores, returns ranks
5. Client updates local state

### 8.4 Dev-Mode Testing (No Steam Required)

The game works locally without Steam running:

1. `DefaultGame.ini` has `DevTicket=dev_player_1` and `DevFriendIds=76561198369499700`
2. `T66SteamHelper` reads these on startup (when Steam API is unavailable)
3. Sets `TicketHex = "dev_player_1"`, `LocalSteamIdStr = "76561100000000001"`
4. Populates `FriendSteamIds` from `DevFriendIds`
5. `BackendSubsystem` sends `X-Steam-Ticket: dev_player_1` in requests
6. Backend's `DEV_STEAM_IDS=dev_player_1:76561100000000001` env var maps this to a valid SteamID
7. All API calls work without real Steam authentication

### 8.5 Key FLeaderboardEntry Fields

```cpp
struct FLeaderboardEntry {
    int32 Rank;
    FString PlayerName;
    TArray<FString> PlayerNames;  // co-op: multiple names
    int64 Score;
    float TimeSeconds;
    FName HeroID;
    ET66PartySize PartySize;
    ET66Difficulty Difficulty;
    int32 StageReached;
    bool bIsLocalPlayer;
    FString EntryId;              // backend UUID for run summary fetch
    bool bHasRunSummary;
    FString AvatarUrl;            // Steam CDN avatar URL
};
```

---

## 9. Anti-Cheat Validation

### 9.1 Phase 1 — Basic Checks (IMPLEMENTED)

| Check | Logic | Outcome |
|---|---|---|
| **Score hard cap** | Score cannot exceed theoretical max for difficulty | Certainty or Suspicion |
| **Time plausibility** | Time cannot be below theoretical minimum | Certainty |
| **Luck Rating > 100** | Flag for review | Suspicion |
| **Skill Rating > 100** | Flag for review | Suspicion |
| **Inventory size** | Max items for the stage | Certainty |
| **Idol count** | Max 6 | Certainty |
| **Duplicate submission** | Same SteamID within 30s (KV rate limit) | Rate limit |

### 9.2 Phase 2 — Deep Checks (DEFERRED, requires structured event log)

Score-kill consistency, gold cap, item provenance, wheel spin count, dodge validation, etc. See `src/lib/anti-cheat.ts` for Phase 1 implementation.

---

## 10. Cron Jobs

| Job | Schedule | Endpoint |
|---|---|---|
| **Weekly reset** | Monday 00:00 UTC | `GET /api/cron/weekly-reset` |
| **Data retention** | Daily 03:00 UTC | `GET /api/cron/data-retention` |

Configured in `vercel.json`.

---

## 11. Admin Portal

Available at `https://t66-backend.vercel.app/admin`. Password-protected (env var `ADMIN_PASSWORD`).

| Page | Purpose |
|---|---|
| `/admin/dashboard` | Overview: pending appeals, recent reports, quarantines |
| `/admin/appeals` | Appeals queue with run summary viewer |
| `/admin/reports` | Reported runs queue |
| `/admin/quarantine` | Auto-flagged runs |
| `/admin/accounts` | Search by SteamID, view history, suspend/ban/unsuspend |
| `/admin/streamers` | Manage streamer whitelist |
| `/admin/audit` | Audit log of all admin actions |

---

## 12. Seed Data

The seed script (`scripts/seed.mjs`) populates the database with test data:

- **840 fake global entries** (10 per key × 84 keys), each with unique fake Steam IDs
- **84 real profile entries** (1 per key for Steam ID `76561198369499700`)
- Display names: `G_Weekly_Solo_Easy_Score_1` through `G_AllTime_Trio_Final_SpeedRun_10`
- Real Steam profile name + avatar fetched live from Steam API
- Streamer whitelist entry for the real profile
- Full run summaries with realistic items, idols, stats, damage sources

**To re-seed:**
```bash
cd C:\UE\Backend
npx dotenv-cli -e .env.local -- node scripts/seed.mjs
```

**To push schema changes:**
```bash
cd C:\UE\Backend
npx dotenv-cli -e .env.local -- npx drizzle-kit push
```

---

## 13. Implementation Phases

| Phase | What | Status |
|---|---|---|
| **1** | Vercel project setup (Next.js + Postgres + Drizzle schema + KV) | **DONE** |
| **2** | Steam integration in UE5 (session tickets, identity, friends list via Steamworks) | **DONE** |
| **3** | `POST /api/submit-run` — validate ticket, basic anti-cheat, store entry + run summary | **DONE** |
| **4** | `GET /api/leaderboard` — serve Top 10 from Postgres with edge caching | **DONE** |
| **5** | `GET /api/run-summary/:id` — serve run summaries for Global + Streamers | **DONE** |
| **6** | UE5 client: replace placeholder leaderboard with real HTTP calls (FHttpModule) | **DONE** |
| **7** | `GET /api/leaderboard/friends` + friends list wiring | **DONE** |
| **8** | `GET /api/my-rank` + 11th row backend endpoint | **DONE** (endpoint exists, UE5 client not yet calling it) |
| **9** | Streamers whitelist + `/admin/streamers` + filter | **DONE** |
| **10** | Co-op submission (backend multi-member) | **DONE** (backend; UE5 client sends solo only) |
| **11** | Report/appeal/account-status endpoints | **DONE** |
| **12** | Admin portal (dashboard, appeals, reports, quarantine, accounts, audit) | **DONE** |
| **13** | Cron jobs (weekly reset, data retention) | **DONE** |
| **14** | `POST /api/bug-report` | **DONE** |
| **15** | `PUT /api/proof-of-run` | **DONE** |
| **16** | Steam Cloud save integration | DEFERRED |
| **17** | Structured event log redesign (client-side recording) | DEFERRED |
| **18** | Deep anti-cheat validation (server replays event log) | DEFERRED |
| **19** | Auto-quarantine pipeline | DEFERRED |

### What remains before launch:

| Item | Description | Difficulty |
|---|---|---|
| **11th row real rank** | UE5 client needs to call `GET /api/my-rank` and display real global rank in the 11th "YOU" row | Easy |
| **Co-op submission from UE5** | `SubmitRunToBackend` currently sends solo only; needs to send `co_op.party_members` for duo/trio | Medium |
| **Player Summary Picker UI** | Clicking a co-op leaderboard row should show a picker for party members' individual summaries | Medium |
| **Real Steam auth** | Swap dev tickets for real `ISteamUser::GetAuthSessionTicket()`. Code exists in `T66SteamHelper`, just needs Steam running. | Easy (just deploy) |
| **Real friends list** | `T66SteamHelper` already calls `ISteamFriends` when Steam is available. Just works once on Steam. | Easy (just deploy) |

---

## 14. Caching Strategy

| Endpoint | Cache | TTL |
|---|---|---|
| `GET /api/leaderboard` | Vercel Edge (CDN) | 60s `s-maxage` |
| `GET /api/leaderboard/friends` | Private | 60s |
| `GET /api/run-summary/:id` | Vercel Edge | 300s |
| `GET /api/my-rank` | None | — |
| `POST /*` | None | — |

**Session ticket cache (KV):** ticket hash → SteamID, 2hr TTL.

---

## 15. Security Model

| Layer | Mechanism |
|---|---|
| Client → Backend auth | Steam session ticket via `X-Steam-Ticket` header |
| Rate limiting (submissions) | 1 per 30s per SteamID (KV) |
| Rate limiting (reports) | 5/min, 20/day per SteamID |
| Admin portal auth | Password cookie (`ADMIN_PASSWORD` env var) |
| Steam Web API key | Server-side only |
| HTTPS | All communication (Vercel default) |
| Input validation | Zod schemas on all API inputs |
| SQL injection prevention | Drizzle ORM parameterized queries |

---

## 16. Quick Reference Commands

```bash
# ── Backend (C:\UE\Backend) ──

# Start local dev server
npm run dev

# Build
npm run build

# Push schema to database
npx dotenv-cli -e .env.local -- npx drizzle-kit push

# Open Drizzle Studio (visual DB browser)
npx dotenv-cli -e .env.local -- npx drizzle-kit studio

# Re-seed database
npx dotenv-cli -e .env.local -- node scripts/seed.mjs

# Pull env vars from Vercel
npx vercel env pull

# Deploy (auto-deploys on push to main)
git push

# ── UE5 (C:\UE\T66) ──

# Build
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "-Project=C:\UE\T66\T66.uproject" -WaitMutex
```

---

## 17. Test Steam Profile

For development, Steam profile `76561198369499700` is seeded as both a friend and a streamer:

- **Steam display name:** Fetched live from Steam API on each seed run
- **Avatar:** Full-size avatar URL stored in `player_profiles.avatar_url`
- **Has entries on all 84 leaderboard keys** with realistic mid-pack scores
- **In `streamer_whitelist`** → appears in Streamers tab
- **In `DevFriendIds`** config → appears in Friends tab during dev-mode testing

---

## 18. Discrepancies with Bible (T66_Bible.md)

| Bible says | This architecture does | Rationale |
|---|---|---|
| "Steam Leaderboards" as competitive layer (§1.3.1) | Postgres is the competitive layer. Steam is identity + social only. | Steam API rate limits make server-side writes infeasible at 200k concurrent. |
| Runs submitted to Steam (§1.3.2) | Runs validated server-side, stored in Postgres only. | Same trusted-write principle, but writing to Postgres. |
| Party sizes: Solo, Duo, Trio only (§1.2) | Same, Quads trivially addable later. | Future-proofed. |
| Friends tab implies Steam leaderboard read (§1.2) | Friends tab uses backend API with friend SteamIDs from client. | No Steam leaderboards exist. |
| References to "Bounty" / "HighScore" | All renamed to "Score" | User decision. Consistent naming across codebase. |
