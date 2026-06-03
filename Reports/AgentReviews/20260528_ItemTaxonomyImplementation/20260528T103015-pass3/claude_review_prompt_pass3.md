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
- From the previous prompt in this same task chain: Execute is "chance of a critical hit OHKO the enemy."

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
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - `Gameplay/Combat/pending_issues_Combat.md`
  - `Source/T66/UI/pending_issues_UI.md`

## Live Evidence

- `Content/Data/Items.csv` currently has old active rows: `Item_Accuracy` with secondary `Accuracy`, `Item_EvasionChance` with secondary `EvasionChance`, Armor rows ordered Taunt/Reflect/Crush/DamageReduction, Luck rows LootCrate/TreasureChest/Cheating/Stealing, and special QuickRevive.
- `Source/T66/Data/T66DataTypes.h` currently has display names `Accuracy`, `Dodge`, `Gambler's Token`, `Damage Reflection`, `Taunt`, `Crush`, `Assassinate`.
- `Source/T66/Core/T66GameInstance.cpp` synthesizes `Item_GamblersToken` and old `Item_Accuracy`.
- `Source/T66/Core/RunState/T66RunStateSubsystem_EconomyInventory.cpp` special-cases `Item_GamblersToken` and records `Source=GamblerToken`.
- `Source/T66/Gameplay/T66CombatComponent.cpp` already has a non-item `Execute` damage event helper for idol special behavior, but no generic item secondary execute-on-crit path.
- A narrow source grep found the existing literal `Execute` event only in `T66CombatComponent.cpp`; floating combat text only special-cases the registered `Crit` event, so the current `Execute` label is not handled as idol-exclusive downstream.
- Existing Headshot passive is used by `UT66RunStateSubsystem::GetAccuracyChance01()`, which currently adds `+0.20`.
- `GetAccuracyChance01()` currently resolves through `GetSecondaryStatValue(Accuracy) + Headshot`. With no Accuracy item bonus, that baseline is `HeroBaseAccuracy * HeroAccuracyMultiplier`; with old `Item_Accuracy` bonuses, the item contribution increases head-targeting chance.
- The requested removal of the secondary Accuracy item stat means old/new owned Accuracy-row item bonuses should no longer increase head-targeting chance. Baseline hero accuracy and Headshot magnitude will remain unchanged.
- The placeholder sprite packages that will be reused are present in `Content/Items/Sprites`: `Item_Accuracy_*`, `Item_EvasionChance_*`, `Item_TreasureChest_*`, and `Item_LootCrate_*`.
- `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md` has no references to the old item IDs or secondary names from this taxonomy pass.
- `Scripts/SetupItemsDataTable.py` is the owning reload script for `Content/Data/Items.csv` -> `/Game/Data/DT_Items`; prior project review packets approved running it via:
  - `UnrealEditor-Cmd.exe T66.uproject -run=pythonscript -script="C:/UE/T66/Scripts/SetupItemsDataTable.py" -unattended -nop4 -nosplash`
- Pending issue reconciliation:
  - `Content/Data/pending_issues_Data.md` has a stale missing-data warning naming `Item_GamblersToken`; update to `Item_VendorToken`.
  - Add or update a `Content/Data` pending issue for the temporary reused Luck/Accuracy/Evasion placeholder sprites so the lack of dedicated `Item_LootBag`, `Item_LootWheel`, `Item_ExecuteChance`, and `Item_DodgeChance` icon art is not lost.
  - The HP Regen/Life Steal sprite pending issue remains valid and out of scope because Mini-inclusive asset deletion is still excluded.
  - Other read pending issues do not conflict with this item taxonomy pass.

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
- Deterministic placeholder sprite references:
  - `Item_ExecuteChance` uses `/Game/Items/Sprites/Item_Accuracy_*`.
  - `Item_DodgeChance` uses `/Game/Items/Sprites/Item_EvasionChance_*`.
  - `Item_LootChest` uses `/Game/Items/Sprites/Item_TreasureChest_*`.
  - `Item_LootBag` uses `/Game/Items/Sprites/Item_LootCrate_*` for this pass.
  - `Item_LootWheel` uses `/Game/Items/Sprites/Item_TreasureChest_*` for this pass.

### Exact display-name table

