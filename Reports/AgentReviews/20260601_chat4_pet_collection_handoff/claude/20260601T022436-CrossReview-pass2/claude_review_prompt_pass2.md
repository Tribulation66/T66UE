You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_handoff\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_handoff\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_handoff\claude\20260601T022224-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
CHAT 4 - PET COLLECTION HANDOFF (CRITIQUE FIRST, DO NOT IMPLEMENT YET)

Task contract:
Working task:
Operator: Codex
Validator: Claude
Scope: read-only critique of the live Foundation Mob Loot API against the existing pet seam and Economy crediting path; no implementation, no gameplay/source/data/config/save mutations.
Stop condition: report risks/fit/blockers and wait for explicit user go-ahead.

Original user prompt:
The Foundation API is live and proven. T66WorldSystemsAPI.h publishes the Mob Loot collection path (CollectMobLootAt and the target/reservation query). Foundation owns collection mechanics; the pet consumes them. This is the deferred pet-collection slice - the pet system, capture, save, and follow behavior already landed in the earlier non-seam pass with Mob Loot targeting disabled behind the seam.

Critique this plan against live code first - flag risks, confirm the surface - then wait for explicit go-ahead.

Scope:
1. Enable the active pet's Mob Loot targeting: claim/reserve a Mob Loot target via the published API, walk to it, and trigger collection when in range. It must NOT scan AT66LootBagPickup, use the loot-bag registry, or reimplement collection mechanics.
2. Confirm bond/union affects ONLY movement (speed/acceleration toward the target, maybe return speed) - never amount, rate, radius, target count, eligibility, or value. QoL/convenience only.

Confirm before implementing (proceed if clean, report and stop if not):
a) That Foundation's published API exposes a target claim/reservation suitable for the vacuum-pet (so two collectors - pet and player - don't double-claim or fight over the same drop), and how reservation/release works.
b) That triggering collection through the API correctly routes crediting to the Economy gold path (Chat 3 owns crediting/values). The pet COLLECTS via Foundation's API; it does NOT credit gold itself - confirm the pet is just the collector and the gold lands through Economy's path.
c) That the pet's follow/targeting behavior you built (disabled behind the seam) snaps onto the live API signatures as published - flag any mismatch between what you stubbed against and what shipped.

Coordination note: Chat 3 (Economy) owns Mob Loot values and gold crediting; you own the pet's collection CALL. You meet at Foundation's API. The pet must not assume it's the only collector, and must not credit gold itself.

New/changed content may require a FULL cook/stage, not -SkipCook. On go-ahead: implement, then a staged proof showing the active pet claiming and collecting Mob Loot through the API, gold landing via Economy's path, and bond affecting only movement speed. No git operations.

Critique first; implement only after I confirm.

Relevant repo rules:
- Follow AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator, Claude is Validator from .t66/operator-state.json.
- Mini/minigame scope is excluded.
- Claude must be read-only for validator work.
- Do not mutate code/data/assets/config/save files for this critique.
- No git operations.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The critique can be produced internally — but the plan's foundational premise does **not** match live code, so the honest critique outcome is **do not implement; report blockers and stop**. Codex can write this up; no user decision is required to *deliver the critique* (implementation is already correctly gated behind explicit go-ahead).

**The published surface contradicts the prompt on three of its load-bearing claims:**

1. **"Live and proven" is false.** `T66WorldSystemsAPI.h:14-16` states in its own doc comment: *"Mob Loot remains a later production subsystem… should compile against these signatures rather than inventing local placeholders."* It is a **compile-time contract**, not a live system. Critically, **nothing implements `IT66MobLootRuntime`** — a repo-wide grep finds zero `.cpp` defining `SpawnMobLoot` or `CollectMobLootAt`. There is no runtime to claim, walk to, or collect from yet.

2. **No target claim/reservation exists (confirm-point a FAILS).** The published interface (`T66WorldSystemsAPI.h:66-75`) exposes only `SpawnMobLoot`, `CollectMobLootAt(Center, Radius, MaxDrops)`, `GetActiveMobLootDropCount`, and `PushMobLootRenderArrays`. There is **no target query and no reservation/release primitive**. `CollectMobLootAt` is a **radius vacuum**, not a per-drop targeted claim. With only a radius primitive, two collectors (pet + player) cannot avoid double-claiming — the exact race the prompt's point (a) asks to prevent has no API support. This is the central blocker.

3. **Crediting does not auto-route to Economy (confirm-point b unconfirmed).** `CollectMobLootAt` returns `FT66MobLootCollectResult{ DropsCollected, QuantityCollected, GoldValueCollected }` (`:43-48`). It hands back a *gold value*; it does **not** credit Economy. So "gold lands through Economy's path" is **not** guaranteed by this surface — it depends entirely on Chat 3's caller wiring, which isn't visible here.

**Confirm-point (c):** The pet seam is a pure no-op. `T66PetActor.cpp:159-169` returns `false` with the comment *"wire claim/reserve/walk/CollectMobLootAt here once the API is published,"* and the flag is hard-disabled at both `T66PetActor.h:43-45` and `T66GameMode_Spawning.cpp:356`. So there's no stubbed-against signature to mismatch — but the seam was clearly built expecting a **claim/reserve** API that was never published. That is the mismatch to flag.

