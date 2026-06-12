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

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ItemSystemBreakdown\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet: Main-Run Item System Breakdown

## Working Goal

Break down the live T66 non-minigame item system, including item categories, counts, and the overall item architecture, without changing production files.

## User Request

"Give me a breakdown of the items, I have, what is the overall system again, how many items I have just give me the breakdown first."

## Scope And Assumptions

- Interpret "items" as the main run item system backed by `Content/Data/Items.csv` and `/Game/Data/DT_Items`.
- Exclude Mini/minigame-mode systems because root instructions say the default scope excludes Mini/minigame systems unless explicitly named.
- No implementation, content edits, build, staging, or runtime capture are requested.
- This is a read-only answer; no PPF is required because this is not process-governed visual/media/import/build work.

## Instructions Read

- Root AGENTS instructions provided in the chat.
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `UI/UI_AGENTS.md`
- `Reports/AGENTS.md`
- Pending issue files checked:
  - `Content/Data/pending_issues_Data.md`
  - `Source/T66/Data/pending_issues_Data.md`
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - `Source/T66/Core/RunState/pending_issues_RunState.md`
  - `Source/T66/UI/pending_issues_UI.md`

## Evidence Gathered

- `Content/Data/Items.csv` has 31 CSV rows.
- `Content/Data/Items.csv` columns are `ItemID`, rarity icons, `PrimaryStatType`, `SecondaryStatType`, `BaseBuyGold`, and `BaseSellGold`.
- Applying the same live-pool logic visible in `T66IsLiveSecondaryStatType(...)` plus reward-only filtering gives 28 normal live item templates.
- The three non-live/special/compatibility rows in the CSV are:
  - `Item_HpRegen` with deprecated secondary `HpRegen`
  - `Item_LifeSteal` with deprecated secondary `LifeSteal`
  - `Item_BackroomsQuickRevive` with secondary `None`
- Live normal templates group cleanly into seven primary families with four items each:
  - Accuracy: 4
  - Armor: 4
  - AttackScale: 4
  - AttackSpeed: 4
  - Damage: 4
  - Evasion: 4
  - Luck: 4
- Raw CSV primary grouping, including non-live rows, is:
  - Armor: 6
  - Evasion: 5
  - Accuracy: 4
  - AttackScale: 4
  - AttackSpeed: 4
  - Damage: 4
  - Luck: 4
- `Content/Items/Sprites` has 121 `Item_*.uasset` sprite assets:
  - 30 black
  - 30 red
  - 30 yellow
  - 30 white
  - 1 no-rarity-suffix asset, `Item_BackroomsQuickRevive`

## Proposed Answer Scope

Answer the user with:

- Main count first:
  - 31 item rows in the main item data table.
  - 28 normal live random/shop/reward item templates.
  - 3 non-live/special rows.
- Category breakdown:
  - Seven live families, four items each.
  - Mention raw CSV includes extra Armor/Evasion rows because `HpRegen`, `LifeSteal`, and `BackroomsQuickRevive` are compatibility/special.
- Overall system:
  - `Content/Data/Items.csv` is the source table and `DT_Items.uasset` is the cooked data table.
  - `FItemData` is the template row; `FT66InventorySlot` is the runtime instance with item ID, rarity, line-1 roll, secondary override, and seed.
  - `UT66GameInstance` caches eligible item IDs and rolls random item templates.
  - `UT66RunStateSubsystem` owns current inventory, adds item slots, rerolls line-1 values by rarity, recomputes derived stats, handles alchemy/selling/special items.
  - Loot bags, crates, loot wheels, shop stock, and special Backrooms/Gambler flows feed items into RunState.
  - HUD/shop/card UI reads item data and rarity-specific icons.
- Caveat:
  - This excludes mode-specific minigame item tables by instruction; if the user wants those included, do a separate explicit pass.

## Verification Performed

- Read folder/process instructions.
- Counted rows with PowerShell `Import-Csv Content\Data\Items.csv`.
- Grouped rows by `PrimaryStatType`.
- Recreated live-pool filter from source rules and counted 28 live / 3 non-live rows.
- Counted `Content\Items\Sprites\Item_*.uasset` assets and grouped by rarity suffix.
- Inspected source paths:
  - `Source/T66/Data/T66DataTypes.h`
  - `Source/T66/Core/T66GameInstance.cpp`
  - `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp`
  - `Source/T66/Core/T66RunStateSubsystem.h`
  - `Source/T66/Gameplay/T66LootBagPickup.cpp`
  - `Source/T66/Gameplay/T66PlayerController_Combat.cpp`
  - `Source/T66/UI/T66CrateOverlayWidget.cpp`
  - `Source/T66/Gameplay/T66LootWheelInteractable.cpp`

## Risks

- The user may mean every item table across all side modes. That would be a different scope and is intentionally excluded here by root instructions.
- No runtime validation was run because this is a read-only data/system breakdown, not a behavior change.

</review_packet>
