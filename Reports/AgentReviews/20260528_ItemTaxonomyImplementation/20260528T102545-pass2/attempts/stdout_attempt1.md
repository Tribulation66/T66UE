Verdict: REVISE

## Blockers

None that prevent the plan from being workable, but the items in Major Issues should be resolved before Codex executes.

## Major Issues

- **Boss exclusion from Execute Chance is a unilateral design call.** User said Execute is "chance of a critical hit OHKO the enemy." The plan reads "enemy" as "non-boss enemy" and silently excludes bosses. That is a gameplay-balance decision, not a clean technical inference — it must be a clarifying question to the user, not an in-plan default.
- **Hidden behavior change to hero accuracy is not called out.** Today `UT66RunStateSubsystem::GetAccuracyChance01()` adds Headshot's +0.20 *plus* the `Accuracy` secondary contribution. Removing the item-facing `Accuracy` secondary without otherwise compensating means baseline player accuracy chance silently drops. The plan describes the refactor neutrally ("preserving target-preference behavior") but does not acknowledge the numeric drop or whether base accuracy / Headshot magnitude should be re-tuned. This needs to be made explicit and either accepted or compensated.
- **Verification does not actually exercise Execute Chance.** Step 7 only smokes "menu/log startup," and Step 8 makes the actual gameplay proof conditional on an existing automation hook, otherwise downgraded to "report as skipped." Since Execute is the only new gameplay-affecting behavior in this pass, that gameplay path must be exercised at least once (manual standalone run granting `Item_ExecuteChance`, forcing crits, observing OHKO on a normal mob and non-OHKO on whatever the answer to the boss question is). "Skipped" is not acceptable for the one new combat mechanic in scope.

## Minor Issues

- `MASTER_PLAYER_EXPERIENCE.md` is on the touched-doc shortlist per repo norms when player-facing item taxonomy and display names move; the plan only lists `MASTER_STATS.md` and `MASTER_COMBAT.md`. Worth checking whether it references the old names.
- Placeholder sprite reuse means `Item_LootBag` shares the LootCrate icon and `Item_LootWheel` shares the TreasureChest icon. That is two pairs of visually identical items in the same Luck row. Acceptable for this pass, but the plan should either flag a follow-up pending-issue entry or explicitly note that it will, so this doesn't get lost.
- Reusing the existing idol `Execute` damage event label for the new item Execute is reasonable, but if anything (analytics, floating text rules, idol-specific handling) currently keys off that label assuming idol-only origin, it will start firing for item Execute too. Worth a one-line confirmation that nothing downstream treats the existing Execute event as idol-exclusive.
- Pending issues touched in the working tree (`pending_issues_Combat.md`, `pending_issues_Data.md`, etc.) should be reconciled — not just the one stale `Item_GamblersToken` warning — to confirm no other entry contradicts this taxonomy.
- "Deterministic placeholder sprite references" claim that all of `/Game/Items/Sprites/Item_Accuracy_*`, `Item_EvasionChance_*`, `Item_TreasureChest_*`, `Item_LootCrate_*` exist at those exact package paths. Live evidence section asserts the CSV rows exist; it does not assert the sprite paths. Should be confirmed before CSV authoring, else `SetupItemsDataTable.py` may bake broken refs.

## Clarifying Questions

1. Does Execute Chance OHKO bosses? Default in plan is no — please confirm or correct.
2. Is the net drop in hero accuracy (from removing item-facing `Accuracy` secondary contribution while Headshot's +0.20 stays) intended, or should base accuracy or Headshot's magnitude be retuned in this pass?
3. For Luck, are `Item_LootBag` and `Item_LootWheel` meant to be live droppable/purchasable items in this pass, or schema-only placeholders? Affects whether shared placeholder icons are acceptable for ship.
4. Should `Item_Cheating` -> `Item_LootWheel` and `Item_Stealing` -> `Item_LootBag` legacy aliases preserve the *old item's effect numbers* on owned-but-renamed saves, or fully adopt the new row's stat config? Plan implies the latter but doesn't say.

## Required Verification

- `Build.bat T66Editor Win64 Development` clean (as planned).
- `SetupItemsDataTable.py` reload via `UnrealEditor-Cmd.exe` (as planned), with log inspection for missing-asset warnings on the placeholder sprite paths.
- `StageStandaloneBuild.ps1` clean (as planned).
- **Staged standalone gameplay smoke**, not just menu/log: grant `Item_ExecuteChance`, force crits, confirm OHKO on a normal mob and confirm the agreed boss behavior. If no automation hook exists, this is manual but must still be performed in this pass — the plan should commit to it rather than mark it skip-eligible.
- Legacy-save sanity check: load a save containing `Item_Accuracy` and `Item_GamblersToken` and confirm they appear as `Item_ExecuteChance` and `Item_VendorToken` with no inventory loss.
- Grep checks (as planned) for player-facing `Gambler` strings and active CSV row order.

## Rationale

The structural plan is sound: enum append-only for serialization safety, CSV row reorder for canonical breakdown order, legacy ID aliasing for save compatibility, Vendor Token rename limited to player-facing text without renaming serialized C++ identifiers, MASTER_STATS doc update, and a real compile + DT reload + staged-build pass. Those parts are well-aligned with `AGENTS.md`, `GAMEPLAY_AGENTS.md`, and `MASTER_STATS.md` discipline.

What pushes this from APPROVE to REVISE is that the only genuinely new gameplay mechanic in scope — Execute Chance — has both an unresolved design question (boss inclusion) and a verification plan that explicitly allows the gameplay proof to be downgraded to "skipped." Combined with the unacknowledged hero accuracy regression from dropping the `Accuracy` secondary, the plan as written could ship a silent combat-balance shift behind a passing build. These are addressable with a short clarification round and a firmer verification commitment, not a redesign, so REVISE rather than BLOCK.

