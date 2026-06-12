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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ItemTaxonomyImplementation\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet: Main-Run Item Taxonomy Implementation

## Working Goal

Implement the main-run item taxonomy/data changes: add a central non-boss OHKO rule for Execute/Assassinate/Crush that can affect minibosses but not bosses, canonically replace Gambler Token with Vendor Token, update item data/order/names for Accuracy/Evasion/Armor/Luck/Special, wire Loot Bag and Loot Wheel reward stats to their interactables, refresh docs/data assets, and verify the affected code/UI/runtime seams.

## User Clarifications To Carry Forward

- Execute, Assassinate, and Crush are all OHKO effects and should share a central rule set.
- Those OHKO effects can kill minibosses.
- Those OHKO effects must not OHKO bosses.
- Loot Bag and Loot Wheel stats improve rewards for the loot bag interactable and loot wheel interactable, the same concept as Loot Crate and Loot Chest.
- No need to broaden this pass to Mini-owned cleanup.

## Applicable Instructions Checked

- `AGENTS.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/Stats/MASTER_STATS.md`
- `Content/Data/pending_issues_Data.md`
- `Source/T66/Data/pending_issues_Data.md`
- `Source/T66/Core/pending_issues_Core.md`
- `Source/T66/Core/RunState/pending_issues_RunState.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Source/T66/UI/pending_issues_UI.md`
- `Scripts/pending_issues_Scripts.md`

PPF is not required: this is data/schema/runtime/UI text behavior, not a visual/media/import/VFX process-governed task.

## Current Evidence

