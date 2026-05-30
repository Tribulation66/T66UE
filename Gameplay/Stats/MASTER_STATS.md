# T66 Master Stats

**Last updated:** 2026-05-29
**Scope:** Single-source handoff for the T66 stat system: authored data, live runtime ownership, primary and secondary formulas, item and buff stacking, stat UI, persistence, and current deprecated or inert stat paths.
**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Gameplay/Combat/MASTER_COMBAT.md`, `Gameplay/Movement/MASTER_MOVEMENT.md`
**Maintenance rule:** Update this file after every material change to hero stat schema, hero level curves, item stat rules, buff progression, stat UI, run-summary stat snapshots, or secondary-stat activation/deprecation.

## May 2026 Status: Secondary-Only Items And Active Leveling

- Items only apply their secondary stat line. Item primary lines remain authored for compatibility/presentation but no longer increase primary stats or proxy secondary gains.
- In-run hero XP and level-up progression are active again.
- `HeroLevel`, `HeroXP`, `XPToNextLevel`, saved precise hero stats, and persistent secondary gain entries are live saved-run state.
- New runs start at level `1`, XP `0`, and the data-driven flat XP threshold from `PlayerExperience` (`100` by current data).
- Enemy XP is data-driven through `Enemies.csv` / `FT66EnemyData::XPValue`; both rich enemies and lightweight mobs grant XP when killed by the hero.
- A level-up heals HP to full, rolls hero per-level primary gains from `Heroes.csv`, propagates those gains into secondary stats, and fires a non-boss OHKO wave at the data-driven radius (`900` UU by current data).
- Level-up wave kills do not award XP, so wave kills cannot recursively chain into another level-up.
- Permanent diploma bonuses apply as primary stat bonuses and then propagate into secondary stats.
- Selected single-use drug bonuses apply as secondary multipliers at run start, but drug purchases remain disabled/unpurchasable.

## 1. Executive Summary

- `UT66RunStateSubsystem` is the live stat authority.
- Base stat authoring comes from `Content/Data/Heroes.csv` / `DT_Heroes`.
- Item stat authoring comes from `Content/Data/Items.csv` / `DT_Items`; live item effects are secondary-only.
- Permanent diploma progression comes from `UT66BuffSubsystem` and adds flat points to primary stats:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
  - `Speed`
- The foundational primary stat schema is:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
  - `Speed`
- `HeroStats` mirrors the selected hero's active primary stats for compatibility and snapshots.
- Effective live primary getters add:
  - hero base
  - in-run level-up primary gains
  - permanent diploma primary gains
- `Speed` levels and affects movement directly; it currently has no primary-to-secondary proxy group.
- Live normal item templates present and apply only one stat effect:
  - secondary stat line = flat secondary stat points
- Matching item secondary bonuses stack additively in `ItemSecondaryStatBonusTenths`.
- Secondary stats start from hero-authored base values, then combine item secondary bonuses, persistent level-up secondary bonuses, permanent diploma secondary bonuses, selected drug multipliers, and primary-derived multipliers.
- `Accuracy` is now a full primary stat. The Accuracy family is:
  - `CritChance`
  - `CritDamage`
  - `AttackRange`
  - `Execute`
- The current default stats panel shows `Level` plus 7 primaries:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
- `Speed` exists and is persisted, but it is hidden from the default panel and mainly feeds movement.
- Deprecated secondary stats are intentionally retained in enums, save parsing, backend parsing, and some hero/item data paths for compatibility, but several of them are no longer live.
- Run summary and backend payloads persist all 8 primaries, including `Speed`, plus all current live secondary values.

## 2. Canonical Files

- `Content/Data/Heroes.csv`
- `Content/Data/Items.csv`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/Core/T66GameInstance.h`
- `Source/T66/Core/T66GameInstance.cpp`
- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/T66RunStateSubsystem.cpp`
- `Source/T66/Core/T66BuffSaveGame.h`
- `Source/T66/Core/T66BuffSubsystem.h`
- `Source/T66/Core/T66BuffSubsystem.cpp`
- `Source/T66/UI/T66StatsPanelSlate.h`
- `Source/T66/UI/T66StatsPanelSlate.cpp`
- `Source/T66/Core/T66LocalizationSubsystem.cpp`
- `Source/T66/Core/T66LeaderboardRunSummarySaveGame.h`
- `Source/T66/Core/T66LeaderboardSubsystem.cpp`
- `Source/T66/Core/T66BackendSubsystem.cpp`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`

