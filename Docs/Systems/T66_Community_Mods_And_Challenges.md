# T66 Community Mods And Challenges Proposal

**Last updated:** 2026-04-20
**Purpose:** Define a practical first implementation for `Official` / `Community` tabs, creator-facing submission flows, and the backend/admin approval loop for community challenges and mods.

## 1. Current Reality In Source

- The current screen is a modal with only two top-level tabs: `Challenges` and `Mods`.
- The entries are placeholder arrays hardcoded inside `Source/T66/UI/Screens/T66ChallengesScreen.cpp`.
- The selected item is stored only as `SelectedRunModifierKind` + `SelectedRunModifierID` in `Source/T66/Core/T66GameInstance.h`.
- A selected run modifier already makes the run ineligible for ranked leaderboards:
  - `Source/T66/Gameplay/T66GameMode.cpp`
  - `Source/T66/Core/T66RunIntegritySubsystem.cpp`
  - `ANTI_CHEAT/MASTER_ANTI_CHEAT.md`
- The current backend stack already supports:
  - authenticated player submissions from UE through `UT66BackendSubsystem`
  - Postgres persistence through the backend repo in `C:\UE\Backend`
  - admin review pages such as `/admin/reports` and `/admin/appeals`
  - a unified admin action route at `C:\UE\Backend\src\app\api\admin\action\route.ts`

## 2. Product Recommendation

The clean first version is:

- top-level tab 1: `Challenges`
- top-level tab 2: `Mods`
- second-level tab inside each: `Official` and `Community`
- a contextual create button:
  - `Create Challenge` when viewing `Challenges > Community`
  - `Create Mod` when viewing `Mods > Community`

The important implementation rule is:

- **Community challenge** = a data-driven ruleset submitted by a player
- **Community mod** = also a data-driven ruleset submitted by a player
- **Not** arbitrary uploaded code, DLLs, Blueprints, pak files, or Workshop content in v1

That rule matters because the current online/ranked policy already assumes pristine builds only, and the existing backend/admin system is designed for authenticated JSON submissions plus moderation, not executable user uploads.

## 3. UX Recommendation

## 3.1 Screen layout

Keep the existing outer shell and add a second segmented row in the area currently empty in the screenshot:

- left side: `Official` / `Community`
- right side:
  - `Create Challenge` or `Create Mod` when `Community` is active
  - hidden or disabled when `Official` is active

## 3.2 List behavior

- `Official` shows curated developer-authored content.
- `Community` shows approved player-authored content from the backend.
- Both lists use the same card layout so the UI only changes the data source, not the whole screen structure.

## 3.3 Create flow

Use a modal editor with:

- title
- short summary
- full description
- rule builder
- preview panel
- `Save Draft`
- `Playtest Locally`
- `Submit For Review`

For challenges only:

- allow `Suggested Chad Coupon Reward`
- make it explicit that this is a suggestion, not the final approved reward

For mods:

- no coupon field by default in v1
- if needed later, add a featured-mod reward separately

## 3.4 Player-facing status states

Each player submission should be visible under a small `My Submissions` / `My Drafts` view with statuses:

- `Draft`
- `Pending Review`
- `Approved`
- `Rejected`
- `Needs Changes`
- `Unpublished`

This is important because it lets players create and test content immediately without waiting for approval, while still keeping the public `Community` tab curated.

## 4. Content Model

Use one shared definition model for both challenges and mods.

Suggested fields:

- `content_id`
- `content_kind`: `challenge` or `mod`
- `content_source`: `official`, `community`, `draft`
- `status`: `draft`, `pending`, `approved`, `rejected`, `unpublished`
- `version`
- `title`
- `summary`
- `description`
- `author_steam_id`
- `author_display_name`
- `rule_payload` JSON
- `suggested_coupon_reward`
- `approved_coupon_reward`
- `review_notes`
- `created_at`
- `updated_at`
- `approved_at`
- `approved_by`

## 4.1 Rule payload

The rule payload should be built from a whitelist of modifier primitives, for example:

- start level override
- stat overrides
- enemy health multiplier
- enemy damage multiplier
- gold gain multiplier
- chest restrictions
- revive restrictions
- timer restrictions
- floor or boss restrictions
- shop restrictions
- flood pressure modifiers

The editor should compose these primitives into JSON.

That gives you:

- safe backend storage
- deterministic approval review
- no arbitrary code execution
- easier future balancing

## 5. Backend Recommendation

## 5.1 Database

Add two new backend tables in `C:\UE\Backend\src\db\schema.ts`.

### `community_content_submissions`

Immutable or mostly immutable submission records:

