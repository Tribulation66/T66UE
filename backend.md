# T66 Backend Architecture — Leaderboard, Integrity & Moderation

**Version:** 1.0  
**Last updated:** 2026-02-16  
**Status:** Architecture finalized, ready for implementation.

This document is the complete implementation spec for the T66 online backend. It defines the service stack, database schema, API contracts, data flows, security model, anti-cheat pipeline, admin portal, and phasing plan.

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
- **No Steam leaderboards.** Eliminates Steam Web API rate limit bottleneck (~19M calls/day at 200k concurrent). The backend validates and stores everything. Steam is only called for session ticket authentication (~900k calls/day with caching).

---

## 2. Service Stack

| Service | Role | Tier | Est. Cost |
|---|---|---|---|
| **Steam** | Identity, friends, invites, avatars, cloud saves, session ticket auth | Publisher (free) | $0 |
| **Vercel** | Hosts Next.js backend (API routes + admin portal) | Pro ($20/mo) | $20/mo + usage |
| **Vercel Postgres (Neon)** | All leaderboard, run summary, and moderation data | Pro (usage-based) | ~$30-60/mo |
| **Vercel KV (Redis)** | Session ticket validation cache | Pro (usage-based) | ~$10-15/mo |
| **Total at 200k concurrent** | | | **~$60-95/mo** |

### What is NOT in the stack (deferred)

| Service | When | Purpose |
|---|---|---|
| Sentry | Later | Crash reporting |
| Custom domain | Optional | `api.tribulation66.com` instead of `*.vercel.app` |
| Steam leaderboards | Not planned | Eliminated in favor of Postgres-only |

---

## 3. Architecture Overview

```
┌──────────────────────────────────────────────────────────────┐
│                      GAME CLIENT (UE5)                        │
│                                                               │
│  ┌─────────────────────┐        ┌──────────────────────────┐  │
│  │ Steamworks (local)  │        │ FHttpModule (HTTPS)      │  │
│  │                     │        │                          │  │
│  │ • GetAuthTicket     │        │ • POST /api/submit-run   │  │
│  │ • GetPersonaName    │        │ • GET  /api/leaderboard  │  │
│  │ • GetFriendAvatar   │        │ • GET  /api/my-rank      │  │
│  │ • GetFriendCount    │        │ • GET  /api/run-summary  │  │
│  │ • GetFriendByIndex  │        │ • POST /api/report-run   │  │
│  │ • Cloud Save/Load   │        │ • GET  /api/account-status│ │
│  │ • Lobby/Invite      │        │ • POST /api/submit-appeal│  │
│  │                     │        │ • POST /api/bug-report   │  │
│  └─────────────────────┘        └────────────┬─────────────┘  │
└──────────────────────────────────────────────┼────────────────┘
                                               │ HTTPS
                                               ▼
                          ┌──────────────────────────────────┐
                          │      VERCEL (Next.js App)         │
                          │                                   │
                          │  /api/*    → game client endpoints│
                          │  /admin/*  → developer portal     │
                          │                                   │
                          │  ┌─────────────────────────────┐  │
                          │  │ Steam Web API (minimal)     │  │
                          │  │ • AuthenticateUserTicket     │  │
                          │  │   (~900k/day with KV cache)  │  │
                          │  └─────────────────────────────┘  │
                          │                                   │
                          │  ┌─────────────────────────────┐  │
                          │  │ Vercel Postgres (Neon)       │  │
                          │  │                              │  │
                          │  │ • leaderboard_entries        │  │
                          │  │ • run_summaries              │  │
                          │  │ • player_profiles            │  │
                          │  │ • streamer_whitelist         │  │
                          │  │ • account_restrictions       │  │
                          │  │ • run_reports                │  │
                          │  │ • quarantined_runs           │  │
                          │  │ • bug_reports                │  │
                          │  │ • audit_log                  │  │
                          │  └─────────────────────────────┘  │
                          │                                   │
                          │  ┌─────────────────────────────┐  │
                          │  │ Vercel KV (Redis)            │  │
                          │  │ • ticket_hash → steam_id     │  │
                          │  │   (2hr TTL)                  │  │
                          │  └─────────────────────────────┘  │
                          │                                   │
                          │  Cron: weekly archive             │
                          │  Cron: daily retention cleanup    │
                          └──────────────────────────────────┘
```

---

## 4. Leaderboard Key Convention

All leaderboard entries are keyed by a deterministic string used in Postgres, API queries, and client code.

**Format:**
```
{type}_{time}_{party}_{difficulty}[_s{stage}]
```

**Components:**

| Component | Values |
|---|---|
| `type` | `bounty`, `speedrun` |
| `time` | `weekly`, `alltime` |
| `party` | `solo`, `duo`, `trio` |
| `difficulty` | `easy`, `medium`, `hard`, `veryhard`, `impossible`, `perdition`, `final` |
| `stage` | `s10`, `s20`, `s30`, `s40`, `s50`, `s60`, `s66` (speedrun only) |

**Examples:**
- `bounty_alltime_solo_hard`
- `bounty_weekly_duo_impossible`
- `speedrun_alltime_trio_final_s66`
- `speedrun_weekly_solo_easy_s10`

**Total leaderboard keys:**
- Bounty: 3 parties × 7 difficulties × 2 time scopes = **42**
- SpeedRun: 3 parties × 7 difficulties × 7 checkpoints × 2 time scopes = **294**
- **Total: 336 keys**

Adding Quads later: +112 keys (448 total). No structural changes needed — just a new party value.

---

## 5. Database Schema

### 5.1 `player_profiles`

Cached Steam identity. Updated on every submission.

```sql
CREATE TABLE player_profiles (
  steam_id        TEXT PRIMARY KEY,
  display_name    TEXT NOT NULL,
  avatar_url      TEXT,
  hero_id         TEXT,                         -- last-used hero (for display)
  updated_at      TIMESTAMPTZ DEFAULT NOW()
);
```

