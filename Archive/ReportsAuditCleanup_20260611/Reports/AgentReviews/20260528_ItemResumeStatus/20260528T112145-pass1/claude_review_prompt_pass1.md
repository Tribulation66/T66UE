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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ItemResumeStatus\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet: Item Resume Status Answer

## Working Goal

Resume the main-run item taxonomy/data changes by inspecting the live repo, identifying what has already been changed, and determining the exact remaining implementation and verification work.

## User Question

"Ok great, lets go back to the items now, where were we and what needs to be done?"

## Applicable Instructions Checked

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Content/Data/pending_issues_Data.md`
- `Scripts/pending_issues_Scripts.md`

Mini/minigame scope is excluded by root `AGENTS.md`; Mini data was not treated as in-scope.

## Live Evidence

- `Content/Data/Items.csv`
  - `Item_HpRegen` and `Item_LifeSteal` are absent from main-run item CSV.
  - `Item_BackroomsQuickRevive` is present with `PrimaryStatType=Special` and `SecondaryStatType=None`.
  - No `Item_VendorToken` row exists.
  - `Item_Accuracy` still exists as `PrimaryStatType=Accuracy`, `SecondaryStatType=Accuracy`.
  - `Item_Cheating` and `Item_Stealing` still exist as Luck rows.
  - Luck rows are currently `LootCrate`, `TreasureChest`, `Cheating`, `Stealing`.
  - Accuracy order is currently `CritDamage`, `CritChance`, `AttackRange`, `Accuracy`.
  - Armor order is currently `Taunt`, `ReflectDamage`, `Crush`, `DamageReduction`.
  - Evasion order is currently `Invisibility`, `CounterAttack`, `Assassinate`, `EvasionChance`.
- `Content/Data/pending_issues_Data.md`
  - Notes main-run `Item_HpRegen` and `Item_LifeSteal` rows were removed, but shared sprite assets still exist pending Mini-inclusive reference audit.
  - Notes staged smoke logs previously warned about missing `Item_GamblersToken` and `Item_Alchemy`.
- `Source/T66/Data/T66DataTypes.h`
  - `ET66HeroStatType` already includes `Special`.
  - `ET66SecondaryStatType` still has `Accuracy`, `EvasionChance` display name "Dodge", `GamblerToken` display name "Gambler's Token", deprecated `HpRegen`, `LifeSteal`, `Cheating`, `Stealing`, and no `Execute`, `LootBag`, `LootWheel`, or `VendorToken`.
- `Source/T66/Core/T66GameInstance.cpp`
  - Synthetic special item path still uses `Item_GamblersToken`, `GamblersTokenItemID`, and sprite paths `Item_GamblersToken_*`.
  - Synthetic `Item_Accuracy` fallback still creates an Accuracy/Accuracy row.
- `Source/T66/Gameplay/T66GamblerBoss.cpp`
  - Still drops `Item_GamblersToken`.
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp`
  - Still checks `Item_GamblersToken`.
- `Source/T66/UI/T66ItemCardTextUtils.cpp`
  - Still has `GamblerTokenLineFormat` and text "Level {0}: sell items for {1}% of buy value."
- `Source/T66/UI/T66StatsPanelSlate.cpp`
  - UI category arrays still include Accuracy secondary `Accuracy`, Luck `TreasureChest`, `Cheating`, `Stealing`, `LootCrate`, old order for Armor/Evasion, and no Execute/LootBag/LootWheel.
- `Scripts/SetupItemsDataTable.py`
  - Reloads `/Game/Data/DT_Items` from `Content/Data/Items.csv`.

## Proposed Answer

Where we were:

- We had clarified the target item taxonomy:
  - Weapon Modifiers: Damage, Attack Speed, Attack Scale.
  - Character Modifiers: Accuracy, Evasion, Armor.
  - Luck Modifier: Luck.
  - Special Items: Backrooms Quick Revive and Vendor Token.
