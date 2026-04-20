# T66 Master Stats

**Last updated:** 2026-04-18  
**Scope:** Single-source handoff for the T66 stat system: authored data, live runtime ownership, primary and secondary formulas, item and buff stacking, stat UI, persistence, and current deprecated or inert stat paths.  
**Companion docs:** `MASTER DOCS/T66_MASTER_GUIDELINES.md`, `MASTER DOCS/MASTER_COMBAT.md`, `MASTER DOCS/MASTER_MOVEMENT.md`, `MASTER DOCS/T66_PROJECT_CATALOGUE.md`  
**Maintenance rule:** Update this file after every material change to hero stat schema, hero level curves, item stat rules, buff progression, stat UI, run-summary stat snapshots, or secondary-stat activation/deprecation.

## 1. Executive Summary

- `UT66RunStateSubsystem` is the live stat authority.
- Base stat authoring comes from `Content/Data/Heroes.csv` / `DT_Heroes`.
- Item stat authoring comes from `Content/Data/Items.csv` / `DT_Items`, with one synthetic runtime fallback for `Item_Accuracy`.
- Permanent buff progression comes from `UT66BuffSubsystem` and adds flat points to the 7 non-Speed primaries:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
- The foundational primary stat schema is:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
  - `Speed`
- `HeroStats` stores the selected hero's base primary stats plus all level-up gains.
- Effective live primary getters add:
  - hero base + level-ups
  - item line-1 flat bonuses
  - permanent buff flat bonuses
- `Speed` is currently the exception:
  - it levels normally
  - it affects movement normally
  - it is not increased by normal item line-1 application
  - it is not part of permanent buff progression
  - it is not shown on the default stats panel
- Live item templates use a two-line model:
  - line 1 = flat additive primary stat bonus
  - line 2 = multiplicative secondary stat scalar
- Matching secondary stats stack multiplicatively, not additively.
- Secondary stats are not item-only values. They start from hero-authored base values, then item and single-use buff multipliers scale those bases.
- `Accuracy` is now a full primary stat. The Accuracy family is:
  - `CritDamage`
  - `CritChance`
  - `AttackRange`
  - `Accuracy`
- Any item row in the Accuracy family is normalized to primary `Accuracy` by runtime code, even if older data was authored differently.
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
- Runtime normalizes item rows through `T66ResolveEffectivePrimaryStatType(...)`.
- Accuracy-family secondaries always resolve the effective primary to `Accuracy`.
- `UT66GameInstance::BuildSyntheticSpecialItemData()` can synthesize `Item_Accuracy` if that row is missing from the DataTable.

### 3.3 Buff progression data

- `UT66BuffSubsystem` persists permanent and single-use stat progression in `UT66BuffSaveGame`.
- Permanent buffs track only 7 primaries:
  - `Damage`
  - `AttackSpeed`
  - `AttackScale`
  - `Accuracy`
  - `Armor`
  - `Evasion`
  - `Luck`
- There is no permanent `Speed` track.
- Single-use buffs track selected secondary stat multipliers for the next run.

## 4. Runtime Ownership And Fresh-Run Flow

- `AT66GameMode` starts a fresh run by calling:
  - `RunState->ResetForNewRun()`
  - `RunState->ActivatePendingSingleUseBuffsForRunStart()`
- `ResetForNewRun()` clears run inventory, clears active dot state, clears single-use secondary multipliers, resets score and timers, and zeroes all item-derived accumulators.
- `ResetForNewRun()` then:
  - sets `HeroLevel = 1`
  - applies any difficulty start bonus levels from `UT66PlayerExperienceSubSystem`
  - reloads the selected hero's primary and secondary baselines
  - seeds the run's hero-stat RNG
  - replays level-up rolls if the run starts above level 1
  - refreshes permanent buff flat bonuses from `UT66BuffSubsystem`
- `ActivatePendingSingleUseBuffsForRunStart()` consumes the selected single-use buff loadout and turns it into per-secondary multipliers for this run.
- The authoritative live stat state therefore lives in:
  - `HeroStats`
  - `HeroPerLevelGains`
  - hero secondary baselines
  - `ItemStatBonuses`
  - `SecondaryMultipliers`
  - `PermanentBuffStatBonuses`
  - `SingleUseSecondaryMultipliers`

## 5. Foundational Primary Stats

### 5.1 Effective primary getters

- `GetDamageStat() = HeroStats.Damage + ItemStatBonuses.Damage + PermanentBuffStatBonuses.Damage`
- `GetAttackSpeedStat() = HeroStats.AttackSpeed + ItemStatBonuses.AttackSpeed + PermanentBuffStatBonuses.AttackSpeed`
- `GetScaleStat() = HeroStats.AttackScale + ItemStatBonuses.AttackScale + PermanentBuffStatBonuses.AttackScale`
- `GetAccuracyStat() = HeroStats.Accuracy + ItemStatBonuses.Accuracy + PermanentBuffStatBonuses.Accuracy`
- `GetArmorStat() = HeroStats.Armor + ItemStatBonuses.Armor + PermanentBuffStatBonuses.Armor`
- `GetEvasionStat() = HeroStats.Evasion + ItemStatBonuses.Evasion + PermanentBuffStatBonuses.Evasion`
- `GetLuckStat() = HeroStats.Luck + ItemStatBonuses.Luck + PermanentBuffStatBonuses.Luck`
- `GetSpeedStat() = HeroStats.Speed`

### 5.2 Primary-derived multipliers

