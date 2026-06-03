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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ItemTaxonomyImplementation\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet: Item Taxonomy Implementation

## Working Goal

Implement the main-run item taxonomy/data changes: canonical Vendor Token naming with no Gambler Token mention in player-facing item presentation, data order updates for Accuracy/Evasion/Armor, Execute replacing secondary Accuracy, Chance suffix display names where requested, the prior Luck target list, and refreshed runtime data assets/verification.

## User Request

- Implement the previous list changes plus added changes.
- Vendor Token should be canonically named Vendor Token; there should be no player-facing mention of Gambler Token.
- Prefer changing item order in data so future item breakdowns report the new order.
- Accuracy order: Crit Chance, Crit Damage, Attack Range, Execute Chance.
- Evasion order: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance.
- Armor order: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance.
- Other names can remain as they are.

## Applicable Instructions

- Root `AGENTS.md`: goal set before work; start from live repo; default scope excludes Mini/minigame; use Claude review before implementation; report verification.
- `Gameplay/GAMEPLAY_AGENTS.md`: prefer data-authored tuning; runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect playable standalone.
- `Gameplay/Stats/MASTER_STATS.md`: update after material changes to stat schema, item stat rules, stat UI, or secondary-stat activation/deprecation.
- `Reports/AGENTS.md`: review packets under `Reports/AgentReviews`.
- Pending issues read:
  - `Content/Data/pending_issues_Data.md`
  - `Source/T66/Data/pending_issues_Data.md`
  - `Source/T66/Core/pending_issues_Core.md`
  - `Source/T66/Core/RunState/pending_issues_RunState.md`
  - `Source/T66/UI/pending_issues_UI.md`

## Live Evidence

- `Content/Data/Items.csv` currently has old active rows: `Item_Accuracy` with secondary `Accuracy`, `Item_EvasionChance` with secondary `EvasionChance`, Armor rows ordered Taunt/Reflect/Crush/DamageReduction, Luck rows LootCrate/TreasureChest/Cheating/Stealing, and special QuickRevive.
- `Source/T66/Data/T66DataTypes.h` currently has display names `Accuracy`, `Dodge`, `Gambler's Token`, `Damage Reflection`, `Taunt`, `Crush`, `Assassinate`.
- `Source/T66/Core/T66GameInstance.cpp` synthesizes `Item_GamblersToken` and old `Item_Accuracy`.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` special-cases `Item_GamblersToken` and records `Source=GamblerToken`.
- `Source/T66/Gameplay/T66CombatComponent.cpp` already has a non-item `Execute` damage event helper for idol special behavior, but no generic item secondary execute-on-crit path.
- `Scripts/SetupItemsDataTable.py` is the owning reload script for `Content/Data/Items.csv` -> `/Game/Data/DT_Items`; prior project review packets approved running it via:
  - `UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupItemsDataTable.py" -unattended -nop4 -nosplash`

## Planned Implementation

### Data

- Rewrite `Content/Data/Items.csv` row order to group active main-run rows as:
  - Damage
  - AttackSpeed
  - AttackScale
  - Accuracy: `Item_CritChance`, `Item_CritDamage`, `Item_AttackRange`, `Item_ExecuteChance`
  - Evasion: `Item_DodgeChance`, `Item_CounterAttack`, `Item_Invisibility`, `Item_Assassinate`
  - Armor: `Item_DamageReduction`, `Item_ReflectDmg`, `Item_Taunt`, `Item_Crush`
  - Luck: `Item_LootCrate`, `Item_LootChest`, `Item_LootBag`, `Item_LootWheel`
  - Special: `Item_BackroomsQuickRevive`
- Replace the old `Item_Accuracy` CSV row with `Item_ExecuteChance`, using existing `Item_Accuracy_*` icon references until dedicated art exists.
- Replace old `Item_EvasionChance` row with `Item_DodgeChance`, using existing `Item_EvasionChance_*` icon references.
- Replace old `Item_TreasureChest` row with `Item_LootChest`, using existing `Item_TreasureChest_*` icon references.
- Add `Item_LootBag` and `Item_LootWheel` rows with existing placeholder item icon references from the current item sprite set, without touching Mini-owned assets.
- Omit `Item_Cheating` and `Item_Stealing` from active CSV.

### Runtime schema and stat behavior

- Append new `ET66SecondaryStatType` values to preserve existing serialized enum values:
  - `ExecuteChance`
  - `LootChest`
  - `LootBag`
  - `LootWheel`
  - `VendorToken`
- Mark old `Accuracy`, `TreasureChest`, `Cheating`, `Stealing`, and `GamblerToken` as deprecated/inert item-facing values while keeping them parsable for compatibility.
- Update `T66IsAccuracyFamilySecondaryStatType()` to include `ExecuteChance`, not the old `Accuracy` item secondary.
- Add `UT66RunStateSubsystem::GetExecuteChance01()`.
- Change the old head-targeting accuracy getter so it no longer depends on an item-facing `Accuracy` secondary. It should use hero base accuracy plus the Headshot passive, preserving target-preference behavior without an Accuracy item.
- Implement Execute Chance in auto-attack crit resolution: after a crit succeeds, roll `GetExecuteChance01()` and OHKO normal enemies/mobs on success; do not OHKO bosses.
- Update secondary stat value/baseline switch cases for `ExecuteChance`, `LootChest`, `LootBag`, and `LootWheel`.
- Update category arrays/order in run state helpers, stats panel, and power-up rows.

### Vendor Token naming

- Add canonical `Item_VendorToken` synthetic special item data and map legacy `Item_GamblersToken` requests to it where item data is looked up or token pickups are recognized.
- Change player-facing text from Gambler's Token/Gambler Token to Vendor Token.
- Change structured runtime event payload source from `GamblerToken` to `VendorToken`.
- Do not rename existing save-field/function identifiers that would break profile/run save compatibility; compatibility internals may still contain old symbol names, but player-facing text/data should not.

### Docs

- Update `Gameplay/Stats/MASTER_STATS.md`.
- Update `Gameplay/Combat/MASTER_COMBAT.md` where it lists old item secondaries.
- Update `Content/Data/pending_issues_Data.md` stale missing-item warning from `Item_GamblersToken` to `Item_VendorToken` if still relevant.

## Out Of Scope

- Mini/minigame files, data, or UI.
- New item sprite generation/import. Reuse existing item sprite references for the new taxonomy rows in this pass.
- Full save migration that renames serialized C++ property identifiers. Compatibility should be preserved through aliases where runtime item IDs are consumed.
- New UI surfaces beyond labels/order/stat behavior.

## Risks

- Introducing new enum values requires C++ compile verification.
- `Item_Accuracy` and `Item_GamblersToken` may exist in old saves; legacy aliases must keep them working.
- New Luck rows may not yet have dedicated icon art; this pass should avoid deleting old sprite assets.
- Execute-on-crit is gameplay-affecting and needs at least focused compile plus staged standalone smoke.

## Verification Plan

1. Narrow grep to verify active `Items.csv` row order and old active CSV rows removed.
2. Narrow grep to verify player-facing strings do not contain `Gambler's Token` or `Gambler Token`.
3. Run focused Development build:
   - `Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
4. Refresh `/Game/Data/DT_Items` via `Scripts/SetupItemsDataTable.py` with `UnrealEditor-Cmd.exe`.
5. Run `Scripts\StageStandaloneBuild.ps1`.
6. Smoke the staged exe to at least menu/log startup enough to verify no immediate load/cook failure from the item schema.

</review_packet>