## 3. Authored Data Model

### 3.1 Heroes data

- `Heroes.csv` authors two different Accuracy concepts:
  - `BaseAccuracyStat` / `LvlAccuracyMin` / `LvlAccuracyMax` for the primary stat point track
  - `BaseAccuracy` for the secondary head-targeting baseline
- `Heroes.csv` also authors:
  - the other foundational primaries
  - decimal per-level gain ranges for all 8 primaries, including `LvlSpeedMin` / `LvlSpeedMax`
  - category-specific base stats for `Pierce`, `Bounce`, `AOE`, and `DOT`
  - secondary baselines such as `BaseCritDamage`, `BaseCritChance`, `BaseAttackRange`, `BaseTaunt`, `BaseReflectDmg`, `BaseCrushChance`, `BaseInvisChance`, `BaseCounterAttack`, `BaseAssassinateChance`, `BaseCheatChance`, and `BaseStealChance`
- The selected hero row is loaded through `UT66GameInstance::GetHeroStatTuning()` and `UT66GameInstance::GetHeroData()`.

### 3.2 Items data

- `Items.csv` defines one row per item template.
- Each item row has:
  - `PrimaryStatType`
  - `SecondaryStatType`
  - `BaseBuyGold`
  - `BaseSellGold`
  - rarity-specific icons
- `Item_BackroomsQuickRevive` is a reward-only inventory item. It is intentionally excluded from random item pools, shop stock, buyback/sell value, alchemy, lab unlock progression, and stat aggregation.
- Normal lethal damage consumes the Backrooms Quick Revive item once, restores one heart, and removes the item from inventory.
- Runtime still normalizes item rows through `T66ResolveEffectivePrimaryStatType(...)` for compatibility and presentation.
- Accuracy-family secondaries resolve under primary `Accuracy`.
- `Item_Accuracy` is retired; `Item_Execute` is the live fourth Accuracy-family item.

### 3.3 Buff progression data

- `UT66BuffSubsystem` persists permanent and single-use stat progression in `UT66BuffSaveGame`.
- Permanent diploma buffs track all 8 primaries:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
  - `Speed`
- Each unlocked fill step and random overflow unit contributes `+1` primary stat point.
- Single-use drug buffs are selected secondary multipliers. Purchases are currently disabled, but already-owned/selected drugs are wired to activate at run start.

## 4. Runtime Ownership And Fresh-Run Flow

- `AT66GameMode` starts a fresh run by calling:
  - `RunState->ResetForNewRun()`
  - `RunState->ActivatePendingSingleUseBuffsForRunStart()`
- `ResetForNewRun()` clears run inventory, clears active dot state, clears single-use secondary multipliers, resets score and timers, and zeroes item-derived accumulators.
- `ResetForNewRun()` then:
  - initializes `HeroLevel = 1`, `HeroXP = 0`, and `XPToNextLevel` from `PlayerExperience`
  - ignores deprecated difficulty start bonus levels and community direct-stat overrides
  - reloads the selected hero's primary and secondary baselines
  - seeds the run's hero-stat RNG for deterministic level-up rolls
- `ActivatePendingSingleUseBuffsForRunStart()` consumes selected owned drug buffs and applies their secondary multipliers for the run.
- The authoritative live stat state therefore lives in:
  - selected hero base primary stats
  - selected hero base secondary stats
  - hero secondary baselines
  - `HeroPreciseStats`
  - `PersistentSecondaryStatBonusTenths`
  - `PermanentBuffStatBonuses`
  - `PermanentSecondaryStatBonusTenths`
  - `ItemSecondaryStatBonusTenths`
  - `SingleUseSecondaryMultipliers`

## 5. Foundational Primary Stats

### 5.1 Effective primary getters

- Primary getters return `HeroPreciseStats + PermanentBuffStatBonuses`, clamped to the stat range.
- Items do not affect primary getters.
- Level-up directly increases `HeroPreciseStats`.
- Diplomas add through `PermanentBuffStatBonuses`.

