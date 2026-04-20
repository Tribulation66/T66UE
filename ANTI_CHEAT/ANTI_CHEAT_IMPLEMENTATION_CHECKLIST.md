# T66 Anti-Cheat Implementation Checklist

**Last updated:** 2026-04-18  
**Scope:** File-by-file execution checklist for the ranked-integrity anti-cheat model: backend authority, pristine-build attestation, compact provenance validation, and reviewable verdicts.  
**Companion docs:** `ANTI_CHEAT/MASTER_ANTI_CHEAT.md`, `MASTER DOCS/MASTER_BACKEND.md`

## Goal

Keep runtime overhead light, avoid invasive scanning, make ranked eligibility backend-authoritative, and add enough integrity and provenance evidence that common mod/cheat paths cannot quietly enter the ranked leaderboard.

## Phase 1: Ranked Submission Policy Plumbing

### `C:\UE\T66\Source\T66\Core\T66GameInstance.h`

- Keep run-modifier selection and ranked ineligibility explicit in persistent run state.
- Preserve the selected challenge/mod choice separately from backend-ranked acceptance state.

### `C:\UE\T66\Source\T66\Gameplay\T66GameMode.cpp`

- Freeze ranked eligibility for fresh runs at run start.
- Preserve the same verdict when importing/resuming a saved run instead of letting resumed runs accidentally regain ranked eligibility.

### `C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp`

- Keep challenge and internal mod selection as expected unranked choices.
- Do not let UI selection state silently diverge from the run-start verdict.

### `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h`

- Persist the ranked-eligibility state needed to keep resumed runs consistent with the run-start policy.

### `C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.cpp`

- Stop treating local PB updates as the primary submit result.
- Save recent-run history locally if desired, but wait for backend `accepted` before ranked PB/rank changes commit.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardSubsystem.h` / `C:\UE\T66\Source\T66\Core\T66LeaderboardSubsystem.cpp`

- Split recent-run snapshot storage from ranked PB/rank storage.
- Treat challenge/mod/integrity-mismatch runs as expected `unranked` outcomes instead of generic submission failures.

### `C:\UE\T66\Source\T66\Core\T66BackendSubsystem.cpp`

- Handle the new backend verdict vocabulary cleanly:
  - `accepted`
  - `unranked`
  - `flagged`
  - `suspended`
  - `banned`

### Exit Criteria

- Challenge and mod runs never create ranked PB/rank state.
- Local ranked PB/rank state advances only after backend `accepted`.
- Resumed runs cannot silently shed their original ranked-ineligible status.

## Phase 2: Integrity Attestation Baseline

### `C:\UE\T66\Source\T66\Core\T66IntegritySubsystem.h` / `C:\UE\T66\Source\T66\Core\T66IntegritySubsystem.cpp` (new)

- Add a dedicated subsystem for ranked integrity snapshots instead of overloading unrelated systems.
- Capture a baseline at launch or immediately before run start.
- Maintain cheap dirty bits for known ranked-sensitive runtime changes.
- Recompute the final snapshot at run end for comparison.

### `C:\UE\T66\Source\T66\Core\T66SteamHelper.cpp`

- Surface the Steam app/build identity needed for ranked integrity context.
- If Workshop or other Steam UGC is added later, expose only the mounted/subscribed state needed for ranked policy; do not move authority to Steam.

### `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h`

- Persist enough integrity state to keep resumed runs tied to the same baseline and dirty-bit verdict.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardRunSummarySaveGame.h`

- Serialize the compact integrity summary:
  - `steam_app_id`
  - `steam_build_id`
  - `beta_name`
  - `manifest_id`
  - `manifest_root_hash`
  - `module_list_hash`
  - `mounted_content_hash`
  - `modded`
  - `mod_reasons`

### `C:\UE\T66\Source\T66\Core\T66LeaderboardSubsystem.cpp`

- Copy the integrity verdict into the submission snapshot and local recent-run history.

### `C:\UE\T66\Source\T66\Core\T66BackendSubsystem.cpp`

- Submit `integrity_context` alongside `anti_cheat_context`.

### `C:\UE\Backend\src\lib\schemas.ts`

- Validate the new `integrity_context` payload.

### `C:\UE\Backend\src\app\api\submit-run\route.ts`

- Reject ranked acceptance when integrity does not match the allowlisted pristine build policy.
- Return `unranked` for intentional non-ranked integrity outcomes rather than `flagged`.

### Exit Criteria

- Every ranked submission carries a compact integrity context.
- Backend acceptance depends on matching an allowlisted pristine-build policy.
- No per-tick file hashing or other heavy runtime scanning is introduced.

