You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_LevelUpStatsBreakdown\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Codex Review Packet - Level-Up And Stat Source Breakdown

## Working Goal

Inspect the live item/stat/level-up/diploma/drug systems, determine the implementation needed for secondary-only item stats and re-enabled level-up primary-stat progression, verify how these systems currently affect stats, and report the full breakdown plus any wiring problems before editing unless the requested change is already unambiguous.

## User Request

The user wants the stat model changed/confirmed as follows:

- Items should only show/apply the secondary line. Previously they improved both primary and secondary stats; now they should improve only the secondary stat.
- Re-enable level-up.
- Killing mobs grants experience.
- On level-up: heal health to full, trigger a wave that kills enemies around the hero within a certain range, and grant primary stats.
- Primary stat gains should be weighted by character/hero tuning, where some characters gain more of certain stats than others.
- Primary stats should be wired to improve secondary stats when upgraded.
- Produce the full breakdown of stat sources: item list, level-up summary, diplomas summary, drugs summary, and report whether each source is correctly wired or has problems.
- Mini/minigame scope should remain excluded.

## Applicable Repo Instructions

- Root `AGENTS.md`: derive working goal, inspect live repo, use Claude review for substantive answers/implementation unless skipped, avoid Mini/minigame scope unless explicitly named, report verification.
- `Gameplay/GAMEPLAY_AGENTS.md`: gameplay stats/XP/combat/runtime changes need compile/build verification and staged standalone validation when affecting playable standalone.
- `UI/UI_AGENTS.md`: UI changes require relevant build/capture/verification, but this pass is not a reference-image fidelity task.
- `Reports/AGENTS.md`: review/proof artifacts belong under `Reports/AgentReviews` and `Reports/Proof`.

## Current Live Findings

### Items

Files inspected:

- `Content/Data/Items.csv`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`
- `Source/T66/UI/T66ItemCardTextUtils.cpp`

Current state:

- Each item row still has `PrimaryStatType` and `SecondaryStatType`.
- Runtime inventory slots still store `Line1RolledValue`.
- `RecomputeItemDerivedStats()` currently applies item line 1 as a primary stat bonus:
  - `AddPrimaryBonusTenths(D.PrimaryStatType, PrimaryGainTenths)`
  - `ApplyPrimaryGainToSecondaryBonuses(...)`
- `RecomputeItemDerivedStats()` also applies the item secondary stat:
  - `AddItemSecondaryStatBonusTenths(D.SecondaryStatType, WholeStatToTenths(Slot.GetSecondaryStatBonusValue()))`
- Item card text currently builds both a primary line and secondary line for normal items.

Conclusion: items are not currently secondary-only. A code/UI change is required.

### Level-Up

Files inspected:

- `Source/T66/Core/T66RunStateSubsystem.h`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66MobBase.cpp`
- `Source/T66/Gameplay/T66MobBase.h`
- `Content/Data/Heroes.csv`
- `Source/T66/Core/T66GameInstance.cpp`

Current state:

- `DefaultXPToLevel = 0`.
- `InitializeHeroStatsForNewRun()` sets `XPToNextLevel = 0`, then `ResetForNewRun()` later sets it to `DefaultXPToLevel`, also 0.
- `AddHeroXP(int32 Amount)` is a deprecated no-op that resets `HeroLevel`, `HeroXP`, and `XPToNextLevel`.
- `ApplyOneHeroLevelUp()` is also a no-op that resets level and XP fields.
- `AT66EnemyBase::XPValue` still exists, defaulting to 20, but `AT66EnemyBase::OnDeath()` does not call `RunState->AddHeroXP(XPValue)`.
- Lightweight `AT66MobBase` does not appear to have an XP field or score/XP award path.
- Existing hero row data already contains per-level gain ranges: `LvlDmgMin/Max`, `LvlAtkSpdMin/Max`, `LvlAtkScaleMin/Max`, `LvlAccuracyMin/Max`, `LvlArmorMin/Max`, `LvlEvasionMin/Max`, `LvlLuckMin/Max`, `LvlSpeedMin/Max`.
- `UT66GameInstance::GetHeroStatTuning()` already loads these per-level ranges into `FT66HeroPerLevelStatGains`.
- `ApplyPrimaryGainToSecondaryBonuses(...)` already maps primary-stat gains to secondary-stat bonus buckets, including a hero-primary-attack-category bias for Damage/AttackSpeed/AttackScale.