**Bond/union (scope item 2):** Movement-only constraint is sound and aligns with the live follow tuning (`ReturnFollowSpeed`/`GroundFollowSpeed`, `T66PetActor.h:37-41`). No live code couples bond to amount/rate/radius/eligibility, so this constraint is enforceable — *if/when* a real API exists.

**Bottom line for Codex:** Report that the plan cannot proceed as written — the reservation API premise is absent and the runtime is unimplemented. Surface to the user that unblocking requires a Foundation-side decision (extend the published API with a target/reservation primitive, or redefine the pet's collection model around the radius vacuum). Then stop, per the stated stop condition.

## Evidence Checked
- `Source/T66/Gameplay/T66WorldSystemsAPI.h` (full) — published interface, doc comment, result struct.
- `Source/T66/Gameplay/T66PetActor.h` / `.cpp:159-169` — disabled seam, no-op follow stub, flag.
- `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp:356` — pet spawned with collection disabled.
- Repo-wide grep for `IT66MobLootRuntime`/`CollectMobLootAt`/`SpawnMobLoot` implementations — **none found** in any `.cpp`.
- `T66MobManagerSubsystem.{h,cpp}` — no Mob Loot collection/reservation methods.
- `.t66/operator-state.json` — confirms Codex=Operator, Claude=Validator.

## Questions Or Blockers
- **Blocker (only the user/Foundation owner can resolve):** the published API has no claim/reservation primitive and no runtime implementation. Proceeding to implement the pet collector against it is impossible without a Foundation-side scope decision. This is correctly a *report-and-stop*, not a *fix-and-proceed*.
- No user question is forced for the critique itself — the deliverable is the critique, and it's complete.

## Caveats
- I read the live header and pet seam directly; the "no implementation" conclusion rests on a grep returning zero `.cpp` definitions — if the runtime lives behind a macro or generated code I didn't surface, re-confirm, but the header's own "later production subsystem" comment corroborates it.
- I did not inspect Chat 3's Economy crediting code; point (b)'s resolution depends on that caller and is out of this surface's scope.
- This is read-only validator output; no files inspected for mutation, no git actions.

</validator_independent_answer>

<codex_draft>
Result: critique only; do not implement yet.

Operator: Codex
Validator: Claude
Scope checked: live source/API surface for pet Mob Loot collection, existing pet seam, bond movement behavior, and Economy crediting boundary.

Draft assessment:
- The plan is not clean to proceed as written. The live `IT66MobLootRuntime` surface in `Source/T66/Gameplay/T66WorldSystemsAPI.h` exposes `SpawnMobLoot`, `CollectMobLootAt`, `GetActiveMobLootDropCount`, and `PushMobLootRenderArrays`; I do not see a target query, claim/reservation, or release method.
- The header still says Mob Loot is a later production subsystem and downstream streams should compile against the signatures. A whole-repo source search found no `.cpp` implementation of `IT66MobLootRuntime`, `SpawnMobLoot`, or `CollectMobLootAt` beyond the interface declaration.
- Confirm point (a) fails against live code: no reservation/release semantics are published, so the pet cannot safely claim one target while the player or another collector is also collecting. Implementing target ownership locally in the pet would violate the Foundation ownership rule.
- Confirm point (b) is not confirmed. `CollectMobLootAt` returns `FT66MobLootCollectResult` with `GoldValueCollected`; there is no visible implementation that credits `UT66RunStateSubsystem::AddGold` or another Economy-owned path. The existing Economy AddGold path exists, but pet should not call it directly under this plan.
- Confirm point (c) mismatches: `AT66PetActor::TryFollowFoundationMobLootTarget` is still a no-op behind `bMobLootCollectionEnabled`, and `AT66GameMode::SpawnPetForPlayer` explicitly sets `bMobLootCollectionEnabled = false`. The seam expected claim/reserve/walk/CollectMobLootAt signatures; the current live API does not provide the claim/reserve part.
- Bond/union movement-only is consistent with live pet code. `FPetData` comments and fields limit bond tuning to fetch movement speed, `AT66PetActor::UpdateMovementTuning` applies bond only to `CurrentFollowSpeed`/`ReturnFollowSpeed`, and no pet code currently touches amount, rate, radius, target count, eligibility, value, or loot-bag collection.
- The pet actor currently has no direct loot-bag references, and a targeted search over `T66PetActor.h/.cpp` for `AT66LootBagPickup`, `LootBagRegistry`, `GetLootBags`, `NearbyLootBag`, and `LootBag` produced no matches.

Recommended answer:
- Report and stop. The implementation should wait until Foundation either publishes the missing reservation/target API and runtime implementation or explicitly approves a changed collection model around `CollectMobLootAt` radius vacuum semantics.
- Do not reimplement target discovery/reservation in Chat 4 and do not credit gold from the pet.

Verification performed:
- Read `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `.t66/operator-state.json`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/README.md`, `Gameplay/World/WORLD_AGENTS.md`, and relevant pending issues.
- Read `T66WorldSystemsAPI.h`, `T66PetActor.h/.cpp`, `T66GameMode_Spawning.cpp`, `T66AchievementsSubsystem.h/.cpp`, `T66DataTypes.h`, `T66StageGate.cpp`, and Economy AddGold source.
- Repo/source searches for Mob Loot API implementation and reservation/claim/release terms.
- Claude independent answer was run read-only through `Invoke-ClaudePlanReview.ps1`.

</codex_draft>