## Phase 3: Compact Provenance Validation

### `C:\UE\T66\Source\T66\Data\T66DataTypes.h`

- Add compact provenance structs or enums for ranked-relevant mutation events.

### `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h`

- Add counters and compact ledgers for:
  - spawn baseline
  - item gains/losses
  - gold gains/spends
  - stat deltas
  - progression transitions
  - ranked-relevant reward sources

### `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.cpp`

- Record provenance only at mutation points already owned by the gameplay code.
- Keep the data compact and machine-checkable.

### `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h`

- Persist provenance state across mid-run save/load so resumed runs stay reviewable.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardRunSummarySaveGame.h`

- Serialize the finalized provenance summary needed for backend validation.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardSubsystem.cpp`

- Copy the provenance evidence into the final run-summary snapshot.

### `C:\UE\T66\Source\T66\Core\T66BackendSubsystem.cpp`

- Submit the compact provenance summary inside a dedicated context object or a clearly named anti-cheat subfield.

### `C:\UE\Backend\src\lib\schemas.ts`

- Validate the provenance payload.

### `C:\UE\Backend\src\lib\anti-cheat.ts`

- Validate compact invariants such as:
  - ending inventory equals legal additions minus removals
  - ending gold equals gains minus spends
  - ending stats reconcile with allowed sources
  - progression follows legal transitions

### Exit Criteria

- Ranked-relevant inventory, currency, stat, and progression changes can be reconciled from the submitted summary.
- The provenance layer remains event-driven and does not add frame-by-frame replay cost.

## Phase 4: Backend Verdicts and Persistence

### `C:\UE\Backend\src\db\schema.ts`

- Expand persistence so accepted, unranked, and flagged runs all keep the evidence needed for later review.
- Keep `leaderboard_entries` reserved for ranked-accepted runs only.

### `C:\UE\Backend\src\app\api\submit-run\route.ts`

- Separate intentional `unranked` outcomes from suspicious `flagged` outcomes.
- Keep host/member adjudication separate in co-op.
- Only write ranked PB rows when the verdict is `accepted`.

### `C:\UE\Backend\src\lib\anti-cheat.ts`

- Split evaluation into:
  - invalid payload checks
  - integrity checks
  - provenance checks
  - luck checks
  - dodge checks
  - skill checks
  - final verdict selection

### `C:\UE\Backend\src\app\api\account-status\route.ts`

- Keep restriction reporting backend-authoritative.
- Do not treat ordinary `unranked` runs as account restrictions.

### `C:\UE\Backend\src\app\api\leaderboard\route.ts`
### `C:\UE\Backend\src\app\api\leaderboard\friends\route.ts`
### `C:\UE\Backend\src\app\api\my-rank\route.ts`

- Continue filtering restricted accounts.
- Consume only ranked-backed leaderboard data, not local-only client state.

### Exit Criteria

- Backend verdicts cleanly distinguish unranked policy cases from suspicious cheating cases.
- Ranked rows exist only for backend-accepted runs.

## Phase 5: Co-op and Review Surfaces

### `C:\UE\T66\Source\T66\Core\T66SessionSubsystem.cpp`

- Keep per-member run evidence separate.
- Extend the member summary cache to include integrity and provenance context.

### `C:\UE\T66\Source\T66\Gameplay\T66PlayerController.cpp`

- Continue forwarding member-owned finalized run summaries to the host.
- Preserve the sender identity binding and avoid arbitrary client-owned identity fields.

### `C:\UE\Backend\src\app\api\admin\action\route.ts`

- Keep moderation actions evidence-linked and backend-authoritative.

### `C:\UE\Backend\src\app\admin\quarantine\page.tsx`
### `C:\UE\Backend\src\app\admin\appeals\page.tsx`
### `C:\UE\Backend\src\app\admin\accounts\page.tsx`

- Surface integrity and provenance summaries next to the existing luck and dodge evidence.
- Keep admin review post-run and evidence-oriented rather than invasive.

### Exit Criteria

- Co-op submissions carry member-owned integrity/provenance evidence.
- Review surfaces expose the new evidence layers without requiring raw JSON digging.

## Current Priority Order

1. Finish the ranked-policy plumbing so local ranked PB/rank state is backend-accepted only.
2. Add the integrity attestation baseline and run-end compare.
3. Extend backend verdicts to support `unranked` cleanly.
4. Add compact provenance validation for inventory, stats, currency, and progression.
5. Expand admin evidence surfaces after the new contexts exist.
