# T66 Anti-Cheat Implementation Checklist

**Last updated:** 2026-04-11  
**Scope:** File-by-file execution checklist for finishing the post-run anti-cheat model built around telemetry capture, `Luck Rating`, `Skill Rating`, and backend adjudication.  
**Companion docs:** `MASTER DOCS/Anti Cheat/MASTER_ANTI_CHEAT.md`, `MASTER DOCS/MASTER_BACKEND.md`

## Goal

Keep runtime overhead light, avoid kernel or file-system inspection, track gameplay evidence during the run, and make the leaderboard verdict once at run submission.

## Phase 1: RNG Centralization

### `C:\UE\T66\Source\T66\Core\T66RngSubsystem.h`

- Add any missing roll helpers needed to remove ad hoc `FMath::FRand` / `RandRange` usage from anti-cheat-relevant systems.
- Keep this subsystem as the canonical source for luck-biased probability and rarity rolls.

### `C:\UE\T66\Source\T66\Core\T66RngSubsystem.cpp`

- Keep run-seeded roll behavior deterministic.
- Add helper coverage only for anti-cheat-relevant gameplay outcomes, not cosmetic randomness.

### `C:\UE\T66\Source\T66\Core\T66GameInstance.h`

- Expose deterministic item-selection helpers that accept a caller-provided `FRandomStream`.
- Keep the Blueprint-callable wrappers for compatibility.

### `C:\UE\T66\Source\T66\Core\T66GameInstance.cpp`

- Stop item selection from defaulting to `FMath::RandRange` when a run stream is available.
- Route loot item selection through either the caller's stream or the shared run stream.

### `C:\UE\T66\Source\T66\Gameplay\T66EnemyDirector.cpp`

- Replace cycle-seeded local RNG for wave-selection decisions with the shared run stream where the result affects spawn probability or composition.
- Keep fallback local RNG only for emergency/no-subsystem paths.

### `C:\UE\T66\Source\T66\Gameplay\T66EnemyBase.cpp`

- Ensure loot bag item rolls use the same deterministic stream as loot bag count and rarity.

### `C:\UE\T66\Source\T66\Gameplay\T66GameMode.cpp`

- Ensure guaranteed-start loot bag rewards use stream-based item selection rather than a separate global random source.
- Audit other stage-generation paths and keep a clear split between:
  - anti-cheat-relevant reward/spawn randomness
  - cosmetic or layout randomness

### `C:\UE\T66\Source\T66\Gameplay\T66StageCatchUpLootInteractable.cpp`

- Ensure catch-up loot item rolls stay on the shared run stream.

### `C:\UE\T66\Source\T66\UI\T66CrateOverlayWidget.cpp`

- Replace cycle-seeded crate reward generation with the run stream for actual reward outcomes.
- Keep purely cosmetic strip behavior deterministic with the same stream until a dedicated visual-only stream is introduced.

### `C:\UE\T66\Source\T66\Gameplay\T66CombatComponent.cpp`

- Audit crit/proc/item-effect randomness.
- Move anti-cheat-relevant combat rolls off raw `FMath::FRand`.
- Leave purely cosmetic combat VFX randomness alone.

### Exit Criteria

- Every loot item selection and luck-relevant reward/probability outcome uses a deterministic stream tied to the run.
- The anti-cheat-relevant spawn/composition paths no longer depend on `FPlatformTime::Cycles()` when the run RNG subsystem exists.

## Phase 2: Luck Telemetry Expansion

### `C:\UE\T66\Source\T66\Data\T66DataTypes.h`

- Add structured anti-cheat event types for reward and probability decisions.

### `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h`

- Add compact accumulators for:
  - roll-family sample counts
  - rarity histograms
  - quantity histograms
  - Bernoulli success/attempt pairs

### `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.cpp`

