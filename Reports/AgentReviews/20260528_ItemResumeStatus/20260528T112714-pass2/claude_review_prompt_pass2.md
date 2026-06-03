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

## Output Scope

Status answer only: summarize where the item work stands and what remains. Do not implement in this pass.

## Applicable Instructions Checked

- `AGENTS.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Content/Data/pending_issues_Data.md`
- `Scripts/pending_issues_Scripts.md`

Mini/minigame scope is excluded by root `AGENTS.md`; Mini data was not treated as in-scope.

## User-Set Target From Prior Messages

The user explicitly set this final order and naming:

- Accuracy: Crit Chance, Crit Damage, Attack Range, Execute Chance.
- Evasion: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance.
- Armor: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance.
- Luck: Loot Crate, Loot Chest, Loot Bag, Loot Wheel.
- Vendor Token is canonical; there should be no mention of Gambler Token.
- Backrooms Quick Revive and Vendor Token should use primary `Special`.
- HP Regen and Life Steal should be deleted from main-run items, not just hidden.
- Cheating and Stealing should be deprecated from the current Luck item set.
- Evasion Chance should be called Dodge/Dodge Chance with the same effect.
- Accuracy secondary item should be replaced by Execute, described as chance of a critical-hit OHKO.

These order changes are intentional and user-requested, including the Crit Chance/Crit Damage swap and the Armor/Evasion reorder.

## Live Evidence

- `Content/Data/Items.csv:17` still has `Item_Accuracy` as `PrimaryStatType=Accuracy`, `SecondaryStatType=Accuracy`.
- `Content/Data/Items.csv:26-29` still has Luck rows `Item_LootCrate`, `Item_TreasureChest`, `Item_Cheating`, and `Item_Stealing`.
- `Content/Data/Items.csv:30` has `Item_BackroomsQuickRevive` as `PrimaryStatType=Special`, `SecondaryStatType=None`.
- `Content/Data/Items.csv` has no `Item_VendorToken`, `Item_Execute`, `Item_LootBag`, or `Item_LootWheel` row.
- `Content/Data/Items.csv` has no `Item_HpRegen` or `Item_LifeSteal` row.
- `Content/Data/pending_issues_Data.md:13` records previous staged smoke warnings for missing `Item_GamblersToken` and `Item_Alchemy`.
- `Content/Data/pending_issues_Data.md:20` records that HP Regen / Life Steal shared sprite assets remain after the main CSV rows were removed, pending Mini-inclusive audit; this remains out of scope for the main-run item pass.
- `Source/T66/Data/T66DataTypes.h:858-859` already has primary `Accuracy` and `Special`.
- `Source/T66/Data/T66DataTypes.h:895-920` still has old secondaries including `HpRegen`, `LifeSteal`, `SpinWheel`, `TreasureChest`, `Cheating`, `Stealing`, `EvasionChance`, `GamblerToken`, `Alchemy`, and `Accuracy`; it has no `Execute`, `LootBag`, `LootWheel`, or `VendorToken`.
- `Source/T66/Data/T66DataTypes.h:930-935` currently treats `SpinWheel`, `GamblerToken`, `HpRegen`, `LifeSteal`, and `Alchemy` as deprecated, but not `Cheating` or `Stealing`.
- `Source/T66/Data/T66DataTypes.h:943-948` still classifies Accuracy-family secondaries as Crit Damage, Crit Chance, Attack Range, and Accuracy.
- `Source/T66/Core/T66GameInstance.cpp:33-34` still defines `GamblersTokenItemID` and `AccuracyItemID`.
- `Source/T66/Core/T66GameInstance.cpp:107-117` still synthesizes `Item_Accuracy` as Accuracy/Accuracy.
- `Source/T66/Core/T66GameInstance.cpp:123-136` still synthesizes `Item_GamblersToken` with `GamblerToken` secondary and `Item_GamblersToken_*` sprite paths.
- `Source/T66/Core/T66GameInstance.cpp:784-798` still forces `Item_Accuracy` into the random item pool when eligible.
- `Source/T66/Gameplay/T66GamblerBoss.cpp:160` still drops `Item_GamblersToken`.
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp:117-119` still recognizes only `Item_GamblersToken` as the token item.
- `Source/T66/UI/HUD/T66HUDPresentationController.cpp:901-903` still special-cases `Item_GamblersToken`.
- `Source/T66/UI/T66ItemCardTextUtils.cpp:85-90` and `:129` still special-case `GamblerToken`.
- `Source/T66/Core/T66LocalizationSubsystem.cpp:304-314`, `:373-383`, `:475-489`, and `:565-583` still expose old Luck/Accuracy/Gambler/Evasion naming.
- `Source/T66/UI/Screens/T66PowerUpScreen.cpp:1487-1490` still groups Accuracy as Crit Damage, Crit Chance, Attack Range, Accuracy; Armor as Taunt, Damage Reduction, Reflect Damage, Crush; Evasion as Evasion Chance, Counter Attack, Invisibility, Assassinate; Luck as Treasure Chest, Cheating, Stealing, Loot Crate.
- `Source/T66/UI/T66StatsPanelSlate.cpp:104-130` still uses old stats-panel category order and old Luck members.
- `Scripts/SetupItemsDataTable.py:2-34` reloads and saves `/Game/Data/DT_Items` from `Content/Data/Items.csv`.

Current narrow git status already shows existing modifications in:

- `Content/Data/DT_Items.uasset`
- `Content/Data/Items.csv`
- `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`
- `Gameplay/Stats/MASTER_STATS.md`
- `Source/T66/Core/T66GameInstance.cpp`
- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/UI/T66ItemCardTextUtils.cpp`

