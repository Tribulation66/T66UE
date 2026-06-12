# Headshot Chance Stat Replacement - Review Packet Pass 2

Working goal: Replace Crit Damage with Headshot Chance across the live T66 item/stat pipeline, make critical hits always deal fixed 2x damage, wire Headshot Chance as a new stun-on-hit secondary stat, update data/docs/UI labels, and verify the runtime and staged build paths.

Review history:
- Pass 1 returned `Verdict: REVISE`.
- Codex-owned corrections in this pass:
  - Make Headshot chance-per-bonus-point data-driven, not hardcoded.
  - Anchor the formula terms and C++ seams to owning files.
  - Add explicit save/backend parser smoke coverage for old `CritDamage` key migration.
  - Make the old-drug migration assumption explicit.

Operator/validator:
- Operator: Codex
- Validator: Claude through `Scripts\Invoke-ClaudePlanReview.ps1`

User request:
- "Crit Damage" should no longer exist as a live item/stat.
- Critical damage should always be 2x damage.
- "Headshot Chance" replaces Crit Damage.
- Headshot is a new stat/effect that stuns the enemy.

Safe assumptions:
- This is main-run gameplay scope only. Do not inspect or edit Mini/minigame implementation.
- Existing `ET66PassiveType::Headshot`, spatial head hit-zone targeting, and Headshot floating-text color remain intact; the new stat is a separate secondary named `HeadshotChance` with player-facing label "Headshot Chance".
- Because existing saves/data may carry old enum/string keys, `CritDamage` may remain as a deprecated compatibility enum/parser path, but it must not be live-facing, item-facing, drug-selectable, or used for crit damage math.
- Replacing a stat means old Crit Damage single-use drug slots should migrate positionally to Headshot Chance rather than being cleared/refunded. This keeps owned player progress useful and is consistent with the user's "replace Crit Damage" wording.
- Headshot tuning is data-driven in `FT66PlayerExperienceDifficultyTuning`:
  - `HeadshotChancePerBonusPoint`, initial value `0.005`
  - `HeadshotStunDurationSeconds`, initial value `0.75`, matching the existing `Dazed` status duration in `Content/Data/StatusEffects.csv`
- Existing hero-authored `BaseCritDamage` values are not meaningful as chance values. Rename the column to `BaseHeadshotChance` and set current rows to `0.0` so designers can retune per-hero base chance later. This avoids converting old multiplier values such as `1.5` or `1.8` into invalid chance baselines.

Applicable repo instructions:
- Root `AGENTS.md`: live repo first, Claude review default, folder instruction discovery, staged standalone verification for playable runtime changes.
- `Gameplay/GAMEPLAY_AGENTS.md`: prefer data-authored tuning over hardcoded C++ defaults; runtime gameplay changes need compile/build and staged standalone validation.
- `Gameplay/Stats/MASTER_STATS.md`: update after material stat schema/item/buff/UI/persistence changes.
- `Gameplay/Combat/MASTER_COMBAT.md`: update after material combat/damage-model/stat changes; preserve current hit-zone/headshot targeting model.
- `UI/UI_AGENTS.md`: UI text/stat panel changes need build-level verification, but this is not reference-fidelity UI work.
- `Scripts/README.md`: use data-table reload helpers after CSV/schema changes.
- Pending issues read: `Content/Data/pending_issues_Data.md`, `Source/T66/Data/pending_issues_Data.md`, `Source/T66/Core/pending_issues_Core.md`, `Source/T66/Core/RunState/pending_issues_RunState.md`, `Source/T66/Gameplay/pending_issues_Gameplay.md`, `Source/T66/UI/pending_issues_UI.md`.

Anchored live seams:
- Data/schema:
  - `Content/Data/Items.csv`: row `Item_CritDamage`.
  - `Content/Data/Heroes.csv`: header/value column `BaseCritDamage`.
  - `Content/Data/PlayerExperience.json`: active level-up tuning already lives here; add headshot tuning fields per difficulty.
  - `Source/T66/Data/T66DataTypes.h`: `FHeroData::BaseCritDamage`, `ET66SecondaryStatType::CritDamage`, `T66IsDeprecatedSecondaryStatType`, `T66IsAccuracyFamilySecondaryStatType`.