- `Content/Data/Items.csv` still has `Item_Accuracy`, `Item_Cheating`, and `Item_Stealing`; it lacks `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, and `Item_LootWheel`.
- `Content/Data/Items.csv` no longer has `Item_HpRegen` or `Item_LifeSteal`, and `Item_BackroomsQuickRevive` is already `PrimaryStatType=Special`.
- `Content/Data/pending_issues_Data.md` says HP Regen/Life Steal sprite asset deletion remains deferred pending Mini-inclusive reference audit; this pass will not delete those shared assets.
- `Source/T66/Data/T66DataTypes.h` has primary `Special`, but secondary enum still has old `Accuracy`, `GamblerToken`, `Cheating`, `Stealing`, no `Execute`, no `LootBag`, no `LootWheel`, no `VendorToken`.
- `Source/T66/Data/T66DataTypes.h` and `Source/T66/Core/RunState/T66RunStateSubsystem_Private.h` still classify Accuracy-family and Luck-family lists with old members.
- `Source/T66/Core/T66GameInstance.cpp` still synthesizes `Item_Accuracy` and `Item_GamblersToken`, and still forces `Item_Accuracy` into the random item pool.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp`, `Source/T66/Gameplay/T66GamblerBoss.cpp`, `Source/T66/Gameplay/T66PlayerController_Combat.cpp`, `Source/T66/UI/HUD/T66HUDPresentationController.cpp`, and `Source/T66/UI/T66ItemCardTextUtils.cpp` still use Gambler Token naming / IDs.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp` currently lets Assassinate and Crush apply 99999 damage to `AT66BossBase`; that violates the new no-boss OHKO rule.
- `Source/T66/Gameplay/T66CombatComponent.cpp` has idol-specific `TryExecuteTarget` logic, but no item-stat Execute hook in the crit path.
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp`, `Source/T66/Gameplay/T66ChestInteractable.cpp`, `Source/T66/Gameplay/T66EnemyBase.cpp`, `Source/T66/Gameplay/T66ArcadeInteractableBase.cpp`, and `Source/T66/Gameplay/T66LootWheelInteractable.cpp` are the reward-source seams.
- No existing sprites were found for `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, or `Item_LootWheel`; existing icon assets can be reused for now without creating new art.
- Narrow git status shows many pre-existing modified gameplay/UI files. The implementation must patch only the item/stat seams and avoid reverting unrelated dirty work.

## Implementation Plan

1. Data and ordering
   - Rewrite `Content/Data/Items.csv` into the requested order:
     - Damage: AOE, Bounce, Pierce, DOT
     - Attack Speed: AOE, Bounce, Pierce, DOT
     - Attack Scale: AOE, Bounce, Pierce, DOT
     - Accuracy: Crit Chance, Crit Damage, Attack Range, Execute Chance
     - Evasion: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance
     - Armor: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance
     - Luck: Loot Crate, Loot Chest, Loot Bag, Loot Wheel
     - Special: Backrooms Quick Revive, Vendor Token
   - Replace live `Item_Accuracy` with `Item_Execute`.
   - Replace live `Item_TreasureChest` with canonical `Item_LootChest` if feasible; use existing TreasureChest icon paths and `TreasureChest` secondary as the compatibility implementation while displaying Loot Chest.
   - Add `Item_LootBag` and `Item_LootWheel` rows using existing safe icon paths until dedicated art exists.
   - Add `Item_VendorToken` as `PrimaryStatType=Special`, `SecondaryStatType=VendorToken`, `BaseBuyGold=100`, `BaseSellGold=0`, using existing safe icon paths until dedicated art exists.
   - Leave HP Regen/Life Steal rows absent; do not delete shared sprite assets in this Mini-excluded pass.

2. Schema and stat families
   - Append new `ET66SecondaryStatType` values after existing serialized values: `Execute`, `LootBag`, `LootWheel`, `VendorToken`.
   - Do not reorder existing enum values.
   - Mark old live-facing `Accuracy`, `Cheating`, and `Stealing` as deprecated/inactive along with existing deprecated stats.
   - Keep `EvasionChance` as the serialized implementation for Dodge; rename only user-facing text to `Dodge Chance`.
   - Update `T66IsAccuracyFamilySecondaryStatType` to include `Execute` and exclude old `Accuracy`.
   - Update `T66_GetAccuracySecondaryTypes`, `T66_GetEvasionSecondaryTypes`, `T66_GetArmorSecondaryTypes`, and `T66_GetLuckSecondaryTypes` to the requested order and live members.
   - Update `UT66BuffSubsystem::GetAllSingleUseBuffTypes` source list to remove HP Regen, Life Steal, Accuracy, Cheating, Stealing, Alchemy, and include Execute/LootBag/LootWheel as live selectable secondaries if single-use buff UI still uses that list.

3. Runtime stat values and reward multipliers
   - Add `GetExecuteChance01()`, `GetLootChestRewardMultiplier()`, `GetLootBagRewardMultiplier()`, and `GetLootWheelRewardMultiplier()` to `UT66RunStateSubsystem`.
   - Implement secondary stat values:
     - `Execute`: chance value, scaled like Assassinate/Crush (`HeroBase` fallback of 0 plus item bonus/multiplier, clamped 0..1).
     - `TreasureChest`: continue same effect, but use as Loot Chest internally.
     - `LootBag`: reward multiplier like Loot Crate / Loot Chest.
     - `LootWheel`: reward multiplier like Loot Crate / Loot Chest.
   - Add baselines and backend serialization keys for new live secondaries so run summaries do not drop them.

4. Central OHKO rule
   - Add one shared helper path for OHKO application with these semantics:
     - `AT66EnemyBase`: lethal damage allowed, including `bIsMiniBoss` enemies.
     - `AT66MobBase`: lethal damage allowed if alive/active.
     - `AT66BossBase`: no OHKO damage; return false.
   - Use that helper for Assassinate and Crush in `T66RunStateSubsystem_Combat.cpp`.
   - Use it for item-stat Execute in `T66CombatComponent.cpp` after a critical hit rolls true.
   - Preserve idol-specific Execute behavior unless it is cheap and safe to route through the same helper without changing its intentionally partial boss burst behavior; the user asked for item OHKO centralization, not an idol redesign.

5. Reward source wiring
   - Loot Crate: preserve existing rarity-quality bias through `RngSub->UpdateLuckStat(...)` / `RollRarityWeighted`; if a direct reward multiplier getter is currently unused, do not invent unrelated crate behavior.
   - Loot Chest: keep same effect as current TreasureChest by applying the Loot Chest multiplier to chest reward quality/quantity where chest reward is rolled.
   - Loot Bag: apply `GetLootBagRewardMultiplier()` to loot bag reward quality/quantity:
     - enemy loot bag count / rarity in `T66EnemyBase.cpp`
     - world/tower loot bag rarity in `T66GameMode_WorldInteractables.cpp`
     - arcade loot bag reward rarity in `T66ArcadeInteractableBase.cpp`
   - Loot Wheel: apply `GetLootWheelRewardMultiplier()` to wheel reward quality:
     - world/tower loot wheel rarity in `T66GameMode_WorldInteractables.cpp`
     - actual loot wheel reward type/gold/item/boost quality in `T66LootWheelInteractable.cpp`
   - Keep changes bounded to existing reward rolls; do not add new interactable types or UI reward surfaces.

6. Vendor Token canonicalization
   - Introduce canonical `Item_VendorToken`.
   - Update token special-case checks to accept `Item_VendorToken` and legacy `Item_GamblersToken` as input aliases where saves/logs could still hold the old ID.
   - New drops, synthetic fallback item data, structured events, HUD item-card checks, and lab unlocks use `Item_VendorToken`.
   - Rename user-facing text to Vendor Token. Persistent variable/member names such as `ActiveGamblersTokenLevel` can remain in this pass if renaming them would be broad save-schema churn, but they should not leak into player-facing strings.
   - Remove user-facing "Gambler Token" text from item card/localization/stat names; leave unrelated casino/gambler gameplay names alone.

7. UI/localization/docs
   - Update `T66LocalizationSubsystem`, `T66StatsPanelSlate`, `T66PowerUpScreen`, and `T66TemporaryBuffUIUtils` for the final names/order.
   - Update `T66ItemCardTextUtils` to display Vendor Token.
   - Update `Gameplay/Stats/MASTER_STATS.md` and `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`.

8. Data asset reload and verification
   - Run a focused text validation over CSV/code to confirm:
     - live CSV contains `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, `Item_LootWheel`.
     - live CSV does not contain `Item_Accuracy`, `Item_Cheating`, `Item_Stealing`, `Item_HpRegen`, `Item_LifeSteal`, or `Item_GamblersToken`.
   - Run Unreal reload for `DT_Items` using `Scripts/SetupItemsDataTable.py` through the editor command path.
   - Compile editor target with:
     - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
   - If compile succeeds, run a narrow runtime/staged smoke if practical:
     - refresh staged standalone via `Scripts/StageStandaloneBuild.ps1`
     - check no missing-row warnings for new item IDs and no new player-facing `Item_GamblersToken` item path
     - capture or dump stats/power-up UI order if available through existing automation
   - If full staging is blocked by unrelated dirty work or existing pending blockers, report exact command/output and stop short of claiming staged proof.

## Known Risks / Containment

- New item IDs lack dedicated art. This plan reuses existing icon paths for data safety and avoids creating art in this pass.
- There are many pre-existing dirty gameplay/UI files. Patch only the named item/stat files and verify with narrow diffs.
- Appending enum values requires updating loops that previously assumed `Accuracy` was the last secondary value, especially `T66LeaderboardSubsystem`.
- Alchemy is not being restored as a live item. If warnings still ask for `Item_Alchemy`, remove or compatibility-isolate that stale resolver rather than reintroducing it into the live item set.
- Boss OHKO is explicitly disallowed; non-OHKO reflected/counter damage to bosses can remain.

## Review Request

Review this implementation plan for repo fit, missing seams, unsafe assumptions, and verification gaps. In particular, check whether the Loot Bag / Loot Wheel reward multiplier plan is too broad or misses the intended interactable reward path, and whether the central OHKO rule should also touch existing idol-specific Execute behavior in this pass.

</review_packet>
