# T66 Community Mods And Challenges Implementation Plan

Date: 2026-04-20
Workspace: `C:\UE\T66`
Backend workspace: `C:\UE\Backend`
Companion concept doc: `Docs/Systems/T66_Community_Mods_And_Challenges.md`

## Objective

Implement a full official/community mods and challenges system that:

- ships curated `Official` mods and challenges authored by the developer
- lets players create `Community` mods and challenges in-game through a guided editor
- lets creators save local drafts and play them immediately
- lets creators submit content for developer approval
- gives the developer a private admin review surface for:
  - approving or rejecting community content
  - editing final challenge rewards
  - unpublishing community content
  - reviewing cheating appeals
  - removing or suppressing leaderboard content
- publishes approved community content back into the in-game `Community` tabs for all players

This is an implementation plan only. It does not perform the implementation.

## Locked Decisions

- `Official` content is authored by the developer and shipped in the game build.
- `Community` content is player-authored and backend-approved.
- Community `mods` are data-driven rule presets, not arbitrary code mods, Blueprints, DLLs, loose assets, pak files, or Workshop uploads.
- Community `challenges` may suggest Chad Coupon rewards, but only developer-approved published versions may actually grant coupons.
- Community `mods` do not grant Chad Coupons in v1.
- Player-created drafts are playable immediately by the creator, even before approval.
- All challenge/mod runs remain intentionally unranked.
- Steam Workshop is out of scope for v1.
- The private admin surface lives in the existing backend web app, not in the Vercel control panel and not in the shipped game client.

## High-Level Architecture Target

The target end state has six layers:

1. A shared gameplay modifier model built from safe, whitelisted rule primitives.
2. A local catalog layer for developer-authored `Official` content.
3. A backend-backed catalog layer for approved `Community` content.
4. An in-game creator/editor flow for drafts and submissions.
5. A private admin web UI for moderation, approval, appeals, and leaderboard actions.
6. A runtime execution path that applies selected rules, tracks completion, grants approved rewards, and keeps the run unranked.

## Non-Goals For This Rollout

- Steam Workshop integration
- executable code mods
- user-uploaded Blueprints or UE assets
- arbitrary user-authored scripting
- public web browsing of community content outside the game
- creator revenue share or paid UGC

## User Flows To Support

## Player Flow: Official Content

1. Open `Challenges` or `Mods`.
2. Choose `Official`.
3. Browse curated entries shipped with the game.
4. Select one and start a run.

## Player Flow: Community Draft

1. Open `Challenges > Community` or `Mods > Community`.
2. Click `Create Challenge` or `Create Mod`.
3. Build a rule preset through the editor.
4. Save it as a draft.
5. Play it locally.

## Player Flow: Community Submission

1. Open an existing local draft.
2. Click `Submit For Approval`.
3. The game sends the structured payload to the backend.
4. Submission status becomes `pending`.
5. Once approved, the published entry appears for everyone under `Community`.

## Developer Flow: Admin Review

1. Sign into the private admin website.
2. Open a new `Community Content` section.
3. Review pending submissions.
4. Approve, reject, request changes, or unpublish.
5. If approving a challenge, set the final Chad Coupon reward.
6. Published versions become visible to the game client.

## Developer Flow: Existing Moderation

The same admin surface must also continue to support:

- appeals
- reports
- quarantine review
- account actions
- leaderboard suppression or removal

## Phase 0: Finalize The Runtime Content Model

### Goal

Define the rule schema and execution model before any UI or backend submission work.

### Scope

- modifier primitives
- content identity
- versioning model
- reward policy
- selection state

### Primary Files

- `Docs/Systems/T66_Community_Mods_And_Challenges.md`
- `Source/T66/Core/T66GameInstance.h`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/Core/T66RunSaveGame.h`
- `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h`

### Tasks

- Define a single shared content definition model with fields such as:
  - `ContentId`
  - `ContentKind`
  - `ContentSource`
  - `Version`
  - `Title`
  - `Summary`
  - `Description`
  - `RulePayload`
  - `SuggestedCouponReward`
  - `ApprovedCouponReward`
- Extend selected modifier state beyond the current `SelectedRunModifierKind` + `SelectedRunModifierID`.
- Add source and version to runtime selection state.
- Decide the exact completion semantics for challenge rewards:
  - one-time per player per published version
  - no rewards for drafts
  - no rewards for rejected/unpublished versions
- Define the first allowed modifier primitives:
  - stat bonus
  - starting level override
  - starting item grant
  - passive override
  - ultimate override
  - revive restriction
  - timer rule
  - economy rule
  - chest/shop/flood rule
- Explicitly reject unsupported primitives for v1.

### Acceptance Criteria

- The project has one stable content schema for both mods and challenges.
- The project has one clear answer for how versioning and reward granting work.
- Runtime selection state is no longer too small for community publishing.

## Phase 1: Build The Gameplay Modifier Primitive Layer

### Goal

Create the actual runtime building blocks that make community-authored rules possible.

### Scope

- safe gameplay rule primitives
- rule application at run start
- passive/ultimate override plumbing
- item grant plumbing

### Primary Files

- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/Core/T66GameInstance.h`
- `Source/T66/Core/T66RunSaveGame.h`
- `Source/T66/Gameplay/T66GameMode.cpp`
- related gameplay files for hero passives, ultimates, and start inventory

