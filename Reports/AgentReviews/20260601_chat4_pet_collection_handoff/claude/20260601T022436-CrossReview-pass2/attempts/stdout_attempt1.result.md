Result: OK

## Summary
The Codex draft and my independent answer converge cleanly: the plan's premise ("live and proven," with a claim/reservation API) does not match the published surface, so the correct critique outcome is **report and stop**. Codex can deliver this critique internally — implementation is already gated behind explicit go-ahead, so no user decision is required to produce the deliverable. The draft is accurate, well-evidenced, and on-scope. A few wording tightenings below.

## Suggested Answer Patch
- Fix the top line. `Result: critique only; do not implement yet.` is fine as a posture statement but bury the lede less — lead with the verdict: *"Verdict: plan not clean — report and stop. The reservation-API premise is absent and the Mob Loot runtime is unimplemented."*
- Strengthen the "live and proven" rebuttal explicitly. The draft notes the header's "later production subsystem" comment but never directly contradicts the prompt's "live and proven" claim. Add one sentence: *"The prompt's 'live and proven' framing is incorrect — this is a compile-time signature contract, not a running system."* This is the load-bearing correction.
- On point (b), tighten the wording. Draft says "there is no visible implementation that credits AddGold." Better: state that `CollectMobLootAt` *returns* a gold value to its caller and does not itself credit Economy, so auto-routing is not guaranteed by this surface and depends on Chat 3's caller wiring. (Same conclusion, sharper causality.)

## Issues To Fix
- Point (b) evidence asymmetry: Codex says it read "Economy AddGold source" and confirms `UT66RunStateSubsystem::AddGold` exists. Good — but the draft should explicitly say it found **no caller wiring** that routes `GoldValueCollected` into that path (rather than just "no visible implementation that credits"). Confirm the search actually covered callers of `CollectMobLootAt`, or soften to "not confirmed within this surface."
- Minor: the draft asserts the seam "expected claim/reserve/walk/CollectMobLootAt signatures." That's an inference from the no-op comment. Fine to state, but mark it as inferred from the `TryFollowFoundationMobLootTarget` comment, not a published contract.

## Question For User
None forced. The critique is the deliverable and it's complete. The implementation blocker (no reservation primitive / no runtime) is correctly surfaced as a Foundation-side scope decision for the user to resolve **before** any go-ahead — but that's reported, not asked, per the stop condition.

## Evidence Or Verification Gaps
- Both Codex and I rest the "unimplemented" conclusion on a grep returning zero `.cpp` definitions of `IT66MobLootRuntime`/`CollectMobLootAt`/`SpawnMobLoot`. Corroborated by the header's own comment, so low risk — but note macro/generated-code possibility as a caveat (Codex omits this caveat; my independent answer included it). Suggest Codex add the one-line caveat.
- Codex claims broader reads (Achievements, StageGate, DataTypes) than I performed; none change the verdict, but they're not load-bearing for the three confirm-points — fine to keep but don't let them pad the answer.

## Notes
- Bond/union movement-only finding is consistent across both passes and well-supported (`UpdateMovementTuning` touches only follow speeds). Enforceable constraint — keep as written.
- Loot-bag negative search (`AT66LootBagPickup`, `LootBagRegistry`, etc.) is a nice positive confirmation that the seam respects the no-scan rule; keep it.
- No mutations, no git, read-only — both passes comply with the validator protocol.
