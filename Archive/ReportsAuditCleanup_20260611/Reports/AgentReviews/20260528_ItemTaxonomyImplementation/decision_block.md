# Item Taxonomy Decision Block

## Working Goal

Implement the main-run item taxonomy/data changes: canonical Vendor Token naming with no Gambler Token mention, data order updates for Accuracy/Evasion/Armor, Execute replacing secondary Accuracy, Chance suffix display names where requested, and refreshed runtime data assets/verification.

## Status

Implementation is paused before code/data edits because the required Claude review returned `Verdict: REVISE` for user-intent decisions that affect gameplay semantics and live-save behavior.

Latest review artifact:

- `Reports/AgentReviews/20260528_ItemTaxonomyImplementation/20260528T103015-pass3/claude_review_pass3.md`

## Decisions Needed

1. Execute Chance boss behavior
   - Decision needed: Should Execute Chance be allowed to OHKO awakened bosses?
   - Conservative default if the user says `no clarification needed`: exclude bosses from Execute OHKO; Execute OHKO applies to normal enemies and mobs only.

2. Deprecated Luck item aliases
   - Decision needed: Should old owned `Item_Cheating` normalize to `Item_LootWheel`, and old owned `Item_Stealing` normalize to `Item_LootBag`?
   - Conservative default if the user says `no clarification needed`: yes, normalize old owned items to the closest new Luck rows so existing saves do not lose inventory slots.

3. Deprecated Accuracy item behavior
   - Decision needed: Should old owned `Item_Accuracy` normalize to `Item_ExecuteChance`, changing those old items from head-targeting accuracy bonus to Execute Chance?
   - Conservative default if the user says `no clarification needed`: yes, normalize `Item_Accuracy` to `Item_ExecuteChance` because the user requested replacing the secondary Accuracy item stat with Execute.

4. Headshot passive value
   - Decision needed: Should the Headshot passive keep its current `+0.20` head-targeting bonus after removing item-facing secondary Accuracy?
   - Conservative default if the user says `no clarification needed`: yes, preserve Headshot's existing `+0.20` value and only remove the old item secondary contribution.

## Implementation Can Resume When

- The user answers the decisions above, or says `no clarification needed` to accept the defaults, or says `skip Claude review` to bypass the project review gate for this request.
- After answers are available, update `review_packet.md` with those decisions and rerun `Scripts/Invoke-ClaudePlanReview.ps1`.
- If the next review returns `Verdict: APPROVE`, implementation may proceed under the reviewed scope.