### 5.2 `leaderboard_entries`

One row per player per leaderboard key. Upserted on submission (keep best).

```sql
CREATE TABLE leaderboard_entries (
  id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  steam_id        TEXT NOT NULL REFERENCES player_profiles(steam_id),
  leaderboard_key TEXT NOT NULL,
  score           BIGINT NOT NULL,              -- bounty value, or time in milliseconds
  hero_id         TEXT NOT NULL,
  companion_id    TEXT,
  stage_reached   INT NOT NULL,
  party_size      TEXT NOT NULL,                -- 'solo', 'duo', 'trio'
  difficulty      TEXT NOT NULL,
  time_scope      TEXT NOT NULL,                -- 'weekly', 'alltime'
  run_id          UUID,                         -- shared across co-op party members
  submitted_at    TIMESTAMPTZ DEFAULT NOW(),

  UNIQUE(leaderboard_key, steam_id)
);

-- Fast Top 10 queries
CREATE INDEX idx_lb_entries_key_score
  ON leaderboard_entries(leaderboard_key, score DESC);

-- Fast player rank lookup
CREATE INDEX idx_lb_entries_key_steamid
  ON leaderboard_entries(leaderboard_key, steam_id);

-- Fast friends queries (steam_id IN list)
CREATE INDEX idx_lb_entries_steamid
  ON leaderboard_entries(steam_id);

-- Weekly cleanup
CREATE INDEX idx_lb_entries_time_scope_submitted
  ON leaderboard_entries(time_scope, submitted_at);
```

**Score convention:**
- **Bounty:** stored as-is (higher = better). Sort `DESC`.
- **SpeedRun:** stored as milliseconds (lower = better). Sort `ASC`.

The API / query layer uses the `type` prefix of the leaderboard_key to determine sort direction.

### 5.3 `run_summaries`

Full run data for Top 10 Global/Streamers entries + quarantined runs.

```sql
CREATE TABLE run_summaries (
  id                UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  entry_id          UUID REFERENCES leaderboard_entries(id) ON DELETE SET NULL,
  steam_id          TEXT NOT NULL,
  leaderboard_key   TEXT NOT NULL,
  run_id            UUID,                       -- shared across co-op members
  party_slot        INT DEFAULT 0,              -- 0=solo/player1, 1=player2, 2=player3
  schema_version    INT DEFAULT 6,

  -- Run identity
  hero_id           TEXT,
  companion_id      TEXT,
  stage_reached     INT,
  bounty            INT,
  time_seconds      FLOAT,

  -- Stats
  hero_level        INT,
  stats             JSONB,                      -- {damage, attackSpeed, scale, armor, evasion, luck, speed}
  secondary_stats   JSONB,

  -- Ratings
  luck_rating       INT,                        -- 0..100
  luck_quantity     INT,                        -- 0..100
  luck_quality      INT,                        -- 0..100
  skill_rating      INT,                        -- 0..100

  -- Build snapshot
  equipped_idols    JSONB,                      -- ["idol_id_1", "idol_id_2", ...]
  inventory         JSONB,                      -- ["item_id_1", "item_id_2", ...]
  event_log         JSONB,                      -- ["event string 1", ...] (v1: strings; v2: structured)
  damage_by_source  JSONB,                      -- {"AutoAttack": 50000, "Ultimate": 30000}

  -- Proof of run
  proof_of_run_url  TEXT,
  proof_locked      BOOLEAN DEFAULT FALSE,

  -- Display (for co-op picker)
  display_name      TEXT,

  -- Lifecycle
  created_at        TIMESTAMPTZ DEFAULT NOW(),
  expires_at        TIMESTAMPTZ                 -- NULL = no expiry (active Top 10)
);

CREATE INDEX idx_run_summaries_entry ON run_summaries(entry_id);
CREATE INDEX idx_run_summaries_run_id ON run_summaries(run_id);
CREATE INDEX idx_run_summaries_expires ON run_summaries(expires_at) WHERE expires_at IS NOT NULL;
```

### 5.4 `streamer_whitelist`

SteamIDs of whitelisted streamers for the Streamers tab.

```sql
CREATE TABLE streamer_whitelist (
  steam_id        TEXT PRIMARY KEY,
  display_name    TEXT,
  platform        TEXT DEFAULT 'twitch',        -- 'twitch', 'youtube', etc.
  added_at        TIMESTAMPTZ DEFAULT NOW()
);
```

### 5.5 `account_restrictions`

Suspicion / Cheating Certainty records per account.

```sql
CREATE TABLE account_restrictions (
  steam_id          TEXT PRIMARY KEY,
  restriction       TEXT NOT NULL,              -- 'suspicion' or 'certainty'
  reason            TEXT NOT NULL,              -- human-readable reason string
  flag_category     TEXT,                       -- 'score', 'gold', 'items', 'too_lucky', 'dodging_too_perfect'
  run_summary_id    UUID REFERENCES run_summaries(id) ON DELETE SET NULL,
  appeal_status     TEXT DEFAULT 'not_submitted', -- 'not_submitted', 'submitted', 'under_review', 'approved', 'denied'
  appeal_message    TEXT,
  appeal_evidence   TEXT,
  appeal_submitted_at TIMESTAMPTZ,
  created_at        TIMESTAMPTZ DEFAULT NOW(),
  resolved_at       TIMESTAMPTZ
);
```

### 5.6 `run_reports`

Player reports ("This Run is Cheating").