### Tasks

- Extract the current hardcoded `Mod_MaxHeroStats` behavior into a reusable modifier primitive path.
- Add a data-driven rule application function that can consume a structured payload.
- Support first-wave primitives:
  - additive and multiplicative stat overrides
  - starting level override
  - starting item grant
  - passive override by allowed character ability ID
  - ultimate override by allowed character ability ID
- Ensure all applied rules are reflected in run telemetry and run summaries.
- Preserve explicit unranked behavior when any modifier payload is active.
- Add runtime validation so invalid or unsupported payloads fail safely instead of partially applying.

### Acceptance Criteria

- At least one official challenge and one official mod can be defined through the new rule payload path instead of hardcoded one-off logic.
- The gameplay runtime can apply multiple whitelisted rules deterministically.
- Unsupported payloads are rejected cleanly.

## Phase 2: Build The Shared Catalog Layer

### Goal

Replace hardcoded placeholder lists with a real catalog abstraction that can serve official, community, and draft entries.

### Scope

- local official catalog
- local drafts
- backend-fetched approved community catalog
- in-memory and disk cache

### Primary Files

- `Source/T66/UI/Screens/T66ChallengesScreen.h`
- `Source/T66/UI/Screens/T66ChallengesScreen.cpp`
- new `Source/T66/Core/T66RunModifierCatalogSubsystem.h`
- new `Source/T66/Core/T66RunModifierCatalogSubsystem.cpp`
- optional local JSON/DataTable assets under `Content` or `Config`

### Tasks

- Create a dedicated catalog subsystem for run modifier content.
- Make the subsystem serve:
  - official challenges
  - official mods
  - approved community challenges
  - approved community mods
  - local player drafts
- Define a local storage format for official content and local drafts.
- Add refresh and cache invalidation behavior for backend community content.
- Remove direct placeholder list ownership from `UT66ChallengesScreen`.

### Acceptance Criteria

- The challenges/mods screen no longer owns hardcoded placeholder content arrays as the source of truth.
- The UI can ask for content by `kind` and `source`.
- Drafts, official items, and community items share one view model.

## Phase 3: Rebuild The Challenges And Mods Screen

### Goal

Turn the current modal into the real browser surface for official and community content.

### Scope

- `Official` / `Community` sub-tabs
- contextual create buttons
- list refresh
- selection and detail panel updates

### Primary Files

- `Source/T66/UI/Screens/T66ChallengesScreen.h`
- `Source/T66/UI/Screens/T66ChallengesScreen.cpp`
- `Source/T66/UI/Style/T66Style.cpp`
- shared button/panel primitives if needed

### Tasks

- Keep the top tabs as `Challenges` and `Mods`.
- Add a second tab row under the current header with:
  - `Official`
  - `Community`
- Add contextual actions:
  - `Create Challenge`
  - `Create Mod`
- Bind each tab combination to the new catalog subsystem.
- Add empty, loading, and failed-fetch states for community tabs.
- Update the detail panel to display:
  - source
  - author for community content
  - approved reward for challenges
  - status messaging when viewing a draft

### Acceptance Criteria

- The screen can browse `Challenges > Official`, `Challenges > Community`, `Mods > Official`, and `Mods > Community`.
- The create buttons only appear where appropriate.
- The screen remains playable and readable with current UI styling.

## Phase 4: Build The Local Creator And Draft Editor

### Goal

Let players create, edit, save, and playtest local content before backend submission exists.

### Scope

- creator modal/screen
- draft persistence
- rule builder controls
- local validation

### Primary Files

- new `Source/T66/UI/Screens/T66ModifierCreatorScreen.h`
- new `Source/T66/UI/Screens/T66ModifierCreatorScreen.cpp`
- `Source/T66/UI/T66UIManager.cpp`
- `Source/T66/Core/T66RunModifierCatalogSubsystem.cpp`
- any new supporting UI widgets/components

### Tasks

- Build one shared creator screen with two modes:
  - challenge authoring
  - mod authoring
- Add editable fields:
  - title
  - summary
  - description
  - rule builder
  - suggested reward for challenges only
- Add draft actions:
  - save draft
  - delete draft
  - duplicate draft
  - playtest
