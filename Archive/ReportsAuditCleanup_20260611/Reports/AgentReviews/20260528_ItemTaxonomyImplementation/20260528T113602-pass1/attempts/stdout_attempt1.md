Verdict: REVISE

## Blockers

None. No hard prerequisites are missing and no external state blocks the work.

## Major Issues

- **Loot Bag / Loot Wheel multiplier scope is broader than the user clarification.** The user said the stats improve rewards for the loot bag interactable and loot wheel interactable, "same concept as Loot Crate and Loot Chest." Before patching `T66EnemyBase.cpp` enemy loot bag drops or `T66ArcadeInteractableBase.cpp` arcade reward rarity, Codex must first read how Loot Crate / Loot Chest multipliers are currently applied and mirror exactly that surface. If Loot Crate does not boost enemy drops, Loot Bag must not either. Right now the plan extends Loot Bag into enemy drops and arcade rolls without proving parity, which can silently widen scope.
- **`Item_TreasureChest` → `Item_LootChest` rename is hedged ("if feasible").** This is a code/data investigation, not a user decision. Either canonicalize the ID (and add a legacy alias just like Vendor Token) or keep `TreasureChest` and only change display text. Decide before implementation; do not ship the ambiguity.
- **Central OHKO rule needs an explicit boss-suppression path inside the helper, not at the call sites.** Plan describes returning false for `AT66BossBase`, which is correct, but the existing `T66RunStateSubsystem_Combat.cpp` Assassinate/Crush path uses 99999 damage, so the helper must also gate the damage application, not just lethal logic. Confirm the helper actually replaces (not augments) the existing 99999 line and that reflected/counter damage to bosses is still routed through a non-OHKO branch.
- **Idol Execute relationship is left ambiguous.** The packet itself asks the reviewer about this; Codex should not enter implementation with a "preserve unless cheap" stance. Either (a) confirm idol Execute is mechanically distinct (boss burst, not OHKO) and explicitly exclude it from the helper with a one-line rationale, or (b) flag it to the user. Do not leave it as an implementation-time judgment call.
- **`GetAllSingleUseBuffTypes` source list edit risks UI fallout.** Removing HP Regen, Life Steal, Accuracy, Cheating, Stealing, and Alchemy from that list without verifying every consumer (temp-buff icon pickers, debug UI, stat-roll UI, leaderboard summaries) can drop or crash entries. Codex must enumerate consumers before editing.

## Minor Issues

- **Enum tail-of-list assumption.** Plan correctly flags `T66LeaderboardSubsystem` for the previous "Accuracy is last" loop assumption, but the same pattern can exist in any range-based or static-array consumer of `ET66SecondaryStatType`. Grep all `ET66SecondaryStatType::Accuracy` references for terminator usage before appending.
- **Vendor Token persistent member rename hedge.** Acceptable for save schema, but Codex should grep `ActiveGamblersTokenLevel` (and similar) and explicitly list every site where the old name remains internal-only versus where it leaks into logs, telemetry, or HUD strings. A list, not a hedge.
- **Alchemy stale-resolver side quest.** "Remove or compatibility-isolate" is open-ended and can expand scope. If Alchemy resolves cleanly by removal, do it; if it requires more than a deletion, defer to a separate pass and add a `pending_issues_*` entry.
- **No mention of `MASTER_PLAYER_EXPERIENCE.md` or other docs that surface item ordering.** Stats audit and MASTER_STATS are listed; confirm no other Gameplay/* doc enumerates the old order.
- **CSV ordering reliance is brittle.** Plan rewrites `Items.csv` in the requested order but doesn't say whether `DT_Items` runtime ordering actually honors CSV row order or sorts by key. If runtime sorts, this ordering is cosmetic-only and the actual order surfaces must be edited separately.

## Clarifying Questions

These are Codex-resolvable by re-reading the repo, not user-blocking:

- Does the current Loot Crate multiplier touch enemy drops or only the loot crate interactable? Mirror exactly what is found.
- Is there a single existing helper for reward rarity rolls that Loot Bag / Loot Wheel can hook through with one edit, or are the rolls fully duplicated per actor?
- Is `Item_TreasureChest` referenced by any save/serialization key, telemetry, or BP asset by name? That governs the rename feasibility question.

## Required Verification

The verification block is acceptable as a baseline but needs these additions before APPROVE on a re-review:

- **Boss-OHKO suppression must be runtime-proven, not just code-read.** Add a narrow PIE or staged check: spawn a boss, fire item Execute crit and Assassinate roll repeatedly, confirm boss HP decrements normally instead of going to 0. Without runtime evidence, "no boss OHKO" is unverified.
- **Miniboss OHKO must be runtime-proven.** Same shape: confirm miniboss death on Execute/Assassinate/Crush trigger.
- **Loot Bag and Loot Wheel multiplier wiring must show observable effect.** Set the secondary to a high value and verify reward rarity/quantity changes on the loot bag and loot wheel interactables. Without that, the wiring is theoretical.
- **Vendor Token legacy alias must be load-tested.** Load a save (or synthetic save fixture) that still references `Item_GamblersToken` and confirm it resolves to Vendor Token without missing-row warnings.
- **Editor compile is required, not optional**, and the exact `Build.bat` invocation is listed — good. Add: scan the compile log for warnings, not just success exit, since enum tail assumptions can compile with warnings.
- **Narrow git diff after implementation must be reviewed against the dirty-file list** to confirm no unrelated pre-existing modifications were reverted.

## Rationale

The plan is well-scoped on data, schema, and UI text, correctly defers art creation, correctly appends enum values rather than reordering, and correctly respects the HP Regen / Life Steal deferred-deletion pending issue. The OHKO central-rule intent is right. However, three substantive issues require Codex revision before implementation: (1) Loot Bag / Loot Wheel reward surfaces are expanded beyond the user's "interactable" clarification without proving parity with Loot Crate / Loot Chest, (2) the TreasureChest → LootChest decision and idol-Execute decision are hedged when they should be resolved by repo inspection, and (3) verification is too compile-centric — boss-suppression and multiplier-effect proofs need to be in-game observable. These are all within Codex's scope to fix by re-reading the repo and tightening the plan, so this is a REVISE rather than NEEDS_HUMAN_DECISION or BLOCK.