- `GetHeroDamageMultiplier() = 1.0 + (Damage - 1) * 0.015`
- `GetHeroAttackSpeedMultiplier() = 1.0 + (AttackSpeed - 1) * 0.012`
- `GetHeroScaleMultiplier() = 1.0 + (AttackScale - 1) * 0.008`
- `GetHeroAccuracyMultiplier() = 1.0 + (Accuracy - 1) * 0.010`
- `GetHeroMoveSpeedMultiplier() = 1.0 + (Speed - 1) * 0.010`

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

## 6. Leveling And Starting Levels

- `DefaultHeroLevel = 1`
- `DefaultXPToLevel = 100`
- XP per level is currently fixed at `100`.
- On level-up:
  - `Damage`, `AttackSpeed`, `AttackScale`, `Accuracy`, `Armor`, `Evasion`, `Luck`, and `Speed` all roll their selected hero's decimal inclusive min/max gain ranges
  - `Damage` is authored uniformly at `0.5-1.0` across heroes in the current first pass
  - the other primaries are currently seeded from each hero's base proficiency values in `Heroes.csv`
- Luck bias is applied through `UT66RngSubsystem::BiasHigh01(...)`, so Luck can push rolls toward the top of each range without changing the authored min/max values.
- If a run starts above level 1 because of difficulty bonuses, `InitializeHeroStatsForNewRun()` replays those level-up rolls immediately so the run begins with a fully materialized stat block.
- Safe defaults still exist if hero data fails to load:
  - all base primaries default to `2`
  - `Damage` defaults to `0.5..1.0`
  - other primary gain ranges default to low decimal bands and runtime fallback ranges

## 7. Secondary Stat Model

### 7.1 Core rule

- Secondary stats are computed from hero-authored baselines, not from zero.
- For any secondary type, runtime starts with:
  - the hero's base secondary value from `Heroes.csv`
  - multiplied by matching item line-2 multipliers
  - multiplied by matching single-use buff multipliers for the run
- Let:
  - `M = product of item multipliers for this secondary * product of selected single-use multipliers for this secondary`

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

- `CritDamage = HeroBaseCritDamage * M * HeroAccuracyMultiplier`
- `CritChance = clamp(HeroBaseCritChance * M * HeroAccuracyMultiplier, 0.0, 1.0)`
- `AttackRange = HeroBaseAttackRange * M * HeroAccuracyMultiplier`
- `Accuracy = clamp(HeroBaseAccuracy * M * HeroAccuracyMultiplier, 0.0, 1.0)`

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
  - `Item_GamblersToken`
  - any item row whose secondary is `GamblerToken`
- It then:
  - adds line-1 flat stat bonuses through `AddPrimaryBonus(...)`
  - multiplies secondary multipliers into `SecondaryMultipliers`

### 8.4 Important current caveats

- `AddPrimaryBonus(...)` intentionally ignores `Speed`.
- There are no authored `Speed` rows in `Items.csv`, so live item line-1 application currently affects only the other 7 primaries.
- The legacy `ET66ItemEffectType` enum and the old item-percent fields in `UT66RunStateSubsystem` are currently not populated by live item recompute.
- Current live item power is therefore the two-line model only:
  - flat primary line 1
  - multiplicative secondary line 2

### 8.5 Accuracy item normalization

- Accuracy-family items are normalized to primary `Accuracy` by runtime code.
- `Item_Accuracy` is treated specially:
  - it can be synthesized if the DataTable row is missing
  - vendor pool and random-item pool code explicitly ensure it can still appear

## 9. Buff Progression Rules

### 9.1 Permanent buffs

- `UT66BuffSubsystem::MaxFillStepsPerStat = 10`
- `UT66BuffSubsystem::PermanentBuffUnlockCostCC = 10`
- Each unlocked fill step contributes `+1` flat primary stat point.
- Overflow above the visible 10 steps is stored in the `RandomBonus*` fields and still counts toward the stat's total.
- Permanent buffs do not apply to:
  - `Speed`
  - any secondary stat

### 9.2 Single-use buffs

- `UT66BuffSubsystem::SingleUseBuffCostCC = 1`
- `UT66BuffSubsystem::MaxSelectedSingleUseBuffs = 4`
- `UT66BuffSubsystem::SingleUseSecondaryBuffMultiplier = 1.10`
- A selected secondary with `N` owned-and-consumed copies gets:
  - `1.10 ^ N`
- Single-use buffs are consumed when the run starts.
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
- `GamblerToken`
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
- `GamblerToken` is handled as a separate run upgrade through `ActiveGamblersTokenLevel`, not as part of normal item stat recompute

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
  - last-stand immunity
  - quick-revive special handling
  - `IronWill` passive flat reduction using `ArmorStat * 2`
  - `Unflinching` passive `15%` reduction
  - full-hit dodge from `EvasionChance01`
  - percent reduction from `ArmorReduction01`
  - reflect and crush checks
- `Assassinate` and `CounterAttack` are live on successful dodge.
- Important caveat:
  - old tooltip language says Armor subtracts flat damage
  - current runtime code uses percent reduction from `GetArmorReduction01()` after the dodge step
  - flat damage reduction currently belongs to the `IronWill` passive, not base Armor itself

### 12.3 Movement

- `UT66HeroMovementComponent` multiplies:
  - `GetHeroMoveSpeedMultiplier()`
  - `GetItemMoveSpeedMultiplier()`
  - `GetMovementSpeedSecondaryMultiplier()`
  - `GetLastStandMoveSpeedMultiplier()`
  - `GetStageMoveSpeedMultiplier()`
  - `GetStatusMoveSpeedMultiplier()`
- Current caveat:
  - the item move-speed multiplier path is still at its default `1.0`
  - the secondary move-speed path is still hardcoded to `1.0`
  - current live movement scaling therefore mainly comes from primary `Speed`, last stand, stage effects, and status effects

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
