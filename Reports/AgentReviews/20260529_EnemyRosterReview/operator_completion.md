Operator Completion: Enemy Roster Review

Status: COMPLETE (Operator artifact — NOT a greenlight; Codex validates before user-facing sign-off)
Operator: Claude (T66 Operator)
Task date: 2026-05-29
Approval reference: codex_operator_approval.md (APPROVE)

## Task

Produce a designer-readable enemy roster inventory for Pablo, ahead of a cleanup/deletion Pass D, so forgotten or missing enemies surface before deprecated code is removed. Read-only inspection + report-only writes.

## Files created / changed

Created (within approved write scope):
- `C:\UE\T66\Reports\RosterReview\enemy_roster_review.md` — main designer-facing report (all 9 required sections).
- `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterReview\operator_completion.md` — this artifact.

Directory `Reports\RosterReview\` did not exist and was created (folder creation only).

No source, config, content, save, CSV, uasset, DataTable, runtime, build, stage, or Git changes were made. No cleanup/deletion performed. No Pass D / Pass E work.

## Data sources read (authoritative live repo data)

- `Content\Data\Bosses.csv` — 23 boss rows.
- `Content\Data\Enemies.csv` — 50 basic mobs (10 per theme x 5 themes).
- `Content\Data\Stages.csv` — 20 stages, EnemyA..EnemyJ assignment columns.
- `Content\Data\BossEncounters.csv` — 20 encounters (Stage 17 = MultiBoss "Four Horsemen").
- `Content\Data\BossEncounterMembers.csv` — 23 rows (Stage 17 contributes 4 horsemen members).
- `Content\Data\UniqueEnemies.csv` — 1 row (BackroomsChaser / "Backrooms Stalker").
- Pending issues: `Content\Data\pending_issues_Data.md`, `Source\T66\Data\pending_issues_Data.md`, `Source\T66\Gameplay\pending_issues_Gameplay.md`, `Source\T66\Gameplay\Enemies\pending_issues_Enemies.md`, `Gameplay\Combat\pending_issues_Combat.md`.

## Code areas inspected (read-only)

- `Source\T66\Gameplay\T66EnemyDirector.cpp` — wave spawn logic; Goblin Thief special spawn (Luck-biased, ~1107-1160); random miniboss promotion DISABLED (`MiniBossIndex = INDEX_NONE`, ~1162-1164); family class resolution (`T66ResolveEnemyClassFromFamilyID`, ~206); lightweight-mob routing (`ShouldRouteSpawnToLightweightMob`, ~713).
- `Source\T66\Gameplay\T66GoblinThiefEnemy.h` — gold-stealing special (GoldStolenPerHit=50, rarity-scaled).
- `Source\T66\Gameplay\T66GamblerBoss.cpp` — BossID "GamblerBoss", MaxHP 1000, 6 bespoke hit-zone parts, AttackProfile Gambler.
- `Source\T66\Gameplay\T66PlayerController_Overlays.cpp` — `TriggerCasinoBossIfAngry()` (~5979) spawns `AT66GamblerBoss` (~6048) when `GetCasinoAnger01() >= 1.0`.
- `Source\T66\Gameplay\T66UniqueDebuffEnemy.h` / `T66UniqueDebuffProjectile.h` — floating debuff-firing enemy (default Burn); spawned only in Lab mode.
- `Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp` — placed tower miniboss (MobID "Slime", HPScalar 3.0, DamageScalar 2.0, Scale 1.75, ~15-18); `T66SpawnTowerGateGuardian` (~146); `IsPlacedTowerMinibossFloor` applies to ALL gameplay floors (~554); `EnsurePlacedTowerMinibossForFloor` (~574).
- `Gameplay\Combat\MASTER_COMBAT.md` — references `AT66VendorBoss` (~164, ~378) with no matching source class (stale-doc flag).

Mini/minigame paths were excluded throughout. No broad Git/LFS scans were run.

## Roster counts (traceable)

- Bosses: 23 (Bosses.csv). 20 reachable via BossEncounters.csv; Stage 17 fields 4 of them as the "Four Horsemen" multiboss.
- Hidden bosses: 1 de-facto — the Gambler (code-only, triggered by Casino anger). No formal "hidden boss" data category exists.
- Minibosses: 1 active type — placed tower gate guardian, currently a scaled placeholder "Slime". Random in-wave miniboss promotion exists in code but is disabled.
- Specials: Goblin Thief (active, wave-spawned), Gambler (active but condition-gated), Unique Debuff enemy (latent — Lab mode only), Backrooms Stalker (latent — UniqueEnemies.csv, Backrooms disabled).
- Basic mobs: 50 (Enemies.csv).

## Validation notes & known confidence limits

- HIGH confidence: data-defined counts (bosses 23, mobs 50, encounters 20, members 23) — direct row reads from named CSVs.
- MEDIUM/code-traced: specials, hidden boss, and miniboss live entirely in C++, not data. Counts/status here depend on the inspected files; any spawn path in unread modules could add cases. Mini/minigame paths were intentionally not inspected.
- The Gambler is reported as the hidden boss with an explicit caveat; no data category confirms "hidden" status — this is an interpretation of code-gated reachability.
- `AT66VendorBoss` flagged as referenced-in-docs but missing-in-source. Not confirmed whether it was renamed, removed, or never built — flag only.
- Unique Debuff enemy and Backrooms Stalker flagged as latent/unreachable in normal play (Lab-only / Backrooms disabled per pending issues).
- Production archetype classes (Exploder / Stutterer / Burrower) flagged as not yet built per `pending_issues_Enemies.md`; data references them but code does not implement them.
- "Floors 2/3/4" framing in the task prompt does not match code: `IsPlacedTowerMinibossFloor` applies to all gameplay floors. Flagged for Pablo.
- All 50 mobs have StatusEffectOnHit=None — flagged (consistent with `pending_issues_Data.md`).
- All gap flags are phrased as review flags for Pablo's decision, not as fixes performed.

## Token routing metadata

- Operator (Claude): full inspection + both report writes this session. Direct-read tool surface only; no mutating commands issued.
- Validator (Codex): not yet run. This artifact awaits Codex validation per OPERATOR_VALIDATOR_PROTOCOL.md before any user-facing greenlight.
- Codex tokens spent this task: 0 (validation pending).