These must be worked with, not reverted.

## Resolved Implementation Assumptions For A Later Pass

- Dodge is a display/localization rename only. Keep existing `ET66SecondaryStatType::EvasionChance` and current dodge behavior for serialization compatibility; update user-facing text to `Dodge Chance`.
- Execute should be appended as a new secondary enum value and replace the old `Accuracy` item. The old `Accuracy` enum value can remain only as deprecated/compatibility data, not as a live item.
- Execute safe default: apply only to normal non-boss enemies on a successful critical hit. Do not OHKO bosses or minibosses unless the user explicitly asks for boss-affecting Execute.
- Vendor Token should become the canonical item ID and user-facing name. Old `Item_GamblersToken` / `GamblerToken` should remain only as a compatibility alias if an existing save/log/runtime reference still needs it; it should not appear in new live item data, item cards, drops, or UI text.
- Loot Chest should preserve the current Treasure Chest effect and likely keep the serialized `TreasureChest` enum as the compatibility implementation unless a direct row-ID migration is safer. User-facing label becomes `Loot Chest`.
- Alchemy is not being reintroduced to the live item list. Because prior smoke logs warned about missing `Item_Alchemy`, the implementation pass must audit active references and either remove the stale resolver or keep a compatibility-only deprecated alias so staged logs stop warning. It should not appear in the final player-facing item breakdown.
- Existing modified `DT_Items.uasset` should be regenerated from the final source CSV via the owning reload script rather than manually discarded. Any existing binary delta must be preserved only if it is reproduced by the CSV reload.

## Proposed User Answer

We were at the point where the intended item taxonomy was clear, but the implementation is only partially done.

Already done in the live repo:

- `Item_HpRegen` and `Item_LifeSteal` are gone from the main `Content/Data/Items.csv`.
- `Item_BackroomsQuickRevive` already uses primary `Special`.
- The C++ primary-stat enum already has `Special`.
- Shared HP Regen / Life Steal sprite assets still exist, but that is intentionally deferred because deleting shared sprites requires a Mini-inclusive reference audit, and Mini is out of scope for this pass.

The target state we still need to finish:

- Weapon Modifiers: Damage, Attack Speed, Attack Scale.
- Character Modifiers:
  - Accuracy: Crit Chance, Crit Damage, Attack Range, Execute Chance.
  - Evasion: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance.
  - Armor: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance.
- Luck Modifier: Loot Crate, Loot Chest, Loot Bag, Loot Wheel.
- Special Items: Item Quick Revive and Vendor Token.
- No player-facing HP Regen, Life Steal, Cheating, Stealing, Accuracy secondary item, or Gambler Token naming.

What still needs to be done:

1. Update item data:
   - Replace `Item_Accuracy` with Execute.
   - Add canonical `Item_VendorToken`.
   - Remove Cheating and Stealing from the current live Luck rows.
   - Add Loot Bag and Loot Wheel.
   - Convert Treasure Chest presentation to Loot Chest while keeping compatibility if needed.
   - Reorder rows/categories exactly as above.

2. Update C++ schema and labels:
   - Append new secondaries for Execute, Loot Bag, Loot Wheel, and Vendor Token.
   - Keep `EvasionChance` internally but rename all player-facing text to Dodge Chance.
   - Update stat labels/tooltips/base item names for Reflect Chance, Taunt Chance, Crush Chance, Counter Chance, Invisibility Chance, Assassinate Chance, Execute Chance, Loot Chest, Loot Bag, Loot Wheel, and Vendor Token.
   - Deprecate old live-facing Accuracy/Cheating/Stealing/Gambler Token paths without breaking serialized compatibility.

3. Update runtime behavior:
   - Remove/update the synthetic `Item_Accuracy` fallback and random-pool injection.
   - Rename token drop/pickup/HUD/card paths from Gambler Token to Vendor Token.
   - Add Execute to the crit combat path. Safe implementation is normal enemies only, not bosses/minibosses, unless you want boss OHKOs too.
   - Wire the Luck-family rows to the actual loot sources: crate, chest, bag, and wheel.
   - Audit the existing `Item_Alchemy` missing-row warning and make it intentionally deprecated/compat-only or remove the stale resolver that asks for it.

4. Refresh docs/assets:
   - Update `Gameplay/Stats/MASTER_STATS.md` tables and tooltips.
   - Update `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md` to reflect Execute replacing Accuracy and the final Dodge/Luck/token state.
   - Regenerate `/Game/Data/DT_Items` from `Content/Data/Items.csv` with `Scripts/SetupItemsDataTable.py`; do not hand-edit the uasset.

5. Verify:
   - Compile the editor target after enum/code changes.
   - Reload/save `DT_Items` through Unreal and confirm rows exist for Execute, Vendor Token, Loot Bag, and Loot Wheel.
   - Confirm logs no longer contain missing-row warnings for `Item_GamblersToken`, `Item_Alchemy`, `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, or `Item_LootWheel`, except any explicitly retained legacy alias path.
   - Open/dump the stats panel or power-up screen and verify the four requested category orders.
   - Smoke a Vendor Token drop/pickup path and confirm all card/HUD text says Vendor Token.
   - Because this affects playable drops/pickups, refresh staged standalone with `Scripts/StageStandaloneBuild.ps1` and verify the staged exe/shortcut path if implementation proceeds.

The main design detail I would carry as a safe assumption is Execute normal-enemy-only. If you want Execute to affect bosses or minibosses too, say that before implementation; otherwise I would avoid letting a random crit delete bosses.

## Review Request

Check whether this revised status answer accurately reflects the live repo, resolves the previous review objections, and avoids overstating completed work.

</review_packet>