### 5.2 Primary-derived multipliers

- `GetHeroDamageMultiplier() = 1.0 + (Damage - 1) * 0.015`
- `GetHeroAttackSpeedMultiplier() = 1.0 + (AttackSpeed - 1) * 0.012`
- `GetHeroScaleMultiplier() = 1.0 + (AttackScale - 1) * 0.008`
- `GetHeroAccuracyMultiplier() = 1.0 + (Accuracy - 1) * 0.010`
- Live walking speed uses raw `GetSpeedStat() * 840 UU/s`; see `Gameplay/Movement/MASTER_MOVEMENT.md`

### 5.3 Defensive totals

- `GetArmorReduction01()` uses:
  - base armor reduction from primary `Armor`
  - bonus reduction from secondary `DamageReduction`
  - `ItemArmorBonus01`, which currently stays at `0`
- Formula:
  - `BaseArmorReduction = clamp((Armor - 1) * 0.008, 0.0, 0.80)`
  - `ArmorReduction01 = clamp(BaseArmorReduction + DamageReductionBonus + ItemArmorBonus01, 0.0, 0.80)`
- `GetEvasionChance01()` uses:
  - base dodge chance from primary `Evasion`
  - bonus dodge from secondary `EvasionChance`
  - `ItemEvasionBonus01`, which currently stays at `0`
- Formula:
  - `BaseEvasionChance = clamp((Evasion - 1) * 0.006, 0.0, 0.60)`
  - `EvasionChance01 = clamp(BaseEvasionChance + EvasionChanceBonus + ItemEvasionBonus01, 0.0, 0.60)`

### 5.4 Accuracy head-targeting chance

- `GetAccuracyChance01()` is not the same thing as primary `AccuracyStat`.
- Formula:
  - `AccuracyChance01 = clamp(GetSecondaryStatValue(Accuracy) + PassiveBonus, 0.0, 0.95)`
- Current passive hook:
  - `Headshot` adds `+0.20`

## 6. Active Leveling

- `DefaultHeroLevel = 1`
- `DefaultXPToLevel = 100`
- `PlayerExperience` owns the active XP threshold and level-up wave radius per difficulty:
  - `LevelUpXPThreshold`
  - `LevelUpWaveRadiusUU`
- The current XP curve is flat: every level uses the current difficulty's `LevelUpXPThreshold`.
- `AddHeroXP(...)` accumulates XP, applies as many level-ups as the XP total supports, and preserves leftover XP.
- Each level-up:
  - increments `HeroLevel`
  - heals `CurrentHP` to `MaxHP`
  - rolls primary stat gains from the selected hero's `Lvl*Min` / `Lvl*Max` ranges
  - adds those primary gains to `HeroPreciseStats`
  - distributes proxy secondary gains through `ApplyPrimaryGainToSecondaryBonuses(...)`
  - runs a non-boss OHKO wave in `LevelUpWaveRadiusUU`
- Level-up wave XP is suppressed while the wave is resolving.
- Safe defaults still exist if hero data fails to load:
  - all base primaries default to `2`
  - primary gain ranges default to low decimal bands

## 7. Secondary Stat Model

### 7.1 Core rule

- Secondary stats are computed from hero-authored baselines, not from zero.
- For any secondary type, runtime starts with:
  - the hero's base secondary value from `Heroes.csv`
  - plus item, level-up, and diploma secondary bonus points when applicable
  - multiplied by selected drug multipliers when applicable
- Let:
  - `BonusPoints = item secondary points + persistent level-up secondary points + permanent diploma secondary points`
  - `M = active run multipliers for this secondary, including selected drugs`

### 7.2 Damage family

- `AoeDamage = BaseAoeDmg * M * HeroDamageMultiplier`
- `BounceDamage = BaseBounceDmg * M * HeroDamageMultiplier`
- `PierceDamage = BasePierceDmg * M * HeroDamageMultiplier`
- `DotDamage = BaseDotDmg * M * HeroDamageMultiplier`

### 7.3 Attack Speed family

