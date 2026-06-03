Verdict: REVISE

## Blockers

None — no hard blockers, missing prerequisites, or unresolvable contradictions. Plan respects PPF non-applicability, AGENTS.md scope, and the user-stated OHKO/boss rules.

## Major Issues

1. **Loot Bag rarity upgrade formula is unspecified.** Plan says "deterministically upgrading the picked item rarity tier from the already-spawned bag rarity before `AddItemWithRarity()`" but does not define the multiplier-to-tier mapping. Without a concrete rule (e.g., `tiers_to_advance = floor(multiplier - 1.0)` clamped to max tier, or a threshold table), Codex's implementation will be unverifiable against the user's intent and divergent from the "same concept as Loot Crate" parity claim. Loot Crate uses a strip-bias multiplier on weighted entries; Loot Bag instead deterministically advances a single locked tier — those are not the same concept and need to be explicitly justified or aligned.

2. **Loot Wheel deterministic upgrade has three different reward types with one multiplier.** Plan says "gold result after roll, item rarity tier after item selection, and boost points/duration after boost selection," but does not specify how `GetLootWheelRewardMultiplier()` maps onto each. Gold is naturally multiplicative; rarity tier is discrete; boost points/duration could be either. Codex needs a single agreed formula per reward type or it will silently pick something arbitrary.

3. **Items.csv row schema for new rows is not specified.** Plan names `Item_Execute`, `Item_VendorToken`, `Item_LootBag`, `Item_LootWheel`, and gives `BaseBuyGold=100` / `BaseSellGold=0` for Vendor Token only. Missing for all four: exact icon path reused (which existing safe asset), description/flavor text, BaseBuyGold/BaseSellGold for the loot stat rows, any rarity weight columns the table uses. A CSV row missing required columns will fail DataTable import and rollback the whole pass.

4. **Reflect/Counter boss-damage policy needs an explicit value.** Plan says "non-OHKO reflected/counter boss damage remains on the existing boss branches" — but the existing branches in `T66RunStateSubsystem_Combat.cpp` may currently use `99999` or a normal-damage path. If Reflect/Counter currently OHKO bosses via the same `99999` line being replaced, the plan as written leaves them OHKO'ing bosses (contradicting the user clarification that OHKO must not affect bosses — though strictly Reflect/Counter were not named, the symmetry should be confirmed). Plan should either confirm Reflect/Counter against bosses already uses scaled non-OHKO damage, or call out the exact replacement value.

## Minor Issues

1. **`T66IsAccuracyFamilySecondaryStatType` Execute include / Accuracy exclude.** Plan should confirm no remaining hero-selection or base-accuracy math callsite filters via the family helper and then breaks when legacy `Accuracy` is removed from the family set. Evidence section notes hero head-targeting `Accuracy` math stays in `GetAccuracyChance01()` — confirm that path does not gate on `T66IsAccuracyFamilySecondaryStatType`.

2. **Defensive UI switch coverage is named but not enumerated.** Plan says "defensive UI switch updates" without naming the switches. List them (e.g., names/slugs/icons in `T66BuffSubsystem`, item-card text helpers, stats panel) so review can confirm none are missed.

3. **`UMETA(Hidden)` skip rationale.** Plan's reasoning that hiding could disrupt serialized assets is plausible but unverified — `UMETA(Hidden)` typically affects editor pickers, not serialization. If the only reason is editor-picker preservation for legacy authoring, say so; otherwise this is over-cautious.

4. **Backend payload key additions need a target file.** Plan says "Add payload keys for `Execute`, `LootBag`, and `LootWheel`" without naming the leaderboard/run-summary serialization site to patch. `UT66LeaderboardSubsystem` is named for enum-tail loop fixes but not for payload-key map additions.

5. **Vendor Token `IsRandomItemPoolEligible()` exclusion.** Plan says exclude canonical Vendor Token plus legacy alias from random pool — confirm this matches current Gambler Token behavior (it sounds like the prior code force-injected `Item_Accuracy` and excluded `Item_GamblersToken`; the new state should force-inject nothing accuracy-like and exclude Vendor Token canonical + legacy).

## Clarifying Questions

None require user input — all gaps are Codex-resolvable via repo inspection and plan tightening.

## Required Verification

1. Editor build with the cited `Build.bat` command; inspect output for `C4061`/`C4062`/`LogDataTable`/`Item_GamblersToken`/`Item_Accuracy`.
2. Text validation greps for the four required CSV inclusions and six required CSV exclusions, plus residual player-facing `Treasure Chest` strings and `Gambler's Token` strings.
3. Narrow git diff vs. the pre-existing dirty file list to confirm no unrelated reversions.
4. Runtime smoke covering: boss rejection of Execute/Assassinate/Crush; miniboss/enemy/mob acceptance; Loot Bag and Loot Wheel multiplier visible effect on locked reward; `Item_GamblersToken` legacy resolution producing Vendor Token row without missing-row warnings.
5. After concretizing the Loot Bag and Loot Wheel formulas, add an explicit table of input multiplier → resulting upgrade for at least two values (e.g., 1.0 and 2.0) so verification is unambiguous.

## Rationale

The plan is well-scoped, respects the user's OHKO/boss rule, defensibly handles Vendor Token canonicalization with a normalize-on-read boundary, correctly bounds Loot Bag/Loot Wheel scope to their interactables (matching Loot Crate/Loot Chest parity), and keeps `Item_TreasureChest`/`EvasionChance` as compatibility keys behind renamed player-facing text — that's the right move for serialized stability. The idol `TryExecuteTarget()` exclusion is grounded: it's a per-target boss-burst mechanic, not an item OHKO, so routing it through the new helper would change idol behavior. However, the plan has two underspecified reward-upgrade formulas (Loot Bag rarity, Loot Wheel quality), an incomplete CSV row schema for four new rows, and an ambiguous Reflect/Counter-vs-boss damage policy. These are all Codex-resolvable with another pass of inspection and tightening, so REVISE rather than NEEDS_HUMAN_DECISION. Once those are concretized and the defensive-switch list is enumerated, this is approve-ready.