```sql
CREATE TABLE run_reports (
  id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  reporter_steam_id   TEXT NOT NULL,
  target_entry_id     UUID REFERENCES leaderboard_entries(id) ON DELETE CASCADE,
  target_steam_id     TEXT NOT NULL,
  reason              TEXT,                     -- aligns with flag categories
  evidence_url        TEXT,
  created_at          TIMESTAMPTZ DEFAULT NOW(),
  expires_at          TIMESTAMPTZ               -- 30 days from creation
);

CREATE INDEX idx_run_reports_target ON run_reports(target_steam_id);
CREATE INDEX idx_run_reports_expires ON run_reports(expires_at);
```

### 5.7 `quarantined_runs`

Runs auto-flagged by integrity checks.

```sql
CREATE TABLE quarantined_runs (
  id                UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  steam_id          TEXT NOT NULL,
  run_summary_id    UUID REFERENCES run_summaries(id) ON DELETE SET NULL,
  flag_category     TEXT NOT NULL,              -- 'score', 'gold', 'items', 'too_lucky', 'dodging_too_perfect'
  reason_string     TEXT NOT NULL,
  supporting_data   JSONB,                      -- {score, gold, kills, luck, bound_exceeded, ...}
  resolution        TEXT DEFAULT 'pending',     -- 'pending', 'cleared', 'confirmed_cheat'
  created_at        TIMESTAMPTZ DEFAULT NOW(),
  expires_at        TIMESTAMPTZ                 -- 180 days from creation
);

CREATE INDEX idx_quarantined_steam ON quarantined_runs(steam_id);
CREATE INDEX idx_quarantined_resolution ON quarantined_runs(resolution);
CREATE INDEX idx_quarantined_expires ON quarantined_runs(expires_at);
```

### 5.8 `bug_reports`

Player-submitted bug reports with run context.

```sql
CREATE TABLE bug_reports (
  id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  steam_id        TEXT NOT NULL,
  message         TEXT NOT NULL,
  run_context     JSONB,                        -- {stage, mode, difficulty, hero, companion, ...}
  created_at      TIMESTAMPTZ DEFAULT NOW(),
  expires_at      TIMESTAMPTZ                   -- 30 days from creation
);

CREATE INDEX idx_bug_reports_expires ON bug_reports(expires_at);
```

### 5.9 `audit_log`

Every moderation action taken in the admin portal.

```sql
CREATE TABLE audit_log (
  id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  admin_id        TEXT NOT NULL,                -- admin username/identifier
  action          TEXT NOT NULL,                -- 'ban', 'suspend', 'unsuspend', 'approve_appeal', 'deny_appeal', ...
  target_steam_id TEXT,
  details         JSONB,                        -- action-specific context
  created_at      TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_audit_log_target ON audit_log(target_steam_id);
CREATE INDEX idx_audit_log_created ON audit_log(created_at DESC);
```

---

## 6. API Contracts

### Base URL

Development: `http://localhost:3000`  
Production: `https://t66-backend.vercel.app` (or custom domain)

### 6.1 `POST /api/submit-run`

Submit a run result at a checkpoint. Called at stages 10, 20, 30, 40, 50, 60, 66.

**Auth:** Steam session ticket (hex-encoded in header).

**Headers:**
```
X-Steam-Ticket: <hex-encoded session ticket>
Content-Type: application/json
```

**Request body:**
```json
{
  "display_name": "PlayerOne",
  "avatar_url": "https://avatars.steamstatic.com/...",
  "run": {
    "hero_id": "KingArthur",
    "companion_id": "Dragon",
    "difficulty": "hard",
    "party_size": "solo",
    "checkpoint": 30,
    "stage_reached": 30,
    "bounty": 145000,
    "time_ms": 1823500,
    "hero_level": 45,
    "stats": {
      "damage": 12, "attackSpeed": 8, "scale": 5,
      "armor": 7, "evasion": 6, "luck": 4, "speed": 10
    },
    "secondary_stats": {},
    "luck_rating": 67,
    "luck_quantity": 55,
    "luck_quality": 79,
    "skill_rating": 82,
    "equipped_idols": ["idol_fire", "idol_ice"],
    "inventory": ["item_sword_white", "item_shield_red"],
    "event_log": [],
    "damage_by_source": {"AutoAttack": 50000, "Ultimate": 30000},
    "proof_of_run_url": ""
  },
  "co_op": {
    "run_id": "uuid-shared-across-party",
    "party_members": [
      {
        "steam_id": "76561198000000001",
        "display_name": "PlayerOne",
        "avatar_url": "...",
        "party_slot": 0,
        "hero_id": "KingArthur",
        "run_summary": { "...same fields as run above..." }
      },
      {
        "steam_id": "76561198000000002",
        "display_name": "PlayerTwo",
        "avatar_url": "...",
        "party_slot": 1,
        "hero_id": "Musashi",
        "run_summary": { "...same fields as run above..." }
      }
    ]
  }
}
```

**Notes:**
- `co_op` is `null` for solo runs.
- For co-op, the host submits on behalf of all party members. Each member's run summary is included.
- `event_log` is empty for v1 (structured event log is Phase 12-14). Anti-cheat checks use summary data only.

**Response (success):**
```json
{
  "status": "accepted",
  "bounty_rank_alltime": 7,
  "bounty_rank_weekly": 3,
  "speedrun_rank_alltime": null,
  "speedrun_rank_weekly": null,
  "is_new_personal_best_bounty": true,
  "is_new_personal_best_speedrun": false
}
```

**Response (flagged):**
```json
{
  "status": "flagged",
  "restriction": "suspicion",
  "reason": "Too Lucky",
  "flag_category": "too_lucky"
}
```

**Response (banned):**
```json
{
  "status": "banned",
  "restriction": "certainty",
  "reason": "Duplicate White item despite uniqueness ON",
  "flag_category": "items"
}
```