- id
- content_kind
- status
- author_steam_id
- author_display_name
- title
- summary
- description
- rule_payload jsonb
- suggested_coupon_reward
- approved_coupon_reward
- review_notes
- current_published_version_id nullable
- created_at
- updated_at

### `community_content_versions`

Published snapshots used by the game client:

- id
- submission_id
- version_number
- content_kind
- source = `community`
- title
- summary
- description
- rule_payload jsonb
- approved_coupon_reward
- published_at
- unpublished_at nullable

This split keeps moderation history clean and makes unpublish/re-publish safer than editing live rows in place.

## 5.2 Player API routes

Add routes like:

- `POST /api/community-content/submit`
- `GET /api/community-content?kind=challenge&source=community`
- `GET /api/community-content?kind=mod&source=community`
- `GET /api/my-community-content`
- `PUT /api/community-content/draft/:id`

These should follow the same pattern already used by:

- `POST /api/bug-report`
- `POST /api/report-run`
- `POST /api/submit-appeal`

Meaning:

- authenticate with Steam ticket
- validate with Zod
- rate limit submissions
- store JSON payloads in Postgres
- return concise status JSON

## 5.3 Admin API routes

Reuse the existing admin pattern instead of inventing a second moderation system.

Add actions to `POST /api/admin/action` such as:

- `approve_content`
- `reject_content`
- `request_changes`
- `unpublish_content`
- `feature_content`

Every action should also write to `audit_log`, matching the current moderation flow.

## 6. Admin Review Recommendation

Add a new admin page:

- `/admin/community`

Suggested columns:

- kind
- title
- author
- suggested reward
- status
- created date
- last update

Detail view should show:

- full description
- structured rule payload
- rendered human-readable modifier summary
- suggested reward
- final reward field
- review notes

Actions:

- `Approve`
- `Approve With Edited Reward`
- `Request Changes`
- `Reject`
- `Unpublish`

This aligns with the existing backend admin model already used for reports and appeals.

## 7. UE Runtime Recommendation

## 7.1 Replace hardcoded placeholder lists

The current hardcoded arrays in `Source/T66/UI/Screens/T66ChallengesScreen.cpp` should be replaced by a catalog source that can return:

- official challenges
- community challenges
- official mods
- community mods
- local drafts

## 7.2 Add a catalog subsystem

Create a dedicated subsystem such as `UT66RunModifierCatalogSubsystem` that:

- loads official definitions from local assets/DataTables
- fetches approved community definitions from the backend
- caches the results locally
- exposes a unified view model to the UI

## 7.3 Extend selected modifier identity

The current selection fields are too small for community publishing/versioning long-term.

Current:

- `SelectedRunModifierKind`
- `SelectedRunModifierID`

Recommended expansion:

- `SelectedRunModifierKind`
- `SelectedRunModifierSource`
- `SelectedRunModifierID`
- `SelectedRunModifierVersion`

## 7.4 Run summary telemetry

When a run starts with a selected official/community modifier, include that in backend-submitted run history.

Add run-summary fields such as:

- `modifier_kind`
- `modifier_source`
- `modifier_id`
- `modifier_version`

The run should still remain intentionally `unranked`, but you will preserve useful audit/history data and be able to show "completed community challenge X" later.

## 8. Reward Recommendation

For community challenges:

- creator submits a reward suggestion
- admin sets the final approved reward
- reward is granted only on successful completion of the approved published version
- reward should be one-time per player per published version

This avoids reward inflation and lets you revise a challenge later by publishing a new version.

For community mods:

- no Chad Coupon reward in v1
- keep them as sandbox/fun content first

## 9. Scope Recommendation

## Phase 1

- `Official` / `Community` sub-tabs
- backend-backed approved community list
- draft editor
- playtest local draft
- submit for review
- admin community review page
- approve / reject / unpublish actions
- community challenges can carry approved rewards

## Phase 2

- my submissions page in-game
- request changes workflow
- version history
- report community content
- featured content carousel

## Phase 3

- real Workshop / UGC discussion, if still wanted

That should be treated as a separate project, because true asset/code mods have very different safety, packaging, anti-cheat, and support requirements than data-driven community rulesets.

## 10. Recommended Direction

If the goal is to ship this relatively soon, the correct first move is:

- treat both community challenges and community mods as backend-approved structured rulesets
- keep official content local and curated
- let players create local drafts and play them instantly
- require approval before anything appears in the public `Community` tab
- keep all of it explicitly unranked

That fits the current T66 codebase, the current anti-cheat policy, and the current backend/admin architecture without forcing a Workshop-sized implementation.
