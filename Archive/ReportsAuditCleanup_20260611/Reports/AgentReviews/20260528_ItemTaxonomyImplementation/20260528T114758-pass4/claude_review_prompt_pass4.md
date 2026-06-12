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
- Current Loot Crate reward boost is scoped to the crate interactable UI path: `UT66CrateOverlayWidget::GenerateStrip()` applies `ApplyLootCrateBias()` using `RunState->GetLootCrateRewardMultiplier()`. It does not modify enemy loot bag drops or arcade reward spawning.
- Current chest reward is scoped to `AT66ChestInteractable::LockChestReward()`, which rolls chest gold using `RunState->GetEffectiveLuckBiasStat()` and the chest rarity. Loot Chest should stay on this interactable reward surface.
- Current loot bag reward consumption is scoped to `AT66PlayerController::HandleSecondaryInteract()` when `AT66LootBagPickup` is selected. That is the place to upgrade the picked item rarity for the Loot Bag stat; enemy/world/arcade loot bag spawning remains out of scope.
- Current loot wheel reward selection is scoped to `AT66LootWheelInteractable::LockLootWheelReward()`. That is the place to bias wheel reward quality for the Loot Wheel stat; world spawn count/range remains out of scope.
- `Item_TreasureChest` is the existing row ID and enum-side stat key. Direct code references are low, but existing data/assets/docs already use TreasureChest naming. This pass keeps `Item_TreasureChest`/`TreasureChest` as compatibility keys and changes player-facing text to Loot Chest instead of introducing `Item_LootChest`.
- `T66CombatShared` is an existing shared combat helper namespace used by combat projectiles/components. It is the best place for the central non-boss OHKO application helper.
- `UT66CombatComponent::TryExecuteTarget()` is idol-specific: it kills regular enemies/mobs but intentionally applies a current-HP burst to bosses. This is not the item-stat Execute path and should not be routed through the item OHKO helper in this pass.
- `T66IsDeprecatedSecondaryStatType()` and `T66IsLiveSecondaryStatType()` in `T66DataTypes.h` are the concrete deprecation mechanism. Iterators should use `T66IsLiveSecondaryStatType()` for item-facing/runtime-live filtering; old enum values remain serializable and parseable for compatibility.
- Current family helpers already place `CounterAttack` under Evasion and `Taunt`/`ReflectDamage` under Armor. The requested order changes their order and display labels, not their primary family. `DamageReduction` is already Armor and `EvasionChance` is already Evasion.
- `UT66BuffSubsystem::GetAllSingleUseBuffTypes()` is consumed by buff roll/expiry/recompute paths and UI helpers. The source list should not be broadly rewritten; item-facing removals should be handled by `T66IsLiveSecondaryStatType()` filtering plus defensive UI switch updates.
- `Source/T66/Core/T66GameInstance.cpp` still synthesizes `Item_Accuracy` and `Item_GamblersToken`, and still forces `Item_Accuracy` into the random item pool.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Combat.cpp` currently lets Assassinate and Crush apply 99999 damage to `AT66BossBase`; that violates the new no-boss OHKO rule.
- `Source/T66/Gameplay/T66CombatComponent.cpp` has no item-stat Execute hook in the crit path.
- `UT66GameInstance::GetItemData()` already routes item lookup through `NormalizeLegacyItemID()` before DataTable lookup and synthetic fallback. That resolver should map `Item_GamblersToken` to `Item_VendorToken`; `IsRandomItemPoolEligible()` should exclude the canonical Vendor Token ID and the legacy alias.
- Existing user-facing "Gambler's Token" strings appear in localization, achievement descriptions, HUD special casing, item-card text, and code comments. Persistent schema/member names such as `ActiveGamblersTokenLevel`, `GamblersTokenUnlockedLevel`, and `ApplyGamblersTokenPickup()` can remain internal this pass to avoid save churn, but new runtime item IDs/events/player-facing text must use Vendor Token.
- No existing sprites were found for `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, or `Item_LootWheel`; existing icon assets can be reused for now without creating new art.
- Narrow git status shows many pre-existing modified gameplay/UI files. The implementation must patch only the item/stat seams and avoid reverting unrelated dirty work.

