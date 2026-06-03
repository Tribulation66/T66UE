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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_LevelUpStatsBreakdown\review_packet_pass2.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet Pass 2 - Level-Up And Stat Source Implementation

## Working Goal

Implement the stat pipeline changes now that user decisions are resolved:

- Inventory items affect only their secondary stat line.
- In-run level-up is enabled.
- XP threshold is a flat, data-driven value.
- Default level-up wave radius is data-driven at 900 UU.
- Both rich enemies and lightweight mobs grant data-driven XP.
- Diplomas affect primary stats, and those primary bonuses also improve secondary stats through the existing primary-to-secondary propagation helper.
- Drugs affect secondary stats through selected single-use multipliers, but drug purchases stay disabled for now.
- Diploma upgrades may remain gated/unpurchasable by the demo gate, but existing saved/unlocked diploma state must work if present.
- Update the master stats/player-experience docs and report the final stat-source breakdown.

## User Decisions Resolved

1. XP curve: flat.
2. Level-up wave radius: 900 UU.
3. XP source: both lightweight and rich enemies.
4. Diplomas and drugs: both should work, but keep them unpurchasable for now.
5. Make these values data-driven because XP per mob and related values will be tuned later.

## Applicable Instructions

- Root `AGENTS.md`: use live repo state, default Claude review, data-driven gameplay tuning, do not include Mini/minigame scope unless explicitly named.
- `Gameplay/GAMEPLAY_AGENTS.md`: read `Gameplay/README.md`, prefer data-authored tuning, runtime-facing gameplay changes need compile/build and staged standalone validation when they affect playable standalone.
- `Gameplay/Stats/MASTER_STATS.md`: current doc says leveling/diplomas/drugs are deprecated and must be updated after this material stat-schema/rule change.
- `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md`: current doc says XP/levels are deprecated and must be updated after this material player-experience change.
- `UI/UI_AGENTS.md` and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`: item-card text changes are surgical and do not require a reference-image fidelity loop.
- Pending issues read:
  - `Source/T66/Core/RunState/pending_issues_RunState.md`
  - `Source/T66/Core/pending_issues_Core.md`
  - `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md`
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`

## Current Live State

- `UT66RunStateSubsystem::RecomputeItemDerivedStats()` still adds item line-1 primary stat bonuses and also applies primary-to-secondary proxy bonuses.
- `T66ItemCardTextUtils::BuildItemCardDescription()` still presents primary plus secondary lines for normal items.
- `DefaultXPToLevel` is `0`.
- `UT66RunStateSubsystem::AddHeroXP()` and `ApplyOneHeroLevelUp()` are no-ops.
- `AT66EnemyBase::OnDeath()` awards score/loot but does not award XP.
- `AT66MobBase` has no XP field and its death path currently only notifies the director/manager.
- `FT66EnemyData` has no XP column, so `Content/Data/Enemies.csv` cannot tune per-mob XP yet.
- `FT66PlayerExperienceDifficultyTuning` has no level-up XP threshold or wave-radius fields, so `Content/Data/PlayerExperience.json` cannot tune those values yet.
- `UT66BuffSubsystem::GetPermanentBuffStatBonuses()` and `GetTotalStatBonus()` return zero.
- `UT66RunStateSubsystem::RefreshPermanentBuffBonusesFromProfile()` clears permanent bonuses instead of reading diploma state.
- `UT66RunStateSubsystem::ActivatePendingSingleUseBuffsForRunStart()` clears selected drug multipliers instead of consuming them.
- `GetSecondaryStatValue()` does not multiply `SingleUseSecondaryMultipliers`.
- The existing helper `ApplyPrimaryGainToSecondaryBonuses(...)` already maps primary stat gains into secondary-stat bonus buckets using the current stat-family order.

## Planned Edit Scope

### Data-driven tuning

- Add `LevelUpXPThreshold` and `LevelUpWaveRadiusUU` to `FT66PlayerExperienceDifficultyTuning`.
- Add those fields to every row in `Content/Data/PlayerExperience.json`, using `100` and `900` as the starting values unless code review finds an existing better default.
- Add `XPValue` to `FT66EnemyData`.
- Add `XPValue` to `Content/Data/Enemies.csv` for all authored mobs, initially `20` unless code review identifies a better existing tuning source.
- Reload `DT_PlayerExperience` via `Scripts/SetupPlayerExperienceDataTable.py`.
- Reload `DT_Enemies` via `Scripts/SetupCombatRosterDataTables.py`.

### Items

- Keep inventory slot fields for save compatibility and vendor/token level use, but stop normal stat recompute from applying line-1 primary bonuses or line-1 primary-to-secondary proxy bonuses.
- Continue applying each normal item's direct secondary stat bonus from `Slot.GetSecondaryStatBonusValue()`.
- Keep reward-only special items and vendor token out of normal stat aggregation.
- Change item card descriptions so normal items present only the secondary line. Special items keep the `Special`/vendor-specific presentation where applicable.

### Level-up