Conclusion: the data seam for weighted per-hero level stat gains already exists, and the primary-to-secondary propagation helper exists. The actual level-up loop, XP award, full heal, and level-up kill wave are not wired.

### Diplomas

Files inspected:

- `Source/T66/Core/T66BuffSubsystem.h`
- `Source/T66/Core/T66BuffSubsystem.cpp`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`

Current state:

- Diploma fill-step state persists and can be displayed.
- Purchase/unlock methods are intentionally disabled:
  - `GetCostForNextFillStepUnlock()` returns 0.
  - `UnlockNextFillStep()` returns false.
  - `UnlockRandomStat()` returns false.
- Runtime stat bonus methods return empty/no bonus:
  - `GetTotalStatBonus()` returns 0.
  - `GetPermanentBuffStatBonuses()` returns an empty `FT66HeroStatBonuses`.
  - `RefreshPermanentBuffBonusesFromProfile()` clears `PermanentBuffStatBonuses`.
- `GetPermanentPrimaryBuffTenths()` exists, but no live code adds it into `GetPrecisePrimaryStatTenths()`.

Conclusion: diplomas are not currently wired to affect runtime stats.

### Drugs

Files inspected:

- `Source/T66/Core/T66BuffSubsystem.h`
- `Source/T66/Core/T66BuffSubsystem.cpp`
- `Source/T66/Core/RunState/T66RunStateSubsystem.cpp`
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`

Current state:

- Drugs/single-use buffs support owned and selected slots in save/UI data.
- `GetPendingSingleUseBuffMultipliers()` and `ConsumePendingSingleUseBuffMultipliers()` can compute 1.10x multipliers per selected copy.
- Purchases are disabled because `AreSingleUseBuffPurchasesAllowed()` returns false.
- Run start activation is not applying the selected multipliers:
  - `ActivatePendingSingleUseBuffsForRunStart()` currently only resets `SingleUseSecondaryMultipliers`.
  - `SingleUseSecondaryMultipliers` is not used in `GetSecondaryStatValue()`, which reads `SecondaryMultipliers`.
- `ConsumePendingSingleUseBuffMultipliers()` has no call sites.

Conclusion: drugs are not currently wired to affect runtime stats and are not purchasable in the current release settings/code path.

## Proposed User-Facing Answer Scope

Tell the user:

- Yes, the desired conceptual model is sound: items should become secondary-only; level-up should become the source of in-run primary-stat growth; primary stats should feed secondary stats through the existing primary-to-secondary propagation/multiplier logic.
- Current code is not yet in that state.
- Item stat source list should remain:
  - Weapon Modifiers: Damage -> AOE Damage, Bounce Damage, Pierce Damage, DOT Damage; Attack Speed -> AOE Speed, Bounce Speed, Pierce Speed, DOT Speed; Attack Scale -> AOE Scale, Bounce Scale, Pierce Scale, DOT Scale.
  - Character Modifiers: Accuracy -> Crit Chance, Crit Damage, Attack Range, Execute Chance; Evasion -> Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance; Armor -> Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance.
  - Luck Modifier: Loot Crate, Loot Chest, Loot Bag, Loot Wheel.
  - Special Items: Backrooms Quick Revive, Vendor Token.
- Level-up summary:
  - Intended: mob kills grant XP; reaching threshold increments hero level; level-up heals to full, fires a non-boss enemy-clearing wave in a radius, rolls hero-weighted primary stats from `Heroes.csv`, and propagates those primary gains into secondary stat bonuses.
  - Current: not wired. XP and level-up are explicitly disabled.
- Diplomas summary:
  - Intended/current decision needed: if re-enabled, diplomas probably should be permanent primary-stat bonuses, and because primary stats drive secondary effects they would indirectly improve secondary stats.
  - Current: displayed/persisted only; purchases and runtime stat effects are disabled.