**Server-side logic:**
1. Validate session ticket (check KV cache → miss: call Steam `AuthenticateUserTicket` → cache result 2hr)
2. Check `account_restrictions` — if permanently banned, reject immediately
3. Run anti-cheat validation checks (see Section 9)
4. If flagged/banned: create `account_restrictions` + `quarantined_runs` entries, return flag response
5. If clean: determine leaderboard keys (bounty + speedrun, weekly + alltime)
6. For each key: upsert `leaderboard_entries` (keep best score)
7. Check if entry is now in Top 10 — if yes, store/update `run_summaries`
8. Upsert `player_profiles`
9. For co-op: repeat steps 6-8 for each party member
10. Return rank info

### 6.2 `GET /api/leaderboard`

Fetch Top 10 for a specific board. Publicly cacheable.

**Auth:** None (public).

**Query parameters:**
| Param | Required | Values |
|---|---|---|
| `type` | Yes | `bounty`, `speedrun` |
| `time` | Yes | `weekly`, `alltime` |
| `party` | Yes | `solo`, `duo`, `trio` |
| `difficulty` | Yes | `easy`, `medium`, `hard`, `veryhard`, `impossible`, `perdition`, `final` |
| `stage` | SpeedRun only | `10`, `20`, `30`, `40`, `50`, `60`, `66` |
| `filter` | No | `global` (default), `streamers` |

**Response:**
```json
{
  "leaderboard_key": "bounty_alltime_solo_hard",
  "entries": [
    {
      "rank": 1,
      "entry_id": "uuid",
      "steam_id": "76561198...",
      "display_name": "ProGamer",
      "avatar_url": "https://...",
      "hero_id": "Musashi",
      "score": 250000,
      "stage_reached": 66,
      "party_size": "solo",
      "run_id": null,
      "has_run_summary": true,
      "submitted_at": "2026-02-16T12:00:00Z"
    }
  ],
  "total_entries": 4523
}
```

**Notes:**
- For co-op boards: entries are deduplicated by `run_id`. Response includes all party members' display names.
- `filter=streamers`: only entries WHERE `steam_id IN (SELECT steam_id FROM streamer_whitelist)`.
- The local player's entry is included if present in Top 10.

**Cache:** `Cache-Control: public, s-maxage=60, stale-while-revalidate=300`

### 6.3 `GET /api/leaderboard/friends`

Fetch leaderboard entries for a list of friend SteamIDs.

**Auth:** Steam session ticket (to identify the requesting player).

**Headers:**
```
X-Steam-Ticket: <hex-encoded session ticket>
```

**Query parameters:**
| Param | Required | Values |
|---|---|---|
| `type` | Yes | `bounty`, `speedrun` |
| `time` | Yes | `weekly`, `alltime` |
| `party` | Yes | `solo`, `duo`, `trio` |
| `difficulty` | Yes | `easy`...`final` |
| `stage` | SpeedRun only | `10`...`66` |
| `friend_ids` | Yes | Comma-separated SteamIDs |

**Response:** Same shape as `/api/leaderboard`, but filtered to the provided SteamIDs + the requesting player. Includes the requesting player's entry (ranked among friends).