- On new run, initialize `XPToNextLevel` from the selected difficulty's data-driven flat threshold.
- `AddHeroXP(Amount)` should accumulate XP, loop through level-ups while `HeroXP >= XPToNextLevel`, and keep the threshold flat.
- `ApplyOneHeroLevelUp()` should:
  - increment `HeroLevel`;
  - full-heal current HP to max HP and broadcast heart/progress updates;
  - roll primary-stat gains from the selected hero's existing `HeroPerLevelGains`;
  - add those primary gains to `HeroPreciseStats`;
  - propagate those gains into persistent secondary-stat bonuses through `ApplyPrimaryGainToSecondaryBonuses(...)`;
  - trigger a non-boss level-up wave in the data-driven radius.
- The level-up wave should use the existing central OHKO rule (`T66CombatShared::TryApplyNonBossOHKO`) so bosses are immune and minibosses/mobs can be killed.
- Do not include Mini/minigame systems.

### Rich and lightweight enemy XP

- Rich `AT66EnemyBase` should populate `XPValue` from `FT66EnemyData::XPValue` during `ConfigureAsMob(...)`.
- Rich death should call `RunState->AddHeroXP(XPValue)` for positive XP.
- Lightweight `AT66MobBase` should gain an `XPValue` field populated from `FT66EnemyData::XPValue` during `ConfigureAsMob(...)`.
- Lightweight death should call `RunState->AddHeroXP(XPValue)` for positive XP.
- Keep score/loot behavior outside this pass except for existing behavior already triggered by current death paths.

### Diplomas

- `UT66BuffSubsystem::GetTotalStatBonus()` should return visible unlocked fill steps plus random/overflow bonus for the requested primary stat.
- `GetPermanentBuffStatBonuses()` should return those primary bonuses for the supported non-Speed primary stats.
- Purchase functions can remain disabled where demo gating says unpurchasable.
- `UT66RunStateSubsystem::RefreshPermanentBuffBonusesFromProfile()` should read diploma bonuses from `UT66BuffSubsystem`, store primary bonuses, and build a permanent secondary-bonus map by passing those primary gains through `ApplyPrimaryGainToSecondaryBonuses(...)`.
- `GetPrecisePrimaryStatTenths()` should include permanent diploma primary bonuses.
- `GetSecondaryStatBonusTenths()` should include permanent diploma-derived secondary bonuses, persistent level-up secondary bonuses, and item secondary bonuses.

### Drugs

- `ActivatePendingSingleUseBuffsForRunStart()` should consume selected single-use buff multipliers from `UT66BuffSubsystem`.
- `GetSecondaryStatValue()` should multiply `SingleUseSecondaryMultipliers` together with item secondary multipliers.
- Purchase paths remain disabled.

### Docs and smoke

- Update `Gameplay/Stats/MASTER_STATS.md` and `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md`.
- Extend or add a non-shipping smoke hook to prove:
  - items no longer raise primary stats but still raise secondaries;
  - level-up increases level, heals HP, grants primary stats, and leaves persistent secondary bonuses intact;
  - rich enemies and lightweight mobs grant XP from data;
  - level-up wave kills non-bosses/mobs and rejects bosses;
  - diploma primary bonuses affect primary and secondary stats;
  - selected drug multipliers affect secondary stats.

## Out Of Scope

- Mini/minigame stat systems.
- Full rebalance of hero per-level gain weights.
- Full economy/unlock UX redesign for diplomas or drugs.
- New item icon/drug icon art.
- Removing compatibility fields from saves, backend payloads, or enums.

## Risks And Rollback Considerations

- XP awarded from lightweight death currently has no score/loot parity with rich death. This pass will add XP only and avoid broad loot/score changes unless required by compile/smoke.
- Level-up wave using OHKO can chain XP if killed enemies award XP; if review sees a reentrancy risk, implement with a guarded XP queue or a no-chain wave source.
- DataTable schema changes require reloading `.uasset` DataTables after source JSON/CSV edits.
- Demo-gated UI may still hide purchase affordances; runtime wiring must be proven independent of purchase availability.
- Item primary bonuses must remain save-compatible but runtime-inert for normal item stat math.

## Verification Plan

1. Run Claude plan review. Implementation only proceeds on a valid `Verdict: APPROVE`.
2. Reload data tables:
   - `Scripts/SetupPlayerExperienceDataTable.py`
   - `Scripts/SetupCombatRosterDataTables.py`
3. Compile focused editor target:
   - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject`
4. Run the non-shipping stat pipeline smoke from Unreal and save JSON proof under `Reports/AgentReviews/20260528_LevelUpStatsBreakdown/`.
5. Refresh staged standalone with `Scripts/StageStandaloneBuild.ps1` because this affects playable gameplay runtime.
6. Run the same smoke or an equivalent staged proof on the staged executable if the hook is available there.

## Codex Position Before Review

The requested implementation is now unambiguous because the user supplied all four human decisions. The safest repo-native approach is to keep the old fields for save/UI compatibility, move item stat contribution to secondary-only at recompute and item-card presentation, put the new XP/radius values in existing PlayerExperience tuning, put per-mob XP in the existing enemy roster table, and reactivate the already-existing primary-to-secondary propagation helper for level-up and diploma sources.

</review_packet>