- PlayerExperience tuning:
  - `Source/T66/Core/T66PlayerExperienceSubSystem.h`: `FT66PlayerExperienceDifficultyTuning`.
  - `Source/T66/Core/T66PlayerExperienceSubSystem.cpp`: add getters for headshot chance scaling and stun duration.
- Run-state stat authority:
  - `Source/T66/Core/T66RunStateSubsystem.h`: existing `GetCritDamageMultiplier()`, `HeroBaseCritDamage`; add `GetHeadshotChance01()`, headshot tuning getters, and `HeroBaseHeadshotChance`.
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`: existing hero secondary load, `GetSecondaryStatValue(...)`, `GetSecondaryStatBaselineValue(...)`, `GetCritDamageMultiplier()`.
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h`: existing Accuracy secondary group and retired item IDs.
- Combat:
  - `Source/T66/Gameplay/T66CombatComponent.cpp`: auto-attack `ResolveCrit` currently multiplies damage by `CachedRunState->GetCritDamageMultiplier()` around the crit-resolution lambda; existing local stun helper shape exists later for idol procs, but auto-attack needs a small local shared target-stun lambda in the auto-attack scope.
  - Existing target stun APIs: `AT66EnemyBase::ApplyStun`, `AT66MobBase::ApplyStun`, `AT66BossBase::ApplyStun`.
- UI/localization:
  - `Source/T66/UI/T66StatsPanelSlate.cpp`: `CatAccuracy` group and percent classification.
  - `Source/T66/UI/Screens/T66PowerUpScreen.cpp`: drug slugs/names/Accuracy row.
  - `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Stats.cpp`: hero preview secondary snapshot.
  - `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h`: drug names.
  - `Source/T66/UI/T66TemporaryBuffUIUtils.cpp`: buff slug.
  - `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`: extended stat lines.
  - `Source/T66/Core/T66LocalizationSubsystem.cpp`: stat names/tooltips/item variants/base names.
- Persistence/backend:
  - `Source/T66/Core/T66LeaderboardSubsystem.cpp`: secondary snapshot enum loop currently ends at `VendorToken`; update for appended `HeadshotChance`.
  - `Source/T66/Core/Backend/T66BackendRunSerializer.cpp`: add `HeadshotChance` key for new snapshots.
  - `Source/T66/Core/Backend/T66BackendRunSummaryParser.cpp`: map new `HeadshotChance` and old `CritDamage` keys to live `HeadshotChance`.
- Buffs:
  - `Source/T66/Core/T66BuffSubsystem.cpp`: replace `CritDamage` with `HeadshotChance` in `GSingleUseBuffStats` at the same array position.

Implementation plan:
1. Schema/data
   - Add appended `ET66SecondaryStatType::HeadshotChance` to preserve existing enum values.
   - Mark `CritDamage` deprecated/inert in `T66IsDeprecatedSecondaryStatType`.
   - Change Accuracy family ordering to `CritChance`, `HeadshotChance`, `AttackRange`, `Execute`.
   - Replace `Item_CritDamage` with `Item_Headshot` in `Content/Data/Items.csv`, primary `Accuracy`, secondary `HeadshotChance`.
   - Add `Item_CritDamage` to retired/removed item ID handling so old inventories do not keep live crit-damage items.
   - Rename `FHeroData::BaseCritDamage` to `BaseHeadshotChance` and update `Content/Data/Heroes.csv` header accordingly.
   - Convert current hero row values in that column to `0.0`.
   - Add `HeadshotChancePerBonusPoint` and `HeadshotStunDurationSeconds` to `FT66PlayerExperienceDifficultyTuning` and `Content/Data/PlayerExperience.json`.