**Notes:**
- `has_run_summary` is always `false` for friends entries (except the requesting player's own entry, which they view locally).
- This endpoint is NOT publicly cached (unique per player's friend set). Response includes `Cache-Control: private, max-age=60`.

### 6.4 `GET /api/my-rank`

Lightweight endpoint to get the requesting player's rank on a specific board.

**Auth:** Steam session ticket.

**Query parameters:** Same as `/api/leaderboard` (type, time, party, difficulty, stage).

**Response:**
```json
{
  "leaderboard_key": "bounty_alltime_solo_hard",
  "rank": 247,
  "score": 85000,
  "total_entries": 4523
}
```

**Notes:** Returns `rank: null` if the player has no entry on this board. Used for the "11th row" display.

### 6.5 `GET /api/run-summary/:id`

Fetch full run summary for a Global or Streamers Top 10 entry.

**Auth:** None (public — Top 10 run summaries are viewable by everyone per Bible §1.3.4).

**Path parameter:** `id` = `entry_id` from the leaderboard response.

**Query parameters:**
| Param | Required | Values |
|---|---|---|
| `slot` | Co-op only | `0`, `1`, `2` (party slot index) |

**Response:**
```json
{
  "entry_id": "uuid",
  "steam_id": "76561198...",
  "display_name": "ProGamer",
  "hero_id": "Musashi",
  "companion_id": "Dragon",
  "stage_reached": 66,
  "bounty": 250000,
  "time_seconds": 1823.5,
  "hero_level": 50,
  "stats": {"damage": 15, "attackSpeed": 10, "...": "..."},
  "secondary_stats": {},
  "luck_rating": 67,
  "luck_quantity": 55,
  "luck_quality": 79,
  "skill_rating": 82,
  "equipped_idols": ["idol_fire", "idol_ice"],
  "inventory": ["item_sword_white", "item_shield_red"],
  "event_log": [],
  "damage_by_source": {"AutoAttack": 50000, "Ultimate": 30000},
  "proof_of_run_url": "https://youtube.com/...",
  "party_members": [
    {"steam_id": "...", "display_name": "...", "hero_id": "...", "party_slot": 0},
    {"steam_id": "...", "display_name": "...", "hero_id": "...", "party_slot": 1}
  ]
}
```

**Notes:**
- For solo: `party_members` is null.
- For co-op without `slot` param: returns the first player's summary + party member list (for the Player Summary Picker).
- For co-op with `slot` param: returns that specific player's detailed summary.

### 6.6 `POST /api/report-run`

Report a Global/Streamers Top 10 run as suspicious.

**Auth:** Steam session ticket.

**Request body:**
```json
{
  "target_entry_id": "uuid",
  "reason": "Score above limit",
  "evidence_url": "https://youtube.com/..."
}
```

**Response:**
```json
{
  "status": "submitted"
}
```

**Rate limits:** 5 reports/minute, 20 reports/day per SteamID (per Bible §1.3.5).

### 6.7 `GET /api/account-status`

Check if the requesting player's account has restrictions.

**Auth:** Steam session ticket.

**Response (no restriction):**
```json
{
  "restriction": "none"
}
```

**Response (suspended):**
```json
{
  "restriction": "suspicion",
  "reason": "Too Lucky",
  "flag_category": "too_lucky",
  "appeal_status": "not_submitted",
  "run_summary_id": "uuid"
}
```

**Response (banned):**
```json
{
  "restriction": "certainty",
  "reason": "Duplicate White item despite uniqueness ON",
  "flag_category": "items",
  "appeal_status": null,
  "run_summary_id": "uuid"
}
```

### 6.8 `POST /api/submit-appeal`

Submit an appeal for a suspicion-flagged account.

**Auth:** Steam session ticket.

**Request body:**
```json
{
  "message": "I had an extremely lucky run. Here is the VOD...",
  "evidence_url": "https://youtube.com/..."
}
```

**Response:**
```json
{
  "status": "submitted",
  "appeal_status": "submitted"
}
```

**Rules:** Only allowed when `restriction = 'suspicion'` and `appeal_status = 'not_submitted'`. One appeal per restriction.

### 6.9 `PUT /api/proof-of-run`

Set or update the proof-of-run URL for the player's own Top 10 entry.

**Auth:** Steam session ticket.

**Request body:**
```json
{
  "entry_id": "uuid",
  "proof_url": "https://youtube.com/..."
}
```

**Rules:** Only the entry owner can set this. Once `proof_locked = true`, it cannot be changed (set by admin if needed).

### 6.10 `POST /api/bug-report`

Submit a bug report with optional run context.

**Auth:** Steam session ticket.

**Request body:**
```json
{
  "message": "Game froze when opening vendor at stage 15",
  "run_context": {
    "stage": 15,
    "difficulty": "hard",
    "party_size": "solo",
    "hero_id": "KingArthur"
  }
}
```

---

## 7. Data Flows

### 7.1 Submit Run (Solo)

```
1. Player completes checkpoint (stage 10/20/30/40/50/60/66)
2. Client: ISteamUser::GetAuthSessionTicket() → ticket bytes → hex encode
3. Client: Serialize run data (hero, stats, inventory, idols, damage, ratings)
4. Client: POST /api/submit-run {ticket, run_payload}
5. Backend: Validate ticket (KV cache hit or Steam Web API call)
6. Backend: Anti-cheat checks on run payload
7. Backend: If clean → upsert leaderboard_entries (bounty_alltime, bounty_weekly, speedrun_alltime, speedrun_weekly)
8. Backend: If entry is now Top 10 on any board → upsert run_summaries
9. Backend: If previously Top 10 entry displaced → set run_summaries.expires_at = NOW() + 7 days
10. Backend: Upsert player_profiles (name, avatar)
11. Backend: Return {rank, status} to client
12. Client: Update local state, show "New Personal Best" if applicable
```

### 7.2 Submit Run (Co-op)

```
1. Host's client collects all party members' run data
2. Host: POST /api/submit-run with co_op.party_members array
3. Backend: Validate host's ticket
4. Backend: Anti-cheat checks on each member's run data
5. Backend: For EACH party member:
   a. Upsert leaderboard_entries (same score, same run_id, different steam_id)
   b. If Top 10: store individual run_summary (party_slot 0, 1, 2)
6. Backend: Upsert player_profiles for all members
7. Backend: Return ranks to host
8. Host: Distributes results to party members (via game networking)
```

### 7.3 Read Global / Streamers Leaderboard

```
1. Player opens Main Menu
2. Client: GET /api/leaderboard?type=bounty&time=alltime&party=solo&difficulty=hard
3. Vercel edge: serve from cache (< 60s old) or forward to serverless function
4. Backend (on cache miss): SELECT * FROM leaderboard_entries WHERE leaderboard_key = ? ORDER BY score DESC LIMIT 10, joined with player_profiles
5. For co-op boards: deduplicate by run_id, aggregate party member names
6. Return Top 10
7. Client: GET /api/my-rank (if player not in Top 10, for 11th row)
8. Client: Display leaderboard
```

### 7.4 Read Friends Leaderboard

```
1. Player selects Friends tab
2. Client: ISteamFriends::GetFriendCount() + GetFriendByIndex() → collect friend SteamIDs
3. Client: GET /api/leaderboard/friends?...&friend_ids=id1,id2,id3,...
4. Backend: SELECT * FROM leaderboard_entries WHERE leaderboard_key = ? AND steam_id = ANY($friend_ids) ORDER BY score DESC LIMIT 10
5. Return friends' entries (no run summaries — has_run_summary = false except player's own)
6. Client: Display. Click own entry → open local save. Click others → no-op / tooltip.
```

### 7.5 View Run Summary (Global / Streamers Click)

```
1. Player clicks a Top 10 row (Solo)
2. Client: GET /api/run-summary/{entry_id}
3. Backend: Return full run summary
4. Client: Display Run Summary Page

-- Co-op variant: --
1. Player clicks a Top 10 row (Duo/Trio)
2. Client: GET /api/run-summary/{entry_id} (no slot param)
3. Backend: Return summary + party_members list
4. Client: Display Player Summary Picker (2 or 3 cards)
5. Player clicks a card
6. Client: GET /api/run-summary/{entry_id}?slot=1
7. Client: Display that player's Run Summary Page
```

### 7.6 Weekly Reset

```
1. Vercel cron fires every Monday 00:00 UTC
2. DELETE FROM leaderboard_entries WHERE time_scope = 'weekly'
3. Associated run_summaries: set expires_at = NOW() + 7 days (cleanup cron handles deletion)
4. Next submissions create fresh weekly entries
```

### 7.7 Moderation Flow

```
Report:
1. Player clicks "This Run is Cheating" on a Global/Streamers Run Summary
2. Client: POST /api/report-run {target_entry_id, reason, evidence_url}
3. Backend: Insert into run_reports (rate-limited per reporter)
4. Run remains visible (no auto-removal)

Admin Review:
1. Admin opens /admin/reports
2. Sees reported runs with reasons, evidence, and full run summary data
3. Admin decides: dismiss report, or flag account (suspicion/certainty)
4. If flagging: insert/update account_restrictions, log in audit_log
5. Flagged player sees Account Status button on Main Menu next login
```

---

## 8. Steam Integration (Client-Side)

### 8.1 Required Steamworks SDK Calls

| Call | When | Purpose |
|---|---|---|
| `SteamAPI_Init()` | Game startup | Initialize Steam |
| `ISteamUser::GetAuthSessionTicket()` | Before any backend call | Get session ticket for auth |
| `ISteamUser::GetSteamID()` | Startup | Get local player's SteamID |
| `ISteamFriends::GetPersonaName()` | Startup | Get display name |
| `ISteamFriends::GetMediumFriendAvatar()` | Startup | Get avatar image handle |
| `ISteamFriends::GetFriendCount(k_EFriendFlagImmediate)` | Friends tab open | Count friends |
| `ISteamFriends::GetFriendByIndex(i, k_EFriendFlagImmediate)` | Friends tab open | Get friend SteamIDs |
| `ISteamFriends::GetFriendPersonaName(steamID)` | Friends tab | Get friend display names (client-side) |
| `ISteamUtils::GetImageRGBA()` | Avatar display | Convert avatar handle to pixels |
| `ISteamRemoteStorage::*` | Save/Load | Steam Cloud saves |
| `ISteamMatchmaking::*` | Co-op | Lobby creation and invites |

### 8.2 Session Ticket Flow (UE5 C++)

```cpp
// In UT66OnlineSubsystem or similar:

// 1. Get ticket
HAuthTicket TicketHandle;
uint8 TicketBuffer[1024];
uint32 TicketSize = 0;
TicketHandle = SteamUser()->GetAuthSessionTicket(
    TicketBuffer, sizeof(TicketBuffer), &TicketSize, nullptr);

// 2. Hex-encode for HTTP header
FString TicketHex;
for (uint32 i = 0; i < TicketSize; i++)
{
    TicketHex += FString::Printf(TEXT("%02x"), TicketBuffer[i]);
}

// 3. Use in HTTP request
Request->SetHeader(TEXT("X-Steam-Ticket"), TicketHex);
```

### 8.3 HTTP Client Pattern (UE5 C++)

```cpp
// GET request pattern
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
Request->SetURL(FString::Printf(TEXT("%s/api/leaderboard?type=bounty&time=alltime&party=solo&difficulty=hard"),
    *BackendBaseUrl));
Request->SetVerb(TEXT("GET"));
Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnLeaderboardResponse);
Request->ProcessRequest();

// POST request pattern
TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
Request->SetURL(FString::Printf(TEXT("%s/api/submit-run"), *BackendBaseUrl));
Request->SetVerb(TEXT("POST"));
Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
Request->SetHeader(TEXT("X-Steam-Ticket"), TicketHex);
Request->SetContentAsString(RunPayloadJson);
Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnSubmitRunResponse);
Request->ProcessRequest();
```

### 8.4 Backend Base URL Config

Store in `DefaultGame.ini` (not hardcoded in C++):

```ini
[T66.Online]
BackendBaseUrl=https://t66-backend.vercel.app
```

Read at startup in the online subsystem/leaderboard subsystem.

---

## 9. Anti-Cheat Validation

### 9.1 Phase 1 — Basic Checks (summary data only, no event log required)

Run on every `/api/submit-run`:

| Check | Logic | Outcome |
|---|---|---|
| **Score hard cap** | Each checkpoint has a theoretical max bounty (derived from wave count × max enemy points). Reject if `bounty > max_for_checkpoint`. | Certainty (if grossly over) or Suspicion (if borderline) |
| **Time plausibility** | Minimum possible time per checkpoint (speed of fastest theoretical clear). Reject if `time_ms < min_for_checkpoint`. | Certainty |
| **Luck Rating threshold** | If `luck_rating > 100`, flag for review. | Suspicion (Too Lucky) |
| **Skill Rating threshold** | If `skill_rating > 100`, flag for review. | Suspicion |
| **Inventory size** | Max possible items for the checkpoint stage. Reject if `inventory.length > max`. | Certainty |
| **Idol count** | Max 6 idols. Reject if `equipped_idols.length > 6`. | Certainty |
| **Duplicate check** | Same SteamID submitting the same checkpoint within 30 seconds. Reject. | Rate limit |

### 9.2 Phase 2 — Deep Checks (requires structured event log, deferred)

Added after the event log redesign (Phase 12-14 in implementation plan):

| Check | What it validates |
|---|---|
| **Score-kill consistency** | Sum of enemy point values from kill events = bounty |
| **Gold cap (non-gambling)** | Gold total matches gold-granting events |
| **Item provenance** | Every inventory item maps to a Vendor purchase or Loot Bag drop event |
| **Wheel spin count** | Within bounded RNG model for the run's Luck |
| **Loot bag count** | Within bounded RNG model for the run's Luck |
| **Dodge rating validation** | Dodge timing regularity is human-plausible |
| **White rarity cooldown** | No two White loot bags back-to-back |
| **White uniqueness** | No duplicate White items if uniqueness ON |
| **Item pool validity** | Every item came from its correct rarity pool |

### 9.3 Forbidden Events (Cheating Certainty — zero appeal)

Per Bible §1.3.6:

- Two White Loot Bags back-to-back (cooldown violation)
- Duplicate White item despite uniqueness ON
- Loot Bag produces item not in its rarity pool
- Item acquired without valid acquisition event record

These result in permanent leaderboard ban (`restriction = 'certainty'`).

---

## 10. Cron Jobs

| Job | Schedule | Logic |
|---|---|---|
| **Weekly reset** | Monday 00:00 UTC | `DELETE FROM leaderboard_entries WHERE time_scope = 'weekly'`. Set displaced `run_summaries.expires_at = NOW() + 7 days`. |
| **Data retention** | Daily 03:00 UTC | Delete `bug_reports` where `expires_at < NOW()`. Delete `run_reports` where `expires_at < NOW()`. Delete `quarantined_runs` where `expires_at < NOW()`. Delete `run_summaries` where `expires_at < NOW()`. |

**Vercel cron config** (`vercel.json`):
```json
{
  "crons": [
    { "path": "/api/cron/weekly-reset", "schedule": "0 0 * * 1" },
    { "path": "/api/cron/retention-cleanup", "schedule": "0 3 * * *" }
  ]
}
```

---

## 11. Admin Portal

The admin portal is a set of Next.js pages under `/admin/*`, protected by password authentication (upgradeable to OAuth later).

### 11.1 Pages

| Page | Purpose |
|---|---|
| `/admin/login` | Admin login gate |
| `/admin/dashboard` | Overview: pending appeals, recent reports, recent quarantines |
| `/admin/appeals` | Appeals queue. View appeal message + evidence + full run summary. Actions: Approve / Deny. |
| `/admin/reports` | Reported runs queue. View report reason + evidence + full run summary. Actions: Dismiss / Flag account (suspicion or certainty). |
| `/admin/quarantine` | Auto-quarantined runs. View flag reason + supporting data + full run summary. Actions: Clear / Confirm cheat. |
| `/admin/accounts` | Search by SteamID. View player's restriction history, submissions, reports. Actions: Suspend / Unsuspend / Ban / Add internal note. |
| `/admin/streamers` | Manage streamer whitelist. Add / Remove SteamIDs. |
| `/admin/audit` | Chronological audit log of every admin action. Read-only. |

### 11.2 Admin Auth

**Phase 1:** Simple password gate.
- Environment variable `ADMIN_PASSWORD` in Vercel.
- `/admin/login` page checks password, sets an HTTP-only secure cookie.
- All `/admin/*` pages check cookie via middleware.

**Phase 2 (optional):** Upgrade to OAuth (GitHub, Google) if multiple admins are needed.

---

## 12. Caching Strategy

| Endpoint | Cache | TTL | Notes |
|---|---|---|---|
| `GET /api/leaderboard` | Vercel Edge (CDN) | 60s (`s-maxage=60, stale-while-revalidate=300`) | Same response for all players per board. |
| `GET /api/leaderboard/friends` | None (or per-user KV) | 60s | Unique per player's friend set. |
| `GET /api/my-rank` | None | — | Lightweight indexed query, fast enough without cache. |
| `GET /api/run-summary/:id` | Vercel Edge | 300s | Run summaries change rarely (only proof URL updates). |
| `GET /api/account-status` | None | — | Per-user, must be fresh. |
| `POST /*` | None | — | Writes are never cached. |

**Session ticket validation cache (Vercel KV):**
- Key: SHA-256 hash of ticket bytes
- Value: validated SteamID
- TTL: 2 hours
- Reduces `AuthenticateUserTicket` calls by ~7x (one per session instead of one per checkpoint)

---

## 13. Error Handling & Offline Behavior

| Scenario | Client behavior |
|---|---|
| Backend unreachable at run start | Mark run as ineligible for leaderboard. Player can still play. No submission attempted at checkpoints. |
| Backend unreachable mid-run | Run becomes ineligible immediately (per Bible §1.3.3). Already-submitted checkpoints are persisted. |
| Backend unreachable on Main Menu | Show locally cached leaderboard data (last successful response). Retry on next Main Menu open or manual refresh. |
| Submission rejected (anti-cheat flag) | Client stores restriction info locally. Shows Account Status button on Main Menu. |
| Submission timeout (> 10s) | Treat as failed. Do not retry. Run may become ineligible. |
| Steam offline at game start | No session ticket available. Run is ineligible. Local-only play allowed. |
| Invalid/expired session ticket | Backend returns 401. Client requests new ticket and retries once. |

**Eligibility tracking (client-side):**
- `bIsLeaderboardEligible` flag in `UT66RunStateSubsystem` (or similar).
- Set `true` at run start if: Steam is running, backend is reachable, account is not restricted, "Submit Scores" setting is on.
- Set `false` permanently (for the run) if: connection lost, submission fails, or player toggles "Submit Scores" off.
- Ineligible runs can still unlock Achievements (per Bible §1.3.3).

---

## 14. Data Retention (Per Bible §1.3.9)

| Data | Retention |
|---|---|
| Top 10 run summaries | While in Top 10 + 7 days after displaced |
| Bug reports | 30 days |
| Run reports | 30 days (pending review) |
| Resolved appeals | 90 days post-resolution |
| Quarantined artifacts | 180 days |
| Player profiles | Indefinite (small, useful) |
| Leaderboard entries (alltime) | Indefinite (small per-row) |
| Leaderboard entries (weekly) | Reset every Monday |
| Audit log | Indefinite |

---

## 15. Security Model

| Layer | Mechanism |
|---|---|
| Client → Backend auth | Steam session ticket validated via `AuthenticateUserTicket` Web API (cached in KV) |
| Rate limiting (submissions) | 1 per 30 seconds per SteamID |
| Rate limiting (reports) | 5/minute, 20/day per SteamID |
| Rate limiting (reads) | 120/minute per IP |
| Admin portal auth | Password cookie (Phase 1), OAuth (Phase 2) |
| Steam Web API key | Server-side `STEAM_WEB_API_KEY` env var only |
| HTTPS | All communication (Vercel default, enforced) |
| No client secrets | Game client only knows the backend URL, no API keys |
| Input validation | All API inputs validated and sanitized server-side |
| SQL injection prevention | Parameterized queries via Drizzle ORM (never raw string interpolation) |

---

## 16. Environment Variables

Set in Vercel project dashboard (Settings → Environment Variables):

| Variable | Source | Purpose |
|---|---|---|
| `STEAM_WEB_API_KEY` | Steamworks partner portal (publisher key) | `AuthenticateUserTicket` calls |
| `STEAM_APP_ID` | Steamworks partner portal | Identifies T66 |
| `POSTGRES_URL` | Auto-set by Vercel Postgres | Database connection |
| `KV_REST_API_URL` | Auto-set by Vercel KV | Redis connection |
| `KV_REST_API_TOKEN` | Auto-set by Vercel KV | Redis auth |
| `ADMIN_PASSWORD` | You choose | Admin portal login |
| `CRON_SECRET` | Auto-generated | Vercel cron auth |

---

## 17. Tech Stack (Backend Repo)

| Layer | Choice | Why |
|---|---|---|
| Framework | **Next.js 15** (App Router) | Native Vercel support, API routes + pages in one project |
| Language | **TypeScript** | Type safety for API contracts and DB schema |
| ORM | **Drizzle** | Lightweight, fast cold starts on serverless, type-safe SQL |
| Database | **Vercel Postgres (Neon)** | Serverless, autoscaling, zero-config with Vercel |
| Cache | **Vercel KV (Redis)** | Session ticket caching |
| Validation | **Zod** | Runtime input validation for all API endpoints |
| Auth | **Custom middleware** | Steam ticket validation + admin password check |

---

## 18. Accounts & Setup Prerequisites

| Account | Status | Action needed |
|---|---|---|
| **Steamworks Partner** | Already have | Get the publisher Web API key (Users & Permissions → Web API Key) |
| **Vercel** | Need | Sign up at vercel.com, Pro tier recommended ($20/mo) |
| **GitHub** | Likely have | Create `t66-backend` repo |
| **Node.js** | Local install | Install v22 LTS from nodejs.org |

---

## 19. Implementation Phases

| Phase | What | Depends on event log? |
|---|---|---|
| **1** | Vercel project setup (Next.js + Postgres + Drizzle schema + KV) | No |
| **2** | Steam integration in UE5 (session tickets, identity, friends list via Steamworks) | No |
| **3** | `POST /api/submit-run` — validate ticket, basic anti-cheat, store entry + run summary | No |
| **4** | `GET /api/leaderboard` — serve Top 10 from Postgres with edge caching | No |
| **5** | `GET /api/run-summary/:id` — serve run summaries for Global + Streamers | No |
| **6** | UE5 client: replace placeholder leaderboard with real HTTP calls (FHttpModule) | No |
| **7** | `GET /api/leaderboard/friends` + friends list from Steam client | No |
| **8** | `GET /api/my-rank` + 11th row display | No |
| **9** | Streamers whitelist + `/admin/streamers` + Streamers filter | No |
| **10** | Co-op submission (multi-member submit, dedup, Player Summary Picker) | No |
| **11** | `POST /api/report-run` + `GET /api/account-status` + `POST /api/submit-appeal` | No |
| **12** | Admin portal pages (dashboard, appeals, reports, quarantine, accounts, audit) | No |
| **13** | Cron jobs (weekly reset, data retention cleanup) | No |
| **14** | `POST /api/bug-report` | No |
| **15** | `PUT /api/proof-of-run` | No |
| **16** | Steam Cloud save integration | No |
| **17** | Structured event log redesign (client-side recording) | **This IS the redesign** |
| **18** | Deep anti-cheat validation in `/api/submit-run` (server replays event log) | Yes |
| **19** | Auto-quarantine pipeline | Yes |

Phases 1–16 deliver a fully functional leaderboard system with real Steam identity, run summaries, moderation tools, and basic integrity. Phases 17–19 add the deep anti-cheat layer.

---

## 20. Discrepancies with Bible (T66_Bible.md)

The Bible was written before this architecture was finalized. These are the known differences:

| Bible says | This architecture does | Rationale |
|---|---|---|
| "Steam Leaderboards: The authoritative competitive ranking layer" (§1.3.1) | Postgres is the competitive ranking layer. Steam is identity + social only. | Steam API rate limits make server-side writes infeasible at 200k concurrent. Postgres gives full control, richer data, single source of truth. |
| "Runs are validated and then submitted through a secure server-side path to Steam" (§1.3.2) | Runs are validated server-side and submitted to Postgres only. | Same trusted-write principle — the backend is the gatekeeper — just writing to Postgres instead of Steam. Stronger security since there's no parallel write path a cheater can exploit. |
| Party sizes: Solo, Duo, Trio only (§1.2) | Same, with Quads designed to be trivially addable (new enum value + new leaderboard keys). | User wants Quads as a future option. |
| Streamers tab: no mention of clickable run summaries (§1.3.4) | Streamers Top 10 entries are clickable and show run summaries (same as Global). | User requirement. Backend stores run summaries for Streamers entries same as Global. |
| Friends tab: implies Steam leaderboard read (§1.2) | Friends tab uses backend API with friend SteamIDs from client. | No Steam leaderboards exist. Backend query achieves the same result. |
| No Weekly/All-Time distinction in detail (§1.3.1 mentions "Weekly/All-Time") | Separate weekly and alltime leaderboard keys. Weekly entries deleted by cron every Monday. | User confirmed both are needed. |