- `AoeSpeed = BaseAoeAtkSpd * M * HeroAttackSpeedMultiplier`
- `BounceSpeed = BaseBounceAtkSpd * M * HeroAttackSpeedMultiplier`
- `PierceSpeed = BasePierceAtkSpd * M * HeroAttackSpeedMultiplier`
- `DotSpeed = BaseDotAtkSpd * M * HeroAttackSpeedMultiplier`

### 7.4 Attack Scale family

- `AoeScale = BaseAoeAtkScale * M * HeroScaleMultiplier`
- `BounceScale = BaseBounceAtkScale * M * HeroScaleMultiplier`
- `PierceScale = BasePierceAtkScale * M * HeroScaleMultiplier`
- `DotScale = BaseDotAtkScale * M * HeroScaleMultiplier`

### 7.5 Accuracy family

- `CritChance = clamp((HeroBaseCritChance + BonusPoints * 0.01) * M * HeroAccuracyMultiplier, 0.0, 1.0)`
- `CritDamage = max(1.0, (HeroBaseCritDamage + BonusPoints * 0.05) * M * HeroAccuracyMultiplier)`
- `AttackRange = max(100.0, (HeroBaseAttackRange + BonusPoints * 25.0) * M * HeroAccuracyMultiplier)`
- `Execute = clamp(BonusPoints * 0.005 * M, 0.0, 1.0)`

### 7.6 Armor family

- `Taunt = HeroBaseTaunt * M`
- `ReflectDamage = HeroBaseReflectDmg * M`
- `Crush = clamp(HeroBaseCrushChance * M, 0.0, 1.0)`
- `DamageReduction = BaseArmorReduction * max(0.0, M - 1.0)`

### 7.7 Evasion family

- `EvasionChance = BaseEvasionChance * max(0.0, M - 1.0)`
- `CounterAttack = HeroBaseCounterAttack * M`
- `Invisibility = clamp(HeroBaseInvisChance * M, 0.0, 1.0)`
- `Assassinate = clamp(HeroBaseAssassinateChance * M, 0.0, 1.0)`

### 7.8 Luck family

- `TreasureChest = 1.0 * M`
- `Cheating = clamp(HeroBaseCheatChance * M, 0.0, 1.0)`
- `Stealing = clamp(HeroBaseStealChance * M, 0.0, 1.0)`
- `LootCrate = 1.0 * M`

## 8. Item Stat Rules

### 8.1 Line 1 and Line 2

- Line 1 is a flat additive primary-stat roll stored per inventory slot as `Line1RolledValue`.
- Line 2 is a multiplicative scalar stored per inventory slot as:
  - the default rarity multiplier
  - or `Line2MultiplierOverride` when an item instance overrides the default

### 8.2 Rarity rules

- Line-1 primary roll range by rarity:
  - `Black: 1..3`
  - `Red: 4..6`
  - `Yellow: 7..10`
  - `White: 20..30`
- Line-2 secondary multiplier by rarity:
  - `Black: 1.1x`
  - `Red: 1.2x`
  - `Yellow: 1.5x`
  - `White: 2.0x`

### 8.3 Recompute path

- `RecomputeItemDerivedStats()` clears all item-derived accumulators and rebuilds them from `InventorySlots`.
- It skips:
  - invalid slots
  - reward-only special items
  - Vendor Token compatibility rows
  - any item row whose primary is `Special`
- It then:
  - adds only the item's secondary stat bonus into `ItemSecondaryStatBonusTenths`

### 8.4 Important current caveats

- Item line-1 primary rolls are retained in inventory slots for compatibility and display plumbing but do not affect runtime stats.
- The legacy `ET66ItemEffectType` enum and the old item-percent fields in `UT66RunStateSubsystem` are currently not populated by live item recompute.
- Current live item power is therefore secondary-only.

### 8.5 Accuracy item normalization

- Accuracy-family items are normalized to primary `Accuracy` by runtime code.
- `Item_Accuracy` is retired.
- `Item_Execute` is explicitly kept in fallback stock and random pools.

## 9. Buff Progression Rules

### 9.1 Permanent buffs

- `UT66BuffSubsystem::MaxFillStepsPerStat = 4`
- `UT66BuffSubsystem::PermanentBuffUnlockCostCC = 10`
- Each unlocked fill step contributes `+1` flat primary stat point.
- Overflow above the visible steps is stored in the `RandomBonus*` fields and still counts toward the stat's total.
- Permanent diploma primary bonuses are then propagated into secondary bonus points by the run state.
- Diploma purchases remain disabled/unpurchasable while this path is wired.