## Implementation Plan

1. Data and ordering
   - Rewrite `Content/Data/Items.csv` into the requested live order:
     - Damage: AOE, Bounce, Pierce, DOT
     - Attack Speed: AOE, Bounce, Pierce, DOT
     - Attack Scale: AOE, Bounce, Pierce, DOT
     - Accuracy: Crit Chance, Crit Damage, Attack Range, Execute Chance
     - Evasion: Dodge Chance, Counter Chance, Invisibility Chance, Assassinate Chance
     - Armor: Damage Reduction, Reflect Chance, Taunt Chance, Crush Chance
     - Luck: Loot Crate, Loot Chest, Loot Bag, Loot Wheel
     - Special: Backrooms Quick Revive, Vendor Token
   - Replace the live `Item_Accuracy` row with `Item_Execute`.
   - Keep the existing `Item_TreasureChest` row ID and `TreasureChest` secondary key for compatibility, but rename player-facing text to Loot Chest.
   - Add `Item_LootBag` and `Item_LootWheel` rows using existing safe icon paths until dedicated art exists.
   - Add `Item_VendorToken` as `PrimaryStatType=Special`, `SecondaryStatType=VendorToken`, `BaseBuyGold=100`, `BaseSellGold=0`, using existing safe icon paths until dedicated art exists.
   - Leave HP Regen/Life Steal rows absent; do not delete shared sprite assets in this Mini-excluded pass.

2. Schema and stat families
   - Append new `ET66SecondaryStatType` values after existing serialized values: `Execute`, `LootBag`, `LootWheel`, `VendorToken`.
   - Do not reorder existing enum values.
   - Mark old live-facing `Accuracy`, `Cheating`, `Stealing`, and `GamblerToken` as deprecated by adding them to `T66IsDeprecatedSecondaryStatType()`. Do not add `UMETA(Hidden)` this pass; editor hiding is separate from runtime live filtering and could disrupt existing serialized assets. Keep internal hero head-targeting `Accuracy` math available for `GetAccuracyChance01()`.
   - Keep `EvasionChance` as the serialized implementation for Dodge; rename only user-facing text to `Dodge Chance`.
   - Keep backend/telemetry keys stable for serialized compatibility: `EvasionChance` stays the payload key while UI labels say Dodge Chance; `TreasureChest` stays the payload key while UI labels say Loot Chest. Add payload keys for `Execute`, `LootBag`, and `LootWheel`.
   - Update `T66IsAccuracyFamilySecondaryStatType` to include `Execute` and exclude old item-facing `Accuracy`.
   - Update `T66_GetAccuracySecondaryTypes`, `T66_GetEvasionSecondaryTypes`, `T66_GetArmorSecondaryTypes`, and `T66_GetLuckSecondaryTypes` to the requested order and live members. This keeps Counter in Evasion and Reflect/Taunt in Armor, matching their current helper families.
   - Do not broadly rewrite `GSingleUseBuffStats`; its callers already use live-stat filtering. Add defensive names/slugs/icons for new values where switch statements require it.
   - Audit every `ET66SecondaryStatType::Accuracy` reference before appending. Update known tail-of-enum loop sites such as `UT66LeaderboardSubsystem` to iterate the new appended maximum or an explicit live list, and leave hero-selection/base-accuracy math references in place when they are not item-facing.