- Add local validation and human-readable error messages.
- Add a preview summary that shows the generated rule payload in gameplay terms.

### Acceptance Criteria

- A player can create a valid draft entirely offline.
- A player can reopen and edit a draft later.
- A player can use a draft immediately in a run.

## Phase 5: Integrate Draft Selection, Run Start, And Completion

### Goal

Make locally created content fully playable in the real game flow.

### Scope

- selecting drafts from the browser
- run start rule application
- completion tracking
- reward gating

### Primary Files

- `Source/T66/Core/T66GameInstance.h`
- `Source/T66/Gameplay/T66GameMode.cpp`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/Core/T66RunSaveGame.h`
- `Source/T66/Core/T66LeaderboardSubsystem.cpp`
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`

### Tasks

- Allow draft entries to be selected like official/community entries.
- Ensure run start applies draft or published rule payloads consistently.
- Track whether the selected content is:
  - official
  - community published
  - local draft
- Add completion checks for challenge rules.
- Only grant Chad Coupons if:
  - the content is a published approved challenge version
  - the player has not already claimed that version reward
- Mark all such runs unranked and reflect that clearly in summary UI.

### Acceptance Criteria

- Drafts are playable.
- Approved challenges can award coupons.
- Drafts and unpublished items cannot award coupons.

## Phase 6: Add Backend Data Model And APIs

### Goal

Create the backend persistence and API contract for community content submissions and publishing.

### Scope

- schema
- validation
- player submission routes
- player fetch routes
- version publishing

### Primary Files

- `C:\UE\Backend\src\db\schema.ts`
- new files under `C:\UE\Backend\src\app\api\community-content\...`
- `C:\UE\Backend\src\lib\env.ts`
- new validation in `C:\UE\Backend\src\lib\schemas.ts`

### Tasks

- Add submission and published-version tables, for example:
  - `community_content_submissions`
  - `community_content_versions`
- Add Zod schemas for content payload validation.
- Add player-authenticated routes:
  - submit content
  - update draft metadata if backend drafts are desired later
  - fetch approved community content
  - fetch a player's own submissions
- Store submissions with statuses:
  - `draft`
  - `pending`
  - `approved`
  - `rejected`
  - `needs_changes`
  - `unpublished`
- Add paging and filtering by `kind`.
- Add rate limiting for submissions.

### Acceptance Criteria

- The backend can accept a structured challenge or mod submission from an authenticated player.
- The backend can return published approved content lists to the game.
- The backend preserves moderation history and published version history.

## Phase 7: Extend The Backend Admin Surface

### Goal

Turn the existing admin site into the real operator surface for community approvals and leaderboard actions.

### Scope

- community content review page
- leaderboard moderation actions
- audit logging

### Primary Files

- new `C:\UE\Backend\src\app\admin\community\page.tsx`
- `C:\UE\Backend\src\app\admin\components\AdminNav.tsx`
- `C:\UE\Backend\src\app\api\admin\action\route.ts`
- new detail API routes if needed

### Tasks

- Add `Community` to the admin nav.
- Build a review page showing:
  - kind
  - title
  - author
  - suggested reward
  - status
  - created date
- Add a detailed review panel with:
  - full description
  - rendered rule payload
  - final reward editor
  - review notes
- Add admin actions:
  - `approve_content`
  - `reject_content`
  - `request_changes`
  - `unpublish_content`
- Extend admin actions for leaderboard operations:
  - remove leaderboard entry
  - hide or suppress run summary if required
  - optionally restore suppressed rows later
- Write every action to `audit_log`.

### Acceptance Criteria

- One private admin website can handle community approvals, appeals, reports, and leaderboard moderation.
- Approval and unpublish actions are auditable.
- The public game client only sees approved published community versions.

## Phase 8: Connect The UE Client To Backend Community APIs

### Goal

Hook the in-game browser and creator flows up to the real backend.

### Scope

- HTTP requests
- auth headers
- retry/failure states
- local cache

### Primary Files

- `Source/T66/Core/T66BackendSubsystem.h`
- `Source/T66/Core/T66BackendSubsystem.cpp`
- `Source/T66/Core/T66RunModifierCatalogSubsystem.cpp`
- `Source/T66/UI/Screens/T66ModifierCreatorScreen.cpp`
- `Source/T66/UI/Screens/T66ChallengesScreen.cpp`

### Tasks

- Add backend subsystem methods for:
  - submit community content
  - fetch approved community content
  - fetch my submissions
- Mirror the existing backend integration pattern already used for:
  - bug reports
  - appeals
  - reports
  - leaderboards
- Add client-side request state delegates and callbacks.
- Cache approved community content locally for startup resilience.
- Handle offline mode gracefully:
  - official content still works
  - local drafts still work
  - community submission and refresh show clear failure messaging

### Acceptance Criteria