- We agreed to remove HP Regen and Life Steal from main-run items, not just hide them.
- We agreed the token should be canonically `Vendor Token`; no user-facing or item-system mention of `Gambler Token`.
- We agreed `BackroomsQuickRevive` and Vendor Token should use primary `Special`.
- We agreed Accuracy should remove the secondary Accuracy item and replace it with Execute, a critical-hit OHKO chance.
- We agreed Evasion Chance should display as Dodge / Dodge Chance while keeping the same dodge effect.
- We agreed the item order should become:
  - Accuracy: Crit Chance, Crit Damage, Attack Range, Execute Chance.
  - Evasion: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance.
  - Armor: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance.
  - Luck: Loot Crate, Loot Chest, Loot Bag, Loot Wheel.
  - Deprecated/removed from current items: Cheating, Stealing, HP Regen, Life Steal.

What is already done in the live repo:

- Main-run `Item_HpRegen` and `Item_LifeSteal` rows are already removed from `Content/Data/Items.csv`.
- `Item_BackroomsQuickRevive` is already a `Special` primary item.
- `Special` primary already exists in the C++ enum.
- The repo already records the remaining shared HP Regen / Life Steal sprite deletion as out-of-scope until Mini-owned references are audited.

What still needs to be done:

1. Data rows in `Content/Data/Items.csv`
   - Replace `Item_Accuracy` with `Item_Execute`.
   - Add/rename `Item_VendorToken` and remove `Item_GamblersToken` naming from the main item path.
   - Remove/deprecate `Item_Cheating` and `Item_Stealing` from current main-run Luck item rows.
   - Add Luck rows for `Item_LootBag` and `Item_LootWheel`; likely rename `Item_TreasureChest` display/stat to Loot Chest.
   - Reorder Accuracy, Evasion, Armor, and Luck rows as requested.

2. C++ enum/schema in `Source/T66/Data/T66DataTypes.h`
   - Add `Execute` as a secondary stat, preserving enum compatibility by appending rather than reordering existing serialized enum values.
   - Add `LootBag`, `LootWheel`, and probably `VendorToken` as appended secondary enum values.
   - Keep legacy `GamblerToken`, `Cheating`, `Stealing`, `HpRegen`, `LifeSteal`, and old compatibility names only where save/backend compatibility requires them, but remove them from live item-facing lists.
   - Update display names: `Damage Reduction`, `Reflect Chance`, `Taunt Chance`, `Crush Chance`, `Dodge Chance`, `Counter Chance`, `Invisibility Chance`, `Assassinate Chance`, `Execute Chance`, `Loot Chest`, `Loot Bag`, `Loot Wheel`, `Vendor Token`.

3. Runtime stat grouping and behavior
   - Update `T66_GetSecondaryTypesForPrimary` / stats-panel category arrays to the requested order.
   - Implement `Execute` value computation and hook it into combat critical-hit resolution as the chance that a critical hit OHKOs the enemy.
   - Keep Dodge behavior wired to the existing `EvasionChance` effect unless a new appended enum is chosen.
   - Add Luck getters/effects for Loot Chest, Loot Bag, and Loot Wheel based on existing loot source systems.

4. Vendor Token canonicalization
   - Rename code constants and item IDs from `Item_GamblersToken` to `Item_VendorToken`.
   - Update boss drop path, pickup recognition, synthetic fallback data, tooltip text, and any saved compatibility aliasing.
   - Decide whether old `Item_GamblersToken` must remain as a compatibility alias for existing saves/logs while being absent from new canonical data.

5. Docs/UI/import refresh
   - Update `Gameplay/Stats/MASTER_STATS.md` and `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`.
   - Run `Scripts/SetupItemsDataTable.py` through Unreal to refresh `/Game/Data/DT_Items`.
   - Run a focused compile/data validation; staged standalone is likely required if this affects playable item drops/pickups.

## Review Request

Check whether this answer accurately reflects the live repo state and whether it overstates what has already been completed or misses any major remaining seam.

</review_packet>