3. Runtime stat values and reward multipliers
   - Add `GetExecuteChance01()`, `GetLootChestRewardMultiplier()`, `GetLootBagRewardMultiplier()`, and `GetLootWheelRewardMultiplier()` to `UT66RunStateSubsystem`.
   - Implement secondary stat values:
     - `Execute`: chance value, scaled like Assassinate/Crush from item bonus/multiplier with a zero baseline, clamped 0..1.
     - `TreasureChest`: keep the same multiplier formula and expose it through `GetLootChestRewardMultiplier()`.
     - `LootBag`: reward multiplier like Loot Crate / Loot Chest.
     - `LootWheel`: reward multiplier like Loot Crate / Loot Chest.
   - Add baselines and backend serialization keys for new live secondaries so run summaries do not drop them.

4. Central OHKO rule
   - Add a shared helper in `T66CombatShared` that applies non-boss OHKO damage with these semantics:
     - `AT66BossBase`: return false before applying damage.
     - `AT66EnemyBase`: apply lethal damage through `TakeDamageFromHeroHitZone()`/`TakeDamageFromHero()`; this includes miniboss enemies because they are still `AT66EnemyBase`.
     - `AT66MobBase`: apply lethal damage through `TakeDamageFromHeroHitZone()` when alive.
   - Helper signature will take `AActor* TargetActor`, an optional `const FT66CombatTargetHandle* TargetHandle`, `FName DamageSourceID`, and `FName EventType`. When no handle is available, the helper resolves the actor's default body handle for enemy/mob classes before applying lethal damage.
   - Replace the existing Assassinate and Crush `99999` lines in `T66RunStateSubsystem_Combat.cpp` with this helper. Reflected/counter non-OHKO boss damage remains on the existing boss branches.
   - Use the same helper for item-stat Execute in `T66CombatComponent.cpp` after a critical hit rolls true.
   - Leave idol `TryExecuteTarget()` unchanged as an idol mechanic, because it intentionally does boss burst rather than item-stat OHKO.

5. Reward source wiring
   - Loot Crate: preserve current `UT66CrateOverlayWidget::GenerateStrip()` crate-only multiplier behavior.
   - Loot Chest: apply `GetLootChestRewardMultiplier()` in `AT66ChestInteractable::LockChestReward()` to the rolled chest gold result after the normal range roll, then record the original range plus final amount. Do not scale both range and result, and do not rename the compatible `TreasureChest` data key.
   - Loot Bag: apply `GetLootBagRewardMultiplier()` only when the loot bag interactable is consumed in `AT66PlayerController::HandleSecondaryInteract()`, by deterministically upgrading the picked item rarity tier from the already-spawned bag rarity before `AddItemWithRarity()` and before the reveal HUD. Do not reroll weights, and do not patch enemy loot bag drop counts, world spawn count, or arcade reward spawning in this pass.
   - Loot Wheel: apply `GetLootWheelRewardMultiplier()` only inside `AT66LootWheelInteractable::LockLootWheelReward()` by deterministically upgrading final locked quality: gold result after roll, item rarity tier after item selection, and boost points/duration after boost selection. Do not reroll reward type weights, and do not patch world loot wheel spawn counts or gallery/showcase spawns.
   - Keep changes bounded to existing reward rolls; do not add new interactable types or UI reward surfaces.

6. Vendor Token canonicalization
   - Introduce canonical `Item_VendorToken`.
   - Map `Item_GamblersToken` to `Item_VendorToken` in `UT66GameInstance::NormalizeLegacyItemID()` so DataTable lookup, HUD card lookup, save-held item lookup, and any `GetItemData()` caller resolve the old ID through the canonical row.
   - Keep special-case predicates accepting both names at input boundaries (`T66_IsVendorTokenItem()` / existing token helper sites in RunState and PlayerController), so old loot-bag drops or save slots still activate the special item path before/after normalization.
   - New drops, synthetic fallback item data, structured events, HUD item-card checks, community starting item rules, and lab unlocks use `Item_VendorToken`.
   - Rename user-facing text and comments to Vendor Token.
   - Keep persistent variable/member/function names such as `ActiveGamblersTokenLevel`, `GamblersTokenUnlockedLevel`, and `ApplyGamblersTokenPickup()` as internal compatibility names only. They should not leak into HUD strings, item text, achievement descriptions, structured events, or new item IDs.
   - Leave unrelated casino/gambler boss/gameplay naming alone.

