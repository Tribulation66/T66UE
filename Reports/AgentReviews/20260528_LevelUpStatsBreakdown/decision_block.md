# Decision Block - Level-Up And Stat Source Implementation

## Goal

Implement secondary-only items and re-enabled in-run level-up primary-stat progression, while reporting the full live stat-source breakdown for items, level-up, diplomas, and drugs.

## Claude Review Status

Review artifact:

`Reports/AgentReviews/20260528_LevelUpStatsBreakdown/20260528T214858-pass1/claude_review_pass1.md`

Verdict:

`Verdict: NEEDS_HUMAN_DECISION`

## Current Confirmed Wiring

- Items are not secondary-only yet. They still add primary stat points and secondary stat points.
- Level-up is not active. `AddHeroXP()` and `ApplyOneHeroLevelUp()` are no-ops, and enemy deaths do not award XP.
- Hero rows already contain per-level stat gain weights/ranges in `Content/Data/Heroes.csv`.
- The primary-to-secondary propagation helper already exists as `UT66RunStateSubsystem::ApplyPrimaryGainToSecondaryBonuses(...)`.
- Diplomas are persisted/displayable but not purchasable and do not affect runtime stats.
- Drugs are persisted/selectable, but purchases are disabled and selected multipliers are not applied to runtime secondary stat values.

## Decisions Needed

1. XP threshold/curve:
   - Example options: flat `100 XP per level`, `100 + 25 per level`, or another curve.

2. Level-up wave radius:
   - Example options: `900 UU`, `1200 UU`, `current attack range`, or another value.

3. Lightweight mob XP scope:
   - Rich enemies (`AT66EnemyBase`) already have `XPValue`.
   - Lightweight mobs (`AT66MobBase`) do not currently have an XP value.
   - Decide whether this pass grants XP from rich enemies only, or adds XP to lightweight mobs too.

4. Diplomas and drugs scope:
   - The user asked for the breakdown and wiring status.
   - Decide whether this implementation pass should also re-enable diploma and drug runtime stat effects, or leave them report-only for now.

## Recommended Default If User Says No Clarification Needed

- Items: implement secondary-only now.
- Level-up XP: flat `100 XP per level`, using existing rich enemy `XPValue` default of 20, so default enemies produce one level every five kills.
- Wave radius: `1200 UU`.
- XP source: rich `AT66EnemyBase` now, and add a simple XP value to `AT66MobBase` only if the active spawn path relies on lightweight mobs for normal enemy kills.
- Diplomas/drugs: do not re-enable purchases/effects in this pass unless the user explicitly says to include them; report the current broken/inert wiring instead.