- Drugs summary:
  - Intended/current decision needed: drugs are single-run selected secondary-stat multipliers.
  - Current: save/UI seams exist, but activation and runtime multiplication are broken/not wired; purchases disabled.

## Proposed Implementation If User Confirms Defaults

Do not implement arbitrary tuning without user confirmation unless the user says no clarification needed.

Required implementation shape:

1. Items secondary-only:
   - In `RecomputeItemDerivedStats()`, stop calling `AddPrimaryBonusTenths()` and stop calling `ApplyPrimaryGainToSecondaryBonuses()` for inventory items.
   - Keep `AddItemSecondaryStatBonusTenths()` for item `SecondaryStatType`.
   - Keep `PrimaryStatType` in data as a grouping/category field for UI list organization unless a broader data schema migration is requested.
   - Update item-card text to suppress primary line for normal items and display only the secondary/special line.

2. Level-up:
   - Set a real default XP threshold and progression rule.
   - Make `AddHeroXP()` accumulate XP and call `ApplyOneHeroLevelUp()` while `HeroXP >= XPToNextLevel`.
   - Make `ApplyOneHeroLevelUp()` increment `HeroLevel`, heal to full, run the kill wave, roll primary gains from `HeroPerLevelGains`, add those gains to `HeroPreciseStats`, call `ApplyPrimaryGainToSecondaryBonuses()` into persistent secondary bonus storage, and broadcast stat/heart/progress changes.
   - Award XP from `AT66EnemyBase::OnDeath()` using `XPValue`.
   - Decide whether lightweight mobs need their own XP value or whether only rich `AT66EnemyBase` enemies count for this first pass.
   - Implement level-up wave using `UT66ActorRegistrySubsystem` and existing damage/OHKO helper paths, excluding bosses.

3. Diplomas:
   - If the user wants diplomas active now, re-enable purchase/cost methods and return `FT66HeroStatBonuses` from fill-step counts.
   - Add `GetPermanentPrimaryBuffTenths()` to `GetPrecisePrimaryStatTenths()` and decide whether diploma primary gains should also propagate to secondary stat bonuses. User asked primary stats should improve secondary stats when upgraded, so likely yes.

4. Drugs:
   - If active now, make `ActivatePendingSingleUseBuffsForRunStart()` consume selected buffs and apply them into the map used by `GetSecondaryStatValue()`.
   - Either use `SingleUseSecondaryMultipliers` inside `GetSecondaryStatValue()` or merge consumed multipliers into `SecondaryMultipliers`.
   - Decide if purchases should be enabled in the current release variant or remain gated.

## Human Decisions Needed Before Implementation

These values are not safely inferable from current code:

1. XP threshold/progression:
   - Existing `XPValue` default is 20, but `DefaultXPToLevel` is 0.
   - Need a value such as 100 XP flat per level, 100 + 25 per level, or another curve.

2. Level-up wave radius:
   - User said "for a certain range" but did not specify a range.
   - Need a value such as 900 UU, 1200 UU, or current attack range.

3. Lightweight mob XP:
   - Rich `AT66EnemyBase` has `XPValue`; lightweight `AT66MobBase` does not.
   - Need to decide whether this first pass grants XP only from rich enemies or also adds XP to lightweight mobs.

4. Diplomas/drugs activation:
   - User asked for the full breakdown and wiring status, but did not explicitly say diplomas and drugs should be re-enabled in this same implementation pass.

## Verification Required If Implemented

- Focused compile:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
- Data reload only if CSV/DT schema/data changes are made.
- Add or extend a focused non-shipping smoke hook that verifies:
  - Item pickup increases secondary stat but no longer increases primary stat.
  - Enemy kill grants XP.
  - Level-up increments hero level and heals to full.
  - Level-up wave kills nearby normal enemies/minibosses but not bosses and not out-of-range enemies.
  - Level-up primary gain increases relevant secondary stats through `ApplyPrimaryGainToSecondaryBonuses()`.
  - Diplomas/drugs status matches the chosen implementation scope.
- If playable runtime behavior is changed, run `Scripts\StageStandaloneBuild.ps1` and staged executable smoke.


</review_packet>