- The game can submit a new mod/challenge definition to the backend.
- The game can download published community content into the browser tabs.
- Offline behavior remains understandable and safe.

## Phase 9: Add Submission Status UI And My Submissions

### Goal

Give creators visibility into what happened to their submissions.

### Scope

- creator feedback
- status list
- review note display

### Primary Files

- `Source/T66/UI/Screens/T66ModifierCreatorScreen.cpp`
- `Source/T66/UI/Screens/T66ChallengesScreen.cpp`
- optional new `My Submissions` screen/widget

### Tasks

- Add a `My Submissions` view or panel.
- Show creator-visible statuses:
  - draft
  - pending review
  - approved
  - rejected
  - needs changes
  - unpublished
- Show final approved reward on approved challenges.
- Show review notes from the admin action path.

### Acceptance Criteria

- A creator can tell whether their content is still local, pending, approved, or rejected.
- Approved and unpublished states are clearly distinguished.

## Phase 10: Add Reward Claim And Anti-Abuse Safeguards

### Goal

Prevent reward duplication and keep the system aligned with existing ranked and anti-cheat rules.

### Scope

- reward ledger
- completion gating
- telemetry
- moderation hooks

### Primary Files

- `Source/T66/Core/T66ProfileSaveGame.h`
- `Source/T66/Core/T66PlayerExperienceSubSystem.cpp`
- `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h`
- `Source/T66/Core/T66BackendSubsystem.cpp`
- backend schema and API files for reward claim persistence if server-authoritative claims are required

### Tasks

- Add a per-player record of claimed published challenge rewards by content version.
- Add completion proof fields to run summary data.
- Ensure a community challenge reward cannot be claimed twice for the same published version.
- Ensure draft content never grants coupons.
- Ensure rejected or unpublished content never grants coupons.
- Preserve explicit unranked verdicts for all challenge/mod runs.

### Acceptance Criteria

- Reward claims are one-time per player per published challenge version.
- Reward logic is not vulnerable to simple replay or resubmit abuse.
- Run history remains useful even though runs are unranked.

## Phase 11: Validation, QA, And Content Seeding

### Goal

Prove that the system works before public exposure.

### Scope

- local test content
- admin approval smoke test
- reward smoke test
- rollback test

### Primary Files

- `Docs/Systems/T66_Community_Mods_And_Challenges.md`
- this plan
- relevant test accounts and QA notes

### Tasks

- Seed a small official content set through the new runtime model.
- Create QA community samples:
  - one mod
  - one challenge with reward
  - one rejected submission
  - one unpublished submission
- Validate the full lifecycle:
  - create draft
  - play draft
  - submit
  - approve
  - fetch on another client
  - complete approved challenge
  - receive reward
  - verify second completion does not duplicate reward
  - unpublish and confirm disappearance from community browser
- Validate that community runs remain unranked and do not pollute leaderboards.

### Acceptance Criteria

- The end-to-end community content workflow works on real accounts.
- Reward granting and suppression behave correctly.
- Admin moderation actions behave predictably.

## Phase 12: Manual Setup, Migration, And Deployment

### Goal

Perform the manual platform work only after repo and UI work is complete.

### Scope

- environment variables
- database push/migration
- deployment
- admin access
- optional custom domain

### Manual Steps

- In Vercel, ensure the backend project has:
  - `POSTGRES_URL`
  - `KV_REST_API_URL`
  - `KV_REST_API_TOKEN`
  - `STEAM_WEB_API_KEY`
  - `STEAM_APP_ID`
  - `ADMIN_PASSWORD`
  - any new content-specific env vars if later introduced
- Run schema push or migrations for the new community content tables.
- Deploy the backend.
- Verify admin login at:
  - `/admin/login`
- Verify the new admin pages and actions on production or a staging deployment.
- Point the game config to the correct backend URL if it changes.
- Optionally add a custom domain or subdomain later for the admin/backend site.

### Explicit Steamworks Note

No Steam Workshop setup is required for this v1 system.

Do not add Workshop or `ISteamUGC` integration unless the project later decides to support Steam-managed file distribution of real downloadable UGC. That is a separate project.

## Recommended Implementation Order

Build in this order:

1. Phase 0
2. Phase 1
3. Phase 2
4. Phase 3
5. Phase 4
6. Phase 5
7. Phase 6
8. Phase 7
9. Phase 8
10. Phase 9
11. Phase 10
12. Phase 11
13. Phase 12

## First Vertical Slice Recommendation

The first end-to-end slice should be intentionally narrow:

1. One official challenge and one official mod using the new payload system.
2. One community challenge primitive set.
3. Local draft save and playtest.
4. Submit to backend.
5. Approve in admin website.
6. Fetch on another client.
7. Complete it and grant one reward.

If that slice is stable, expand the primitive set and admin tools from there.