2. Runtime stat math
   - Keep `GetCritDamageMultiplier()` as a compatibility method but make it return fixed `2.0f`.
   - Add `GetHeadshotChance01()`.
   - Add `GetDataDrivenHeadshotChancePerBonusPoint()` and `GetDataDrivenHeadshotStunDurationSeconds()`.
   - Formula source anchors:
     - `BonusPoints` comes from existing `GetSecondaryStatBonusValue(StatType)` in `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`; this already aggregates item secondary points, level-up secondary bonuses, and diploma secondary bonuses.
     - `M` comes from the existing `SecondaryMultipliers` and `SingleUseSecondaryMultipliers` multiplication inside `GetSecondaryStatValue(...)`; this is the same path used by selected drugs.
     - `AccuracyMult` comes from existing `GetHeroAccuracyMultiplier()`.
     - `ChancePerPoint` comes from `PlayerExperience` tuning.
   - Live formula:
     - `HeadshotChance = clamp((HeroBaseHeadshotChance + BonusPoints * ChancePerPoint) * M * HeroAccuracyMultiplier, 0.0, 1.0)`
   - Keep old `CritDamage` secondary getter/baseline inert/fixed for compatibility only; it should not appear in live stat lists.

3. Combat behavior
   - In auto-attack resolved damage, crits always multiply by fixed `2.0`.
   - On each resolved auto-attack hit, roll `GetHeadshotChance01()`.
   - On success, apply stun to rich enemies, lightweight mobs, and bosses using existing `ApplyStun` APIs and `GetDataDrivenHeadshotStunDurationSeconds()`.
   - Show existing `Headshot` status/event text when the stun applies.
   - Preserve Execute behavior as "chance on critical hit" after crits.

4. UI/localization/buffs/persistence
   - Replace Crit Damage labels/tooltips/item-name variants/drug slugs/drug names/stat panel grouping with Headshot Chance.
   - Update run summary to show Headshot Chance instead of Crit Damage.
   - Replace `CritDamage` in `UT66BuffSubsystem` single-use drug stat list with `HeadshotChance` at the same array position so existing owned/selected Crit Damage drug slots migrate to Headshot Chance.
   - Backend serializer emits `HeadshotChance` for new snapshots.
   - Backend parser maps both `HeadshotChance` and old `CritDamage` keys to the live `HeadshotChance` stat.
   - Snapshot secondary loop includes appended `HeadshotChance` rather than stopping at `VendorToken`.

5. Docs and smokes
   - Update `Gameplay/Stats/MASTER_STATS.md` and `Gameplay/Combat/MASTER_COMBAT.md`.
   - Extend existing `-T66StatPipelineSmoke` or add focused checks to prove:
     - `Item_CritDamage` is retired/skipped.
     - `Item_Headshot` exists and is live under Accuracy.
     - `CritDamage` is not live-facing.
     - `GetCritDamageMultiplier()` returns `2.0`.
     - Headshot Chance increases from the item/drug path.
     - A forced headshot chance stuns a target.
     - Old backend/save key `CritDamage` parses into live `HeadshotChance`.
     - The appended enum is included in live secondary snapshots.

Out of scope:
- Mini/minigame stat systems.
- New headshot VFX/audio/art assets.
- Renaming the existing `ET66PassiveType::Headshot`.
- Removing old enum/string compatibility paths that are needed for save/backend deserialization.
- New item icon art; the new row may reuse existing icon paths until a separate art pass.

Risks:
- Renaming a hero CSV field requires `DT_Heroes` reload and compile validation because the generated row struct must match CSV headers.
- Appending `HeadshotChance` means loops that formerly used `VendorToken` as the last enum must be updated.
- There is an existing unrelated dirty file: `Source/T66/UI/Screens/T66MinigamesScreen.cpp`; do not touch it.

Verification plan:
- Confirm `ANTHROPIC_API_KEY` is not set before Claude review.
- Run Claude review and require first line `Verdict: APPROVE` before edits.
- After edits:
  - `UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=Scripts\SetupItemsDataTable.py`
  - `UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=Scripts\ImportHeroDataTable.py`
  - `UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script=Scripts\SetupPlayerExperienceDataTable.py`
  - focused `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - editor smoke for stat pipeline and item taxonomy
  - staged standalone refresh through `Scripts\StageStandaloneBuild.ps1`
  - staged smoke for the changed stat path
  - shortcut target verification for `T66 Standalone.lnk`