- Record reward/probability telemetry at the point each roll is finalized.
- Keep payloads compact and machine-readable.
- Current state: category accumulators plus capped `luck_events` and `hit_check_events` are now emitted through the run summary path, and replayable luck events now capture run-stream replay metadata including float replay bounds for rounded wheel-gold rolls.

### `C:\UE\T66\Source\T66\Core\T66RunSaveGame.h`

- Persist the new anti-cheat luck accumulators across mid-run save/load.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardRunSummarySaveGame.h`

- Add serialized fields for the summarized luck telemetry that supports `Luck Rating`.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardSubsystem.cpp`

- Copy the expanded luck evidence into the run summary snapshot.

### `C:\UE\T66\Source\T66\Core\T66BackendSubsystem.cpp`

- Submit the expanded luck evidence inside `anti_cheat_context`.

### `C:\UE\Backend\src\lib\schemas.ts`

- Validate the expanded luck telemetry payload.

### `C:\UE\Backend\src\lib\anti-cheat.ts`

- Replace broad envelope checks with family-specific replayable luck checks.
- Current state: exact replay now exists for the supported replayable luck-event subset, including weighted rarity, Bernoulli chance including spawn checks such as goblin-wave, mini-boss-wave, chest-mimic, and tower special-floor outcomes, biased integer rolls including level-up gains, and rounded float-biased wheel-gold rolls. Broad envelope checks still handle the remaining telemetry.

### `C:\UE\T66\Source\T66\UI\T66GamblerOverlayWidget.cpp`

- Keep gambler minigame outcome RNG on the shared run RNG whenever the result matters for anti-cheat.
- Record per-round gambler telemetry for bet size, payout, cheat attempt/success, and the replayable inputs needed to review the round after submission.
- Current state: coin flip, RPS, lottery, plinko, box opening, and blackjack now submit gambler anti-cheat evidence, and their outcomes now contribute to `Luck Rating`.

### `C:\UE\Backend\src\lib\schemas.ts`

- Validate the gambler telemetry payload under `anti_cheat_context`.
- Current state: the backend now accepts `gambler_summary`, `gambler_events`, and `gambler_events_truncated`.

### `C:\UE\Backend\src\lib\anti-cheat.ts`

- Add dedicated gambler adjudication instead of relying only on generic luck caps.
- Current state: the backend now validates gambler summary/event consistency and can replay coin flip, RPS, lottery, plinko, box-opening, and blackjack rounds from the submitted telemetry.

## Phase 3: Skill and Dodge Telemetry Expansion

### `C:\UE\T66\Source\T66\Core\T66SkillRatingSubsystem.h`

- Redefine `Skill Rating` as a summary of combat plausibility, not only “damage avoided per 5 seconds.”

### `C:\UE\T66\Source\T66\Core\T66SkillRatingSubsystem.cpp`

- Keep the final 0-100 output but compute it from stronger signals.
- Current state: `Skill Rating` now scores 5-second combat-pressure windows using damage taken plus hit-check/dodge performance instead of only counting applied damage.

### `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.h`

- Add counters or histograms for:
  - dodge streak buckets
  - dodge rate by evasion bucket
  - hit-check rate by combat window
  - damage-taken bursts
- Current state: the anti-cheat path now persists fixed evasion buckets plus a compact 5-second pressure-window summary alongside the capped sampled hit-check events.

### `C:\UE\T66\Source\T66\Core\T66RunStateSubsystem.cpp`

