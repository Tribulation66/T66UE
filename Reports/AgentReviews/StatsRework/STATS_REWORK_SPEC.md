# Stats Rework — Implementation Spec

Task slug: `StatsRework`. Owner doc; source of truth for the multi-session change.
Created 2026-06-09. Status: **Phase 0 complete (spec locked), Phase 1 in progress.**

## 1. Locked decisions (from user, 2026-06-09)

1. **Single tier of stats.** Remove the primary-stat tier as a player concept. Full collapse — the 8 primaries are removed from the math, not just hidden.
2. **Every stat has a hidden backend base value** (e.g. Move Speed's real UU/s, base AOE damage, base dodge %). The player never sees the raw base.
3. **All upgrade sources apply `+%`:** items, relics, *and* level-ups. No source grants flat stat points anymore.
4. **Effective stat = `base × (1 + Σ% / 100)`**, where Σ% is the sum of every % bonus to that stat (items + relics + level-ups + temporary amplifiers + Saint blessing).
5. **Relics stay permanent.** Only their *mechanism* changes: a relic upgrade now bumps the named stat(s) directly by a %, instead of granting a primary-stat bonus that fans out.
6. **Rename** `secondary stat` → `stat` everywhere player-facing; purge "primary stat" from UI/strings.
7. **Hero select:** numeric stat readouts replaced by **"Best stats"** + an **authored top-3 per hero**.

## 2. The 8 old primaries → new stat mapping

The primary tier did 3 jobs (global multiplier, direct effect, auto-feeder). Mapping each into the unified stat list:

| Old primary | Old role | New stat | New base value source |
|---|---|---|---|
| Damage | ×mult on all category damage | **All Damage** (umbrella) | base = 1.0 (neutral) |
| AttackSpeed | ×mult on all category speed | **All Attack Speed** (umbrella) | base = 1.0 |
| AttackScale | ×mult on all category scale | **All Scale** (umbrella) | base = 1.0 |
| Accuracy | ×mult on crit/range/accuracy family | **Accuracy** (existing enum, un-deprecate) | base = 1.0 |
| Armor | →damage reduction % | **Damage Reduction** (existing) | base % converted from `FHeroData.BaseArmor` |
| Evasion | →dodge chance % | **Dodge** (`EvasionChance`, existing) | base % converted from `FHeroData.BaseEvasion` |
| Luck | →loot luck modifier | **Luck** (NEW single entry; collapses the 4 luck sub-stats) | base converted from `FHeroData.BaseLuck` |
| Speed | →MaxWalkSpeed | **Move Speed** (`MovementSpeed`, existing, un-deprecate) | base UU/s converted from `FHeroData.BaseSpeed` |

Conversion of the 1–99 primary bases into the new stat bases is a one-time deterministic mapping in code (Phase 1), so **no hero-data re-authoring is required** to preserve current balance.

## 3. Final stat list (enum end-state, `ET66SecondaryStatType`)

Renamed conceptually to "stat types" (enum identifier kept to avoid a churn rename of the type itself — display/comments updated; see §7).

**Live (37):**
- Umbrella offense (4): **All Damage, All Attack Speed, All Scale, Accuracy**
- Per-category offense (12): {AOE, Bounce, Pierce, DOT} × {Damage, Speed, Scale}
- Precision (4): Crit Chance, Headshot Chance, Attack Range, Execute
- Defense (4): Damage Reduction, Reflect, Taunt, Crush
- Evasion (4): Dodge, Counter, Invisibility, Assassinate
- Elemental (5): Fire / Ice / Electricity / Nature / Wind Power
- Utility (3): Move Speed, Luck, HP Regen

**Enum edits:**
- APPEND new entries at the end (save-value stability): `AllDamage`, `AllAttackSpeed`, `AllScale`, `Luck`.
- Un-deprecate (remove from `T66IsDeprecatedSecondaryStatType`): `MovementSpeed`, `Accuracy`, `HpRegen`.
- Keep deprecated/hidden, route effects through new stats: `InteractableLuck`, `StealingLuck`, `GamblingLuck`, `ProcLuck` (→ folded into `Luck`); `LifeSteal` stays deprecated unless requested.
- `ET66HeroStatType` retained ONLY where still needed for item Line-1 data compat (`FItemData.PrimaryStatType`); removed from all live stat math.

## 4. Open assumptions (proceeding on these — flag if wrong)

- **A1. RESOLVED (user 2026-06-09): chance stats use additive percentage points.** "+20% Dodge" on a 5% base = 25%. Magnitude stats (per-category damage/speed/scale, the umbrellas, Move Speed, Luck) stay multiplicative `base × (1 + Σ%/100)`. Two formulas: points for chances, multiply for magnitudes.
- **A2. Umbrella + per-category coexist** (confirmed): an AOE hero benefits from both All Damage and AOE Damage.
- **A3. Luck collapses** the 4 sub-stats into one Luck (confirmed).
- **A4. HP Regen included** as a live stat (was optional).
- **A5. Item Line-1 (`PrimaryStatType`) data** is migrated to target a stat in the unified list; existing item rows remapped via a compat table rather than re-authored.

## 5. Phased change list

### Phase 1 — Core model (C++) — IN PROGRESS
- `Data/T66DataTypes.h`: enum appends + un-deprecate list; comments/display rename.
- `Core/RunState/T66RunStateSubsystem_Private.h`: delete `T66_GetSecondaryTypesForPrimary` + the per-primary group helpers + `T66_GetHeroMainAttackSecondaryType` (feeder gone).
- `Core/RunState/T66RunStateSubsystem_Stats.cpp`: rewrite stat resolution to `base × (1+Σ%)`; the 4 `GetHeroXMultiplier()` read umbrella stats; Armor/Evasion/Luck/Speed read their new stats; delete `ApplyPrimaryGainToSecondaryBonuses`, `GetPrecisePrimaryStatTenths` primary plumbing, `RollHeroPrimaryGainTenthsBiased` feeder use; convert `FHeroData` primary bases → new stat bases on init.
- `Core/T66RunStateSubsystem.h`: retire/repoint `GetDamageStat/GetSpeedStat/...` primary getters.
- Player movement: repoint `MaxWalkSpeed` to the Move Speed stat's effective value.

### Phase 2 — Relics & level-ups
- `Core/T66BuffSubsystem.{h,cpp}` + `FT66RelicDefinition`: add `{StatType, PercentAmount}` target(s); `RefreshPermanentBuffBonusesFromProfile` applies % directly.
- `ApplyOneHeroLevelUp`: grant `+%` per level into the hero's stats on a per-hero weighting (reuse `Lvl*Min/Max` intent), replacing the primary→secondary roll.

### Phase 3 — UI
- Hero select (`T66HeroSelectionScreen_Stats.cpp`): "Best stats" + authored top-3; drop the 8-primary numeric columns.
- `T66StatsPanelSlate`, `T66PowerUpScreen`, `T66TooltipResolvers`, `T66ItemCardTextUtils`: show `+%`; rename strings; localization (`T66LocalizationSubsystem`).

### Phase 4 — Save / serialization compat
- `T66LeaderboardRunSummarySaveGame.h` (DamageStat…SpeedStat + SecondaryStatValues), `T66RunSaveGame.h`, `T66BuffSaveGame.h`, snapshot/serializer: migrate dropped primary fields; relic data migration.

### Phase 5 — Verify
- Compile (editor target). Then `Scripts/StageStandaloneBuild.ps1`. Hero-select frontend check via Unreal-owned capture.

## 6. Authored "Best stats" table (to fill with user)

Pre-fill proposed top-3 per hero from each hero's highest base values, then user edits. (Populated in Phase 3.)

## 6b. Progress log

**2026-06-09 — Phase 1 combat core landed** (`T66DataTypes.h`, `T66RunStateSubsystem_Stats.cpp`):
- Enum: appended All Damage / All Attack Speed / All Scale / Luck; un-deprecated Move Speed, Accuracy, HP Regen.
- `T66MapPrimaryToUnifiedStat` maps 6 primaries → unified stats (Damage→AllDamage, AttackSpeed→AllAttackSpeed, AttackScale→AllScale, Armor→DamageReduction, Evasion→Dodge, Luck→Luck).
- Multiplier getters (`GetHeroDamage/AttackSpeed/ScaleMultiplier`) now: `innate-base-excl-permanent-buff × (1 + umbrella%)`.
- 12 magnitude cases: `MagnitudeValue` = `base × (1 + accumulated%) × M × categoryMult`.
- `GetArmorReduction01` / `GetEvasionChance01` / `GetTotalLuckModifierPercent`: innate base excludes permanent-buff; upgrades add via the unified stat (percent points for chances).
- Level-up (`ApplyOneHeroLevelUp`) and relics (`RefreshPermanentBuffBonusesFromProfile`) route the 6 mapped stats directly into the unified accumulation; **no random fan-out** for them.
- Double-count avoided by excluding `GetPermanentPrimaryBuffTenths` from every migrated reader (relic power now lives only in the unified accumulation).

**Deferred (Phase 1b):**
- **Speed → Move Speed** and **Accuracy**: still on the legacy primary path. Their effective-value readers live outside the subsystem (`T66HeroMovementComponent` for Speed; `GetHeroAccuracyMultiplier`/`GetAccuracyChance01` for Accuracy). Migrate by routing those readers off the primary, then add Speed/Accuracy to `T66MapPrimaryToUnifiedStat`.
- `GetSecondaryStatValue(DamageReduction/EvasionChance)` cases still compute an internal base from `GetArmorStat()`/`GetEvasionStat()` (full primary). Only non-combat callers (UI/tooltips) now hit them; resolve during Phase 3 display work.

**Verified:** `T66Editor` compiles clean (`Result: Succeeded`, only a pre-existing unrelated Niagara C4996 warning). Combat math is a balance change — needs playtest tuning of the routed `%` magnitudes.

**2026-06-09 — Phase 3a hero-select "Best stats" landed** (`T66DataTypes.h`, `T66HeroSelectionScreen.h`, `T66HeroSelectionScreen_Stats.cpp`):
- `FHeroData.BestStat1/2/3` (ET66SecondaryStatType, default None) — authored per-hero top-3.
- Hero card now shows **"BEST STATS"** + the 3 labels (via `MakeHeroSelectionBestStatsList`), replacing the numeric primary-stat columns. Empty authoring falls back to the hero's primary attack-category triple (e.g. AOE → AOE Damage/Scale/Speed).
- Required relocating the `ET66HeroStatType`/`ET66SecondaryStatType` enum cluster above `FHeroData` (UHT needs the enum declared before the UPROPERTY that references it). Done via deterministic LF-preserving line reorder.
- Old primary-stat column helpers in `T66HeroSelectionScreen_Stats.cpp` are now dead (MSVC C4505 off by default; clean up in a later pass).

**2026-06-09 — Phase 1b (Speed/Accuracy) + legacy cleanup landed** (`T66RunStateSubsystem_Stats.cpp`, `T66RunStateSubsystem.h`, `T66RunStateSubsystem_Private.h`, `T66HeroSelectionScreen_Stats.cpp`):
- All 8 stats now migrated. `Speed→MovementSpeed` (folded into `GetSpeedStat` so the movement component needs no change); `Accuracy→Accuracy` to-hit stat. Both readers exclude permanent-buff from the innate base.
- `T66MapPrimaryToUnifiedStat` covers all 8; level-up/relic routing dropped their legacy `else` branches.
- **Previous-version cleanup:** deleted `ApplyPrimaryGainToSecondaryBonuses` + `ApplyLevelUpPrimaryGainTenths` (defs + decls), the per-primary feeder helpers in `_Private.h` (`T66_Get*SecondaryTypes`, `T66_GetSecondaryTypesForPrimary`, `T66_GetHeroMainAttackSecondaryType`), and the dead primary-column helpers in `T66HeroSelectionScreen_Stats.cpp`.
- All verified compiling (`Result: Succeeded`).

**Infrastructure + cleanup: COMPLETE.** The full single-tier stat model is wired end-to-end and the legacy primary-fan-out is gone.

**2026-06-09 — Full primary/secondary → "stat" rename (scripted, scoped):**
- Pass 1 (1805 replacements / 53 files) + pass 2 (348 / 35 files). `ET66SecondaryStatType`→`ET66StatType`; `SecondaryStat*`→`Stat*`; `PrimaryStat*`→`BaseStat*` (old primaries are the hidden base axis); plus `SecondaryBoost`→`StatBoost`, `SecondaryMultiplier`→`StatMultiplier`, `T66Secondary*`→`T66Stat*`, `T66Primary*PerPoint`→`T66Base*PerPoint`.
- Only stat-specific multi-word substrings renamed; unrelated `*Color`/`*Tint`/`*Text`/`*Interact`/attack `PrimaryCategory`/`PrimaryTarget`/`PrimaryActorTick` left alone.
- 3 base-vs-unified member collisions fixed (`AT66BoostInteractable`, `FT66LootWheelPresentationParams`, `FLockedLootWheelReward`): base member → `BaseStatType`/`BoostBaseStatType`.
- `[CoreRedirects]` added (`Config/DefaultEngine.ini`): enum + item-DataTable property redirects so authored item data carries over.

**Deliberately left as primary/secondary (rationale):** lowercase JSON wire keys (`secondary_stats`, …) for save/backend compat; `ET66HeroStatType` (no offending word; the base-axis enum) and `PrimaryCategory`/`HeroPrimaryAttackCategory` (attack category, not a stat); historical save-migration fn names; a few ambiguous UI tokens and prose comments.

**2026-06-09 — JSON wire keys renamed** (saves wipeable, no migration): `secondary_stats`→`stat_bonuses`, `secondary_stat_bonus_override`→`stat_bonus_override`, `secondary_buff`→`stat_buff` — consistent across `T66BackendRunSerializer` (write), `T66BackendRunSummaryParser` (read), `T66PlayerController_Overlays`, and `T66PowerUpScreen`; compiles clean. The base-stats object stays `"stats"` (no offending word; renaming the unified bonuses to `"stats"` would have collided with it). `outcome_value_secondary` left (casino, not a stat). No in-repo server references these keys; an external leaderboard server would need matching key updates. `Gameplay/Stats/MASTER_STATS.md` design doc still uses old terminology (docs, not updated here).

**Remaining (presentation + balance, deferred):** Phase 3b `+%` display language (stats panel / power-up / tooltips / item cards still render legacy numbers), save-field review of legacy base-stat snapshot fields, and balance playtest tuning of the routed `%` magnitudes.

## 7. Notes
- Enum is now `ET66StatType` (renamed from `ET66SecondaryStatType` in the full rename pass; CoreRedirect added for data continuity).
- Anti-double-count check: umbrella All-X must not also be applied as a per-category multiplier in the same pass — verified in `GetStatValue` (relic power lives only in the unified accumulation, excluded from every innate base reader).