### 9.2 Single-use buffs

- `UT66BuffSubsystem::SingleUseBuffCostCC = 1`
- `UT66BuffSubsystem::MaxSelectedSingleUseBuffs = 4`
- `UT66BuffSubsystem::SingleUseSecondaryBuffMultiplier = 1.10`
- A selected secondary with `N` owned-and-consumed copies gets:
  - `1.10 ^ N`
- Single-use buffs are consumed when the run starts and stored in `SingleUseSecondaryMultipliers`.
- Drug purchases remain disabled/unpurchasable while this path is wired.
- Runtime only exposes live secondaries through `GetAllSingleUseBuffTypes()`, so deprecated secondaries cannot be selected into the modern loadout.

## 10. Live, Deprecated, And Inert Secondary Stats

### 10.1 Live item-facing secondaries

- Damage: `AoeDamage`, `BounceDamage`, `PierceDamage`, `DotDamage`
- Attack Speed: `AoeSpeed`, `BounceSpeed`, `PierceSpeed`, `DotSpeed`
- Attack Scale: `AoeScale`, `BounceScale`, `PierceScale`, `DotScale`
- Accuracy: `CritDamage`, `CritChance`, `AttackRange`, `Accuracy`
- Armor: `Taunt`, `DamageReduction`, `ReflectDamage`, `Crush`
- Evasion: `EvasionChance`, `CounterAttack`, `Invisibility`, `Assassinate`
- Luck: `TreasureChest`, `Cheating`, `Stealing`, `LootCrate`

### 10.2 Deprecated or inert secondary enums retained for compatibility

- `Goblin`
- `Leprechaun`
- `Fountain`
- `CloseRangeDamage`
- `LongRangeDamage`
- `SpinWheel`
- `MovementSpeed`
- `VendorToken` / legacy `GamblerToken` enum alias
- `HpRegen`
- `LifeSteal`
- `Alchemy`

### 10.3 Current runtime behavior of those deprecated or inert paths

- `HpRegen` returns `0.0`
- `LifeSteal` returns `0.0`
- `Alchemy` returns `0.0`
- `CloseRangeDamage` multiplier returns `1.0`
- `LongRangeDamage` multiplier returns `1.0`
- `MovementSpeed` secondary multiplier returns `1.0`
- `Goblin`, `Leprechaun`, `Fountain`, and `SpinWheel` remain in enum and persistence paths but are not live item-facing stats
- `VendorToken` is handled as a separate run upgrade through `ActiveGamblersTokenLevel` (field name retained for save compatibility), not as part of normal item stat recompute

### 10.4 Hero data columns that still exist but are no longer live stat paths

- `BaseHpRegen` is loaded as `0.0`
- `BaseLifeSteal` is loaded as `0.0`
- `BaseCloseRangeDmg` is currently ignored by the active secondary getter path
- `BaseLongRangeDmg` is currently ignored by the active secondary getter path

## 11. UI And Player-Facing Presentation

- `T66StatsPanelSlate` is the shared builder for:
  - pause menu
  - vendor overlay
  - gambler overlay
  - hero selection
  - run summary