- `CritChance`: Crit Chance
- `CritDamage`: Crit Damage
- `AttackRange`: Attack Range
- `ExecuteChance`: Execute Chance
- `EvasionChance`: Dodge Chance
- `CounterAttack`: Counter Chance
- `Invisibility`: Invisibility Chance
- `Assassinate`: Assassinate Chance
- `DamageReduction`: Damage Reduction
- `ReflectDamage`: Reflect Chance
- `Taunt`: Taunt Chance
- `Crush`: Crush Chance
- `LootChest`: Loot Chest
- `LootBag`: Loot Bag
- `LootWheel`: Loot Wheel
- `VendorToken`: Vendor Token

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
- Change the old head-targeting accuracy getter so it no longer depends on an item-facing `Accuracy` secondary. It should use hero base accuracy, the primary Accuracy multiplier, and the existing Headshot passive:
  - preserved: no-item baseline head-targeting chance and Headshot's `+0.20`;
  - intentionally removed: old `Item_Accuracy` bonus contribution to head-targeting chance, because the user requested that secondary Accuracy be replaced by Execute.
- Implement Execute Chance per the user's previous prompt literally: after a crit succeeds, roll `GetExecuteChance01()` and OHKO the hit target on success for any valid auto-attack target type routed through this combat component, including enemies, mobs, and awakened bosses.
- Reuse the existing `Execute` damage event label for the new item Execute so floating combat/status handling stays semantically aligned with the existing idol execute event rather than creating a second event name.
- Update secondary stat value/baseline switch cases for `ExecuteChance`, `LootChest`, `LootBag`, and `LootWheel`.
- Update category arrays/order in run state helpers, stats panel, and power-up rows.
- Add a focused non-shipping gameplay automation capture mode for this pass that grants high-roll `Item_CritChance` and `Item_ExecuteChance`, uses the live combat component auto-attack path against a spawned normal target and an awakened boss target, and logs before/after HP plus PASS/FAIL. This is not a new design surface; it is verification plumbing for the requested new combat mechanic.

### Vendor Token naming

- Add canonical `Item_VendorToken` synthetic special item data and map legacy `Item_GamblersToken` requests to it where item data is looked up or token pickups are recognized.
- Change player-facing text from Gambler's Token/Gambler Token to Vendor Token.
- Change structured runtime event payload source from `GamblerToken` to `VendorToken`.
- Do not rename existing save-field/function identifiers that would break profile/run save compatibility; compatibility internals may still contain old symbol names, but player-facing text/data should not.

### Legacy item mapping behavior

Legacy item IDs in existing saves or external callers silently normalize at runtime:

- `Item_Accuracy` -> `Item_ExecuteChance`
- `Item_EvasionChance` -> `Item_DodgeChance`
- `Item_TreasureChest` -> `Item_LootChest`
- `Item_Cheating` -> `Item_LootWheel`
- `Item_Stealing` -> `Item_LootBag`
- `Item_GamblersToken` -> `Item_VendorToken`

This avoids orphaned owned items while keeping deprecated rows out of the active CSV. Existing serialized token-level fields keep their old C++ property names for save compatibility, but all new item IDs, display strings, structured event source labels, and UI text use Vendor Token.

Legacy aliases fully adopt the new row's stat config when recomputed. For example, an owned `Item_Accuracy` slot becomes `Item_ExecuteChance` and contributes Execute Chance, not the old head-targeting Accuracy effect.

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
- Literal boss OHKO from Execute Chance is a balance-impacting behavior, but it is the direct implementation of the user's "OHKO the enemy" request and avoids introducing an unrequested boss exception.

## Verification Plan

1. Narrow grep to verify active `Items.csv` row order and old active CSV rows removed.
2. Narrow grep to verify player-facing strings do not contain `Gambler's Token` or `Gambler Token`.
3. Narrow grep to verify mapping aliases exist for the legacy IDs listed above.
4. Run focused Development build:
   - `Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
5. Refresh `/Game/Data/DT_Items` via `Scripts/SetupItemsDataTable.py` with `UnrealEditor-Cmd.exe`.
6. Run `Scripts\StageStandaloneBuild.ps1`.
7. Smoke the staged exe to at least menu/log startup enough to verify no immediate load/cook failure from the item schema.
8. Run the focused staged gameplay execute proof mode. Required proof lines:
   - grants `Item_CritChance` and `Item_ExecuteChance`;
   - normal target before HP > 0 and after HP <= 0 with `Execute` event path;
   - awakened boss before HP > 0 and after HP <= 0 with `Execute` event path;
   - overall `ExecuteChanceSmoke Result=PASS`.
9. Legacy-save sanity check through runtime normalization evidence: add a narrow automation/log or source-level verification that `Item_Accuracy` and `Item_GamblersToken` normalize to `Item_ExecuteChance` and `Item_VendorToken` without dropping inventory/token recognition.

</review_packet>