7. UI/localization/docs
   - Update `T66LocalizationSubsystem`, `T66StatsPanelSlate`, `T66PowerUpScreen`, `T66TemporaryBuffUIUtils`, and hero-selection stat helper text for the final names/order.
   - Grep and update every player-facing `Treasure Chest` string to `Loot Chest`. Keep internal enum/data key names `TreasureChest`/`Item_TreasureChest` only where they are compatibility identifiers, asset filenames, or row IDs.
   - Update `T66ItemCardTextUtils` to display Vendor Token.
   - Update `Gameplay/Stats/MASTER_STATS.md`, `Gameplay/Stats/Accuracy_Item_And_TempBuff_Audit.md`, and any checked Gameplay stats/player-experience doc section that enumerates the old order.

8. Data asset reload and verification
   - Run a focused text validation over CSV/code to confirm:
     - live CSV contains `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, `Item_LootWheel`.
     - live CSV does not contain `Item_Accuracy`, `Item_Cheating`, `Item_Stealing`, `Item_HpRegen`, `Item_LifeSteal`, or `Item_GamblersToken`.
     - remaining `Item_GamblersToken`/`GamblersToken` references are legacy aliases or persistent schema names only, not new drops, structured events, or player-facing strings.
     - remaining `Treasure Chest` strings are not player-facing; row IDs/asset filenames may remain `TreasureChest`.
   - Run Unreal reload for `DT_Items` using the owning item DataTable setup command path if available.
   - Compile editor target with:
     - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
   - Inspect compile output for warnings/errors related to enum switches or item IDs, specifically `C4061`, `C4062`, `LogDataTable`, `Item_GamblersToken`, and `Item_Accuracy`.
   - Add a narrow, non-shipping automation smoke path only if existing hooks cannot prove the changed runtime seams. The smoke proof needs to cover:
     - boss target rejects Execute/Assassinate/Crush OHKO while non-OHKO Reflect/Counter boss damage still applies on the same combat path;
     - miniboss/enemy/mob targets accept Execute/Assassinate/Crush OHKO;
     - Loot Bag and Loot Wheel high multipliers visibly change locked/consumed reward quality;
     - legacy `Item_GamblersToken` resolves to `Item_VendorToken` without missing-row warnings.
   - If a full staged standalone smoke is practical after compile, refresh staged standalone via `Scripts/StageStandaloneBuild.ps1` and report logs. If staging is blocked by unrelated dirty work or existing pending blockers, report exact command/output and stop short of claiming staged proof.
   - Review a narrow git diff after implementation against the dirty-file list to confirm unrelated pre-existing modifications were not reverted.

## Known Risks / Containment

- New item IDs lack dedicated art. This plan reuses existing icon paths for data safety and avoids creating art in this pass.
- There are many pre-existing dirty gameplay/UI files. Patch only the named item/stat files and verify with narrow diffs.
- Appending enum values requires updating loops that previously assumed `Accuracy` was the last secondary value, especially `T66LeaderboardSubsystem`.
- Alchemy is not being restored as a live item. If stale Alchemy warnings require more than a simple resolver deletion, defer and record a pending issue instead of reintroducing it.
- Boss OHKO is explicitly disallowed; non-OHKO reflected/counter damage to bosses can remain.

## Review Request

Review this revised implementation plan for repo fit, missing seams, unsafe assumptions, and verification gaps. In particular, check whether the revised Loot Bag / Loot Wheel interactable-only scope now matches Loot Crate/Loot Chest parity, whether keeping `Item_TreasureChest` as the compatible Loot Chest data key is acceptable, and whether excluding idol Execute as a separate boss-burst mechanic is sufficiently grounded.

</review_packet>
