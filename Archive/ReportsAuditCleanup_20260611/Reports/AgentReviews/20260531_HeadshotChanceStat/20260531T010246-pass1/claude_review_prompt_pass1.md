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

Review depth: targeted.
Perform packet completeness, cited-anchor, instruction/scope, and verification-adequacy checks. Deepen only when the protocol escalation triggers require it.
Keep the exact output headings below.

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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260531_HeadshotChanceStat\review_packet_pass1.md
- Output scope: review of the packet below only.

<review_packet>
# Headshot Chance Stat Replacement - Review Packet Pass 1

Working goal: Replace Crit Damage with Headshot Chance across the live T66 item/stat pipeline, make critical hits always deal fixed 2x damage, wire Headshot Chance as a new stun-on-hit secondary stat, update data/docs/UI labels, and verify the runtime and staged build paths.

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
- Headshot stun needs tuning. Use a data-driven initial value of `0.75` seconds, matching the existing `Dazed` status effect duration in `Content/Data/StatusEffects.csv`, so design can tune it later without C++ changes.
- Headshot chance formula should be chance-like, not multiplier-like: authored hero base chance plus secondary bonus points, then selected-drug multiplier and Accuracy primary multiplier, clamped `0..1`.

Applicable repo instructions:
- Root `AGENTS.md`: live repo first, Claude review default, folder instruction discovery, staged standalone verification for playable runtime changes.
- `Gameplay/GAMEPLAY_AGENTS.md`: prefer data-authored tuning over hardcoded C++ defaults; runtime gameplay changes need compile/build and staged standalone validation.
- `Gameplay/Stats/MASTER_STATS.md`: update after material stat schema/item/buff/UI/persistence changes.
- `Gameplay/Combat/MASTER_COMBAT.md`: update after material combat/damage-model/stat changes; preserve current hit-zone/headshot targeting model.
- `UI/UI_AGENTS.md`: UI text/stat panel changes need build-level verification, but this is not reference-fidelity UI work.
- `Scripts/README.md`: use data-table reload helpers after CSV/schema changes.
- Pending issues read: `Content/Data/pending_issues_Data.md`, `Source/T66/Data/pending_issues_Data.md`, `Source/T66/Core/pending_issues_Core.md`, `Source/T66/Core/RunState/pending_issues_RunState.md`, `Source/T66/Gameplay/pending_issues_Gameplay.md`, `Source/T66/UI/pending_issues_UI.md`.

Current live seams found:
- Item row: `Content/Data/Items.csv` currently has `Item_CritDamage` under primary `Accuracy`, secondary `CritDamage`.
- Hero row schema: `Content/Data/Heroes.csv` and `FHeroData` currently expose `BaseCritDamage`.
- Stat enum: `ET66SecondaryStatType::CritDamage` is live in `Source/T66/Data/T66DataTypes.h`.
- Accuracy stat order currently includes `CritChance`, `CritDamage`, `AttackRange`, `Execute`.
- Run-state computes crit damage through `GetSecondaryStatValue(CritDamage)` and `GetCritDamageMultiplier()`.
- Combat auto-attack resolves crit damage by multiplying base damage by `GetCritDamageMultiplier()`.
- Existing stun APIs are available on rich enemies, lightweight mobs, and bosses: `ApplyStun(float)`.
- Player-facing labels/drugs/stats/run summary include Crit Damage.
- Backend serializer/parser and saved-run snapshots carry secondary stat maps by enum key.
- Data reload helpers exist: `Scripts\SetupItemsDataTable.py`, `Scripts\ImportHeroDataTable.py`, `Scripts\SetupPlayerExperienceDataTable.py`.

Implementation plan:
1. Schema/data
   - Add appended `ET66SecondaryStatType::HeadshotChance` to preserve existing enum values.
   - Mark `CritDamage` deprecated/inert in `T66IsDeprecatedSecondaryStatType`.
   - Change Accuracy family ordering to `CritChance`, `HeadshotChance`, `AttackRange`, `Execute`.
   - Replace `Item_CritDamage` with `Item_Headshot` in `Content/Data/Items.csv`, primary `Accuracy`, secondary `HeadshotChance`.
   - Add `Item_CritDamage` to retired/removed item ID handling so old inventories do not keep live crit-damage items.
   - Rename `FHeroData::BaseCritDamage` to `BaseHeadshotChance` and update `Content/Data/Heroes.csv` header accordingly.
   - Convert existing hero row values for that column to chance values; use `0.0` as the initial baseline so Headshot Chance comes from items/level/diploma/drug tuning unless designers later author hero-specific base chance.
   - Add `HeadshotStunDurationSeconds` to `FT66PlayerExperienceDifficultyTuning`, current JSON value `0.75` for all difficulties.

2. Runtime stat math
   - Keep `GetCritDamageMultiplier()` as a compatibility method but make it return fixed `2.0f`.
   - Add `GetHeadshotChance01()` and `GetDataDrivenHeadshotStunDurationSeconds()`.
   - Implement `HeadshotChance = clamp((HeroBaseHeadshotChance + BonusPoints * 0.005) * M * HeroAccuracyMultiplier, 0.0, 1.0)`.
   - Keep old `CritDamage` secondary getter/baseline inert/fixed for compatibility only; it should not appear in live stat lists.

3. Combat behavior
   - In auto-attack resolved damage, crits always multiply by fixed `2.0`.
   - On each resolved auto-attack hit, roll `HeadshotChance`; on success, apply stun to rich enemies, lightweight mobs, and bosses using the existing `ApplyStun` APIs and the data-driven stun duration.
   - Show existing `Headshot` status/event text when the stun applies.
   - Preserve Execute behavior as "chance on critical hit" after crits.

4. UI/localization/buffs/persistence
   - Replace Crit Damage labels/tooltips/item-name variants/drug slugs/drug names/stat panel grouping with Headshot Chance.
   - Update run summary to show Headshot Chance instead of Crit Damage.
   - Replace `CritDamage` in `UT66BuffSubsystem` single-use drug stat list with `HeadshotChance` at the same array position so existing selected/owned Crit Damage drug slots effectively migrate to Headshot Chance.
   - Backend serializer emits `HeadshotChance` for new snapshots; parser maps both `HeadshotChance` and old `CritDamage` keys to the live `HeadshotChance` stat.
   - Snapshot secondary loop includes the appended `HeadshotChance` enum.

5. Docs and smokes
   - Update `Gameplay/Stats/MASTER_STATS.md` and `Gameplay/Combat/MASTER_COMBAT.md`.
   - Extend existing `-T66StatPipelineSmoke` or add focused checks to prove:
     - `Item_CritDamage` is retired/skipped.
     - `Item_Headshot` exists and is live under Accuracy.
     - `CritDamage` is not live-facing.
     - `GetCritDamageMultiplier()` returns `2.0`.
     - Headshot Chance increases from the item/drug path.
     - A forced headshot chance can stun a target.

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

</review_packet>
