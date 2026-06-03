# Decision Block: Altar Spawn and Idol Slot Changes

Date: 2026-05-31  
Operator: Codex  
Validator: Claude  
Validator verdict: NEEDS_HUMAN_DECISION  
Report: `Reports/AgentReviews/20260531_AltarSpawnIdolSlotAudit/altar_spawn_idol_slot_audit.md`  
Validation: `Reports/AgentReviews/20260531_AltarSpawnIdolSlotAudit/ClaudeValidation/20260530T234356-pass1/claude_review_pass1.md`

## Decision Required

The audit is complete as a read-only report, but implementation cannot start until the user decides these design/routing points.

## Choices

1. Stage 4 structure:
   - Option A: local stage 4 stops using the current 2-floor boss-rush finale and becomes the same 5-floor tower shape as stages 1-3, with floor 1 start, floors 2-4 miniboss floors, and floor 5 final boss.
   - Option B: keep the current 2-floor boss-rush finale and reach the 16-idol target through another reward route.

2. Floor 1 "only weapon altar" meaning:
   - Option A: only reward/utility interactables are excluded; the required tower descent/progression hole may remain.
   - Option B: literally no other interactable/progression actor may be on floor 1, including the descent/progression hole, so traversal needs another path.

3. Rarity cadence:
   - Option A: local stage 1 = black, local stage 2 = red, local stage 3 = yellow, local stage 4 = white for both weapons and idols.
   - Option B: use a different rarity mapping.

4. Idol cap and combat throughput:
   - Option A: set equipped/held idol capacity to 16 and include the resulting combat/projectile throughput implications in the implementation scope.
   - Option B: set equipped/held idol capacity to 16 but defer combat throughput handling to the separate projectile/rendering pass.
   - Option C: 16 altar visits do not mean 16 equipped idols; define a different held/equipped model.

## Stop Condition

Wait for the user's answers before implementation planning or edits.
