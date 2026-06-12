Verdict: APPROVE

## Blockers
None.

## Major Issues
- OHKO helper damage magnitude vs. damage reduction: the helper applies `MaxHP + CurrentHP + 1` through `TakeDamageFromHeroHitZone()`. If that path applies percentage-based armor/damage-reduction, a hardened target (e.g., armored miniboss or a tank mob) could survive what is supposed to be an unconditional OHKO. The old `99999` had the same latent bug, but the user-clarified rule "Execute/Assassinate/Crush kill minibosses" should be unconditional. Confirm whether `TakeDamageFromHeroHitZone()` short-circuits/clamps for OHKO sentinel values, or have the helper bypass mitigation (e.g., direct lethal kill API) so a miniboss with high DR cannot survive. The smoke test should explicitly include a high-DR miniboss case.

## Minor Issues
- Asymmetric multiplier semantics across reward surfaces: items use discrete tier breakpoints (`floor(Mult - 1.0)`), Chest/Wheel gold uses raw multiplication, Wheel boost multiplies only `BoostBonusStatPoints` (not duration). This is internally documented but produces a confusing player-facing curve where 1.0–1.99 Loot Bag/Wheel does nothing for items while every fractional step matters for gold. Recommend either (a) calling this out in MASTER_STATS so future tuning is grounded, or (b) confirming with the user before locking the breakpoint behavior as canonical.
- Scope creep risk in verification: adding a new `-T66ItemTaxonomySmoke=<path>` command-line hook is justified, but it is net-new test infrastructure inside a data/schema pass. Keep the hook narrowly scoped to the listed assertions, and add a pending-issues note if it must live on beyond this pass.
- `T66IsAccuracyFamilySecondaryStatType` change excludes legacy `Accuracy` while `GetAccuracyChance01()` still reads `GetSecondaryStatValue(Accuracy)`. The plan asserts hero head-targeting math doesn't depend on the family helper — good — but during implementation grep should explicitly confirm no other site uses `T66IsAccuracyFamilySecondaryStatType(Accuracy)` for hit-rate, telemetry, or hero-selection branches.
- `Item_VendorToken` reuses `Item_BackroomsQuickRevive` icon paths for every icon column. Two unrelated items sharing all four rarity icons may confuse HUD/inventory previews until art lands. Reasonable for this pass, but log a follow-up in `Content/Data/pending_issues_Data.md`.
- Save warn-and-skip policy for stale IDs (`Item_Accuracy`, `Item_Cheating`, `Item_Stealing`, `Item_HpRegen`, `Item_LifeSteal`) is mentioned but the exact validation site isn't named. Identify the inventory/save load entry point before implementation and confirm whether to refund/skip/silently drop; otherwise legacy saves may surface as silent slot loss.

## Clarifying Questions
None — all product-direction questions (OHKO scope vs. bosses, miniboss inclusion, Loot Bag/Wheel parity with Crate/Chest, Mini-exclusion of cleanup) were already resolved in the packet's user-clarification section.

## Required Verification
- Compile clean with `T66Editor Win64 Development` and inspect for `C4061`/`C4062` enum-switch warnings around new `Execute`, `LootBag`, `LootWheel`, `VendorToken` cases; treat any new warning as a missed switch site.
- Run `SetupItemsDataTable.py` against the updated CSV and confirm `DT_Items.uasset` regenerates with the new rows and old rows absent. Inspect editor output for `LogDataTable` warnings.
- Execute the new `-T66ItemTaxonomySmoke` hook covering: boss rejects Execute/Assassinate/Crush; miniboss + enemy + mob all OHKO including a high-DR miniboss; Reflect/Counter still damage bosses on the same combat path; Loot Bag and Loot Wheel multiplier tiers shift rarity/gold/boost as specified; `Item_GamblersToken` resolves to `Item_VendorToken` without missing-row warnings; stale `Item_Accuracy`/`Item_Cheating`/`Item_Stealing`/`Item_HpRegen`/`Item_LifeSteal` IDs are warn-and-skipped without crash.
- Narrow git diff review after implementation against the pre-existing dirty file list to confirm no unrelated modified file was reverted.
- Repo-wide grep for `ET66SecondaryStatType::Accuracy`, `T66IsAccuracyFamilySecondaryStatType`, and `static_cast<.*ET66SecondaryStatType` upper-bound loop sites after the enum append; confirm leaderboard, buff index, and stat panel iterations use `T66IsLiveSecondaryStatType()` or the new appended max.
- Grep that no new player-facing string contains "Gambler", "Treasure Chest" (outside compatibility row IDs/filenames), "Item_Accuracy", "Item_Cheating", "Item_Stealing", "Item_HpRegen", or "Item_LifeSteal".

## Rationale
The plan is well-grounded in concrete repo evidence: it correctly identifies that minibosses are `AT66EnemyBase` (not `AT66BossBase`), that idol `TryExecuteTarget()` is a separate boss-burst mechanic and should not route through the item OHKO helper, and that Loot Bag/Wheel parity belongs on the interactable consumption sites (`HandleSecondaryInteract` and `LockLootWheelReward`) rather than on world spawn or enemy-drop surfaces. Backend-key compatibility (`EvasionChance`, `TreasureChest`) and the legacy-ID resolver pattern (`NormalizeLegacyItemID`) are reused rather than re-invented — consistent with prior project discipline. Deprecation via `T66IsDeprecatedSecondaryStatType()` plus enum append (no reorder) protects serialization. The remaining concerns — OHKO interaction with damage reduction, multiplier curve asymmetry, and the new smoke harness footprint — are implementation- and verification-level, not plan-level, so Codex can proceed and prove them through the verification steps.

```
Plan file would be created at: C:\Users\DoPra\.claude\plans\you-are-claude-reviewing-majestic-wren.md
```

Note: I did not write a plan file because this invocation is a review of an external Codex packet, not a planning task — there is no implementation plan for me to author. If you want a plan file anyway summarizing this review, say the word.