- The current default panel shows:
  - `Level`
  - `Damage`
  - `Attack Speed`
  - `Attack Scale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
- `Speed` is not in the default panel.
- The extended panel groups secondaries under those same 7 primary categories.
- The extended panel also shows derived lines for:
  - `Total Damage Reduction`
  - `Total Dodge Chance`
- Display formatting is presentation-oriented:
  - `Level` shows as a raw integer
  - primary and secondary values render as `/100` style text in the stats panel
  - percent-style secondaries are converted to a percentage number first, then shown as `/100`
- Important maintenance note:
  - `T66StatsPanelSlate.h` comments still say "Level + 6 displayed stats"
  - runtime code is authoritative and currently shows 7 displayed primaries because `Accuracy` was added

## 12. Confirmed Runtime Consumers

- This section only lists consumers confirmed in the current C++ source.
- A stat can still be schema-valid, UI-visible, and persisted even if it is not listed below as a confirmed direct C++ consumer.

### 12.1 Combat

- `UT66CombatComponent` uses:
  - `GetHeroDamageMultiplier()`
  - `GetHeroAttackSpeedMultiplier()`
  - `GetHeroScaleMultiplier()`
  - `GetCritChance01()`
  - `GetCritDamageMultiplier()`
  - `GetAccuracyChance01()`
  - `GetCloseRangeThreshold()`
  - `GetLongRangeThreshold()`
- Untargeted auto-attacks use `GetAccuracyChance01()` to prefer `Head` hit zones on enemies and bosses that expose combat hit zones.
- `CritChance` and `CritDamage` are live and confirmed in the combat hit path.
- `Invisibility` is live and can proc confusion on hit.
- `Taunt` is live through `GetAggroMultiplier()`.

### 12.2 Incoming damage on the hero

- `UT66RunStateSubsystem::ApplyDamage(...)` currently resolves damage in this order:
  - saint blessing immunity
  - `IronWill` passive flat reduction using `ArmorStat * 2`
  - `Unflinching` passive `15%` reduction
  - full-hit dodge from `EvasionChance01`
  - percent reduction from `ArmorReduction01`
  - reflect and crush checks
  - HP loss through the shared damage path
  - normal lethal damage consumes `Item_BackroomsQuickRevive` once if owned; Backrooms chaser contact bypasses Quick Revive
- `Assassinate` and `CounterAttack` are live on successful dodge.
- Important caveat:
  - old tooltip language says Armor subtracts flat damage
  - current runtime code uses percent reduction from `GetArmorReduction01()` after the dodge step
  - flat damage reduction currently belongs to the `IronWill` passive, not base Armor itself

### 12.3 Movement

- `UT66HeroMovementComponent` multiplies:
  - raw primary `Speed` converted at `840 UU/s` per Speed point
  - `GetItemMoveSpeedMultiplier()`
  - `GetStageMoveSpeedMultiplier()`
  - `GetStatusMoveSpeedMultiplier()`
- Current caveat:
  - primary `Speed` is now the base live hero walking-speed stat
  - `HeroData.MaxSpeed` is reserved metadata for future cap semantics and is not currently part of live walking speed
  - the item move-speed multiplier path is still at its default `1.0`
  - current live movement scaling comes from `Speed` plus explicit item, stage, and status effects

### 12.4 Range caveat

- `AttackRange` secondary is computed in `RunStateSubsystem`.
- Confirmed current C++ consumers of that secondary are:
  - close-range threshold
  - long-range threshold
  - enemy spawn spacing in `T66EnemyDirector`
- Important current mismatch:
  - `UT66CombatComponent` still builds the actual attack radius from hero base attack range and total `AttackScale`
  - the live attack radius does not currently read `GetSecondaryStatValue(AttackRange)`
- When this file conflicts with older tooltip text, the code path above wins.

## 13. Persistence, Run Summary, And Backend

- `UT66LeaderboardSubsystem::CreateCurrentRunSummarySnapshot()` stores:
  - `HeroLevel`
  - all 8 primaries, including `Speed`
  - `SecondaryStatValues` for every live secondary stat
- `HeroLevel` is retained as a compatibility/backend field and should be written as `1`.
- Current local run-summary schema is `15`.
- `UT66BackendSubsystem` serializes those stats into:
  - `stats`
  - `secondary_stats`
- Primary backend keys are:
  - `damage`
  - `attack_speed`
  - `attack_scale`
  - `accuracy`
  - `armor`
  - `evasion`
  - `luck`
  - `speed`
- Backend parsing still recognizes deprecated secondary keys as well as live ones so older payloads and saved summaries can deserialize safely.

## 14. Source-Of-Truth Rules

- Current C++ runtime behavior wins when old tooltips, stale comments, or historical docs disagree.
- The main authoritative owner is `UT66RunStateSubsystem`.
- The main authoring sources are `Heroes.csv` and `Items.csv`.
- The main buff-progression owner is `UT66BuffSubsystem`.
- The main player-facing stats UI owner is `T66StatsPanelSlate`.
- The main persistence owners are:
  - `UT66LeaderboardSubsystem` for local run-summary snapshots
  - `UT66BackendSubsystem` for JSON serialization and parsing
