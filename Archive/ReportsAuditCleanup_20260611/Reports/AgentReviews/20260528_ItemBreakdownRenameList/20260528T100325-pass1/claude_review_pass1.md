Verdict: REVISE

## Blockers
- None.

## Major Issues
- **Unspecified intra-group reordering beyond the stated rule.** The user instruction was only that Execute / Assassinate / Crush move to the end of their groups. The proposed Evasion order (Dodge, Counter Attack, Invisibility, Assassinate) silently swaps Invisibility and Counter Attack and promotes Dodge from position 4 to position 1. The proposed Armor order (Taunt, Damage Reduction, Reflect Damage, Crush) silently promotes Damage Reduction from position 4 to position 2 and demotes Reflect Damage. CSV order for Evasion is Invisibility, CounterAttack, Assassinate, EvasionChance; CSV order for Armor is Taunt, ReflectDamage, Crush, DamageReduction. The packet should either (a) preserve the surviving CSV order with only the named item moved to end, or (b) flag the additional reshuffles as an explicit recommendation needing confirmation.
- **"Vendor Token / Gambler's Token" naming is ambiguous.** The live evidence section lists only `Item_GamblersToken` (synthetic). Presenting it as "Vendor Token / Gambler's Token" implies either a dual display name or two items. The packet does not resolve which, and it is a user-facing taxonomy answer.

## Minor Issues
- The Risks/Caveats correctly note LootBag and LootWheel are not in the CSV today, but the Proposed Final List presents them with no inline marker. A reader scanning only the list will read them as currently active. Consider an inline "(not yet in items data)" marker next to Loot Bag and Loot Wheel, matching the existing inline note style used for Dodge and Loot Chest.
- Accuracy group lists Execute with a definition appended ("chance for a critical hit to OHKO the enemy") while other secondaries are bare names. Either annotate all renamed items consistently or strip the inline definition for parallel formatting.

## Clarifying Questions
- For Evasion and Armor, should items other than Assassinate/Crush keep their current CSV relative order, or is the proposed reshuffle (Dodge first, Damage Reduction second) intended?
- Is "Vendor Token / Gambler's Token" one item with two display names or two distinct Special items? The repo currently has only `Item_GamblersToken`.
- Should Loot Bag and Loot Wheel carry an explicit "intended, not yet in data" marker inline?

## Required Verification
- After clarifying ordering and Vendor/Gambler's Token, the deliverable is text-only with a one-line note that no source files were changed. No code, build, or test verification is needed for this turn.

## Rationale
The packet is correctly scoped (main-run only, no Mini, no HP Regen / Life Steal), uses the repo spelling Assassinate, surfaces the right live evidence (`Items.csv`, `T66DataTypes.h` Dodge UMETA, `Item_BackroomsQuickRevive`, `Item_GamblersToken`), and flags the right caveats on LootBag/LootWheel/LootChest. The remaining issue is that it silently makes ordering and naming choices the user did not authorize — exactly the kind of surprise the answer-only deliverable should avoid. Resolve the ordering and token-name questions, then this is safe to present.