- Emit event-level dodge and hit-check evidence with low overhead.
- Current state: hit checks now update both capped sampled events and aggregate evasion-bucket / pressure-window telemetry during the run.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardRunSummarySaveGame.h`

- Serialize the summarized dodge/skill evidence.
- Current state: leaderboard run summaries now carry `AntiCheatEvasionBuckets` and `AntiCheatPressureWindowSummary`.

### `C:\UE\T66\Source\T66\Core\T66BackendSubsystem.cpp`

- Submit the summarized skill/dodge evidence.
- Current state: submissions now include `evasion_buckets` and `pressure_window_summary` inside `anti_cheat_context`.

### `C:\UE\Backend\src\lib\anti-cheat.ts`

- Separate:
  - dodge plausibility
  - skill consistency
  - final skill verdict
- Current state: backend dodge checks now also verify impossible sampled hit-check combinations, full-sample max-dodge-streak consistency, evasion-bucket consistency, and low-evasion bucket anomalies; skill checks now enforce a hard maximum reachable `Skill Rating` based on five-second windows plus pressure-window consistency and a conservative high-pressure suspicion rule.

## Phase 4: Backend Evidence Persistence and Adjudication

### `C:\UE\Backend\src\db\schema.ts`

- Add durable storage for anti-cheat evidence or add JSONB fields to `run_summaries`.
- Current state: `run_summaries` now persist `anti_cheat_context`, backend verdict, flag category, reason, supporting data, and evaluation time for active PB rows and quarantined rows.

### `C:\UE\Backend\src\app\api\submit-run\route.ts`

- Persist submitted anti-cheat evidence for accepted and quarantined runs.
- Persist backend-calculated suspicion metrics for review.
- Current state: accepted PB summaries now store a `clean` verdict with submitted anti-cheat context, and flagged runs persist the evaluated restriction evidence into the quarantined-linked summary row.

### `C:\UE\Backend\src\lib\anti-cheat.ts`

- Split evaluation into:
  - invalid payload checks
  - luck checks
  - dodge checks
  - skill checks
  - final verdict selection

## Phase 5: Co-op Provenance

### `C:\UE\T66\Source\T66\Core\T66BackendSubsystem.cpp`

- Stop cloning the host snapshot for every party member.
- Forward one real per-player telemetry summary per member.
- Current state: the host now consumes per-request member-owned run JSON forwarded from each client, and queues/retries co-op submits until all required member summaries are available.

### `C:\UE\T66\Source\T66\Core\T66SessionSubsystem.cpp`

- Cache co-op member run summaries on the host by backend request key plus SteamID.
- Clear stale cached member summaries when a new run starts or the party session ends.

### `C:\UE\T66\Source\T66\Gameplay\T66PlayerController.cpp`

- Forward non-host finalized run summaries to the host via reliable RPC.
- Bind the stored summary to the sender's replicated session player state rather than trusting arbitrary client-supplied SteamIDs.

### `C:\UE\T66\Source\T66\Core\T66LeaderboardSubsystem.cpp`

- Support building and caching per-player co-op run summaries.

### `C:\UE\Backend\src\app\api\submit-run\route.ts`

- Keep host/member adjudication separate and evidence-linked.

## Phase 6: Visibility and Review

### `C:\UE\Backend\src\app\api\run-summary\[id]\route.ts`

- Apply the same restricted-account visibility rules used by leaderboard endpoints.
- Current state: public run-summary reads now 404 for restricted entry owners and hide restricted co-op member summaries from slot and party-member views.

### `C:\UE\Backend\src\app\api\admin\action\route.ts`

- Keep moderation actions backend-authoritative and evidence-linked.

### Admin UI surface

- Expose backend `supportingData` clearly for review so the final system stays post-run and reviewable instead of invasive.
- Current state: quarantine, appeals, and account lookup now render structured anti-cheat evidence cards instead of relying only on raw JSON dumps.

## Current Priority Order

1. Finish Phase 1 RNG centralization where raw shared-stream draws still bypass wrappers.
2. Decide on durable evidence retention for accepted non-PB runs.
3. Decide whether co-op needs stronger delivery guarantees than the current host retry window.
4. Expand exact replay coverage to any remaining luck-driven event families that still bypass replay metadata.
5. Add broader skill-side signals beyond dodge pressure if needed after threshold tuning.
6. Improve admin review from readable to diagnostic, for example threshold highlights and direct reviewed-run links across all admin lists.
