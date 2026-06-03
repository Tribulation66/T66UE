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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_option3_impl\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_option3_impl\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_option3_impl\20260601T032911-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
CHAT 4 -- DECISION: Option 3, with refinement. Proceed on the Foundation-side pet work now; defer only the stack-deposit proof.

Important boundary clarification that simplifies your blocker: credit timing is now locked as CREDIT-ON-SELL (Chat 3's decision). That means:
- The pet NEVER touches gold, and collection does NOT credit gold. Collection grows Economy's 999-capped COLLECTED STACK; gold is credited only later, at the vendor Sell card. So the "source-aware Economy gold credit path" is NOT on the pet's critical path at all -- the pet's job ends at depositing the collected drop into Economy's collected stack.
- Do NOT have the pet call AddGold or any gold path. The pet reserves, walks, collects via Foundation, and the collected value flows into Economy's collected stack. The vendor sale (Chat 3) is what credits gold, entirely outside the pet.

Build NOW (fully unblocked against proven Foundation API):
- The active pet's reserve/walk/collect/release loop: QueryAndReserveMobLoot (CollectorType=Pet), walk to the reserved drop, CollectReservedMobLoot in range, ReleaseMobLootReservation on unreachable/cancel. Confirmed all present and proven in T66MobLootSubsystem.
- Bond affects ONLY movement speed (already so in your code) -- keep it.
- Pass an ExclusionSphere through the query filter even if empty, so the future "ignore drops in hero radius" rule is a later filter change, not a rework. (Foundation already honors ExclusionSpheres.)
- No loot-bag actor/registry references (already clean).

DEFER only this: the end-to-end proof that pet collection GROWS ECONOMY'S COLLECTED STACK. That stack is greenfield Economy work Chat 3 is landing now. So:
- Build the pet so its collected drops deposit into Economy's collected-stack entry point. If that entry point isn't in live source yet, build against the documented boundary (collection returns value to Economy's stack), and PROVE the Foundation side now: pet reserves, walks, collects, releases, reservation blocks the player from the same drop, bond affects only speed.
- The full "pet collection -> collected stack grows -> sell credits gold" proof resumes once Chat 3's collected-stack path is live. Flag it as the deferred proof, don't fake it.

New content needs a FULL cook/stage, not -SkipCook. No git operations. Proceed on the pet behavior + Foundation-side proof now; defer the stack-growth proof to when Chat 3 lands.

</original_prompt>

<validator_independent_answer>
All technical claims verified. Here is my independent answer.

Result: OK

## Independent Answer

Proceed exactly as the prompt directs. The decision is the user's to make and it's already made (Option 3 with refinement, CREDIT-ON-SELL locked), so the models can execute internally — no further user input is needed.

The implementation Codex should land now:

1. **Replace the collect-loop stub.** `AT66PetActor::TryFollowFoundationMobLootTarget` (`T66PetActor.cpp:159-169`) is currently a no-op that returns `false`. Wire the real loop here:
   - `QueryAndReserveMobLoot(Filter, Collector)` with `Collector.CollectorType = ET66MobLootCollectorType::Pet`.
   - Walk to the reserved drop using existing movement (the `FVInterpTo`/ground-trace logic already in `Tick`).
   - `CollectReservedMobLoot(Handle, Collector, MaxDistance)` once in range.
   - `ReleaseMobLootReservation(Handle, Collector)` on unreachable/cancel.
2. **Pass an `ExclusionSphere` through the query filter even when empty** so the future hero-radius rule is a filter tweak, not a rework. Foundation honors `ExclusionSphere`s (`T66MobLootSubsystem.cpp`), so populate the field on `FT66MobLootQueryFilter` rather than leaving it unset.
3. **Keep bond → movement speed only.** `UpdateMovementTuning` (`T66PetActor.cpp:71-89`) already does exactly this via `BondFetchSpeedPerStage`/`MaxBondFetchSpeedMultiplier`. Don't touch the collect path with bond.
4. **No gold path.** Correct per CREDIT-ON-SELL — the pet must not call `AddGold` or any gold credit. Confirmed the gold/sell credit lives in the vendor flow (`SellInventoryItemAt`, `T66RunStateSubsystem_EconomyInventory.cpp:740-756`), entirely outside the pet.
5. **Deposit into Economy's collected stack via the documented boundary.** I confirmed no collected-stack entry point exists in live Economy source yet (only the existing inventory/sell value path). So build the pet's deposit against the documented boundary and **defer the stack-growth proof** as instructed — do not fabricate it.

**Foundation-side proof to capture now** (full cook/stage, not `-SkipCook`, no git ops): pet reserves → walks → collects → releases; the Pet reservation blocks the player from the same drop; bond changes speed only. Flag the "collection → collected stack grows → sell credits gold" end-to-end proof as explicitly deferred until Chat 3's collected-stack path lands.

## Evidence Checked
- `T66WorldSystemsAPI.h:22-26,82` — `ET66MobLootCollectorType { Player, Pet, System }` and `CollectorType` field exist.
- `T66MobLootSubsystem.h:121-124` — `QueryAndReserveMobLoot`, `ReleaseMobLootReservation`, `CollectReservedMobLoot(... MaxDistance)` all present.
- `ExclusionSphere` referenced in `T66MobLootSubsystem.cpp` — Foundation honors it.
- `T66PetActor.cpp:71-89` — bond affects only `CurrentFollowSpeed`/`ReturnFollowSpeed`.
- `T66PetActor.cpp:159-169` — collect loop is a stub returning `false`; no loot-bag refs.
- `T66RunStateSubsystem_EconomyInventory.cpp:740-756` — gold credited at Sell, outside the pet; no collected-stack entry point yet in live source.

## Questions Or Blockers
None requiring the user. The single dependency (Economy's collected-stack entry point) is a known Chat 3 deliverable the prompt already accounts for by deferring that one proof.

## Caveats
- Codex should confirm the exact field name/shape of `ExclusionSphere` on `FT66MobLootQueryFilter` before populating it, so the empty pass-through compiles against the real struct.
- The pet's "deposit into collected stack" call is necessarily written against a documented boundary, not a live symbol — Codex should keep that seam narrow and clearly marked so the deferred end-to-end proof can attach cleanly when Chat 3 lands.
- "Reservation blocks the player from the same drop" should be proven with an actual Player-collector query against the Pet-reserved handle, not assumed from the API contract.

</validator_independent_answer>

<codex_draft>
# Codex Draft - Chat 4 Pet Collection Option 3

## Task Contract

Working task:
- Operator: Codex
- Validator: Claude
- Scope: implement the active pet's Foundation-side Mob Loot reserve/walk/collect/release loop, keep pet collection away from gold crediting, preserve bond-as-movement-only behavior, pass an ExclusionSpheres query field for future hero-radius ignores, avoid loot-bag actors/registries, full cook/stage and staged proof, no git operations.
- Stop condition: Foundation-side pet collection is implemented and proved in staged standalone, or a blocker is reported.

## Implementation Summary

- `Source/T66/Gameplay/T66PetActor.h` and `Source/T66/Gameplay/T66PetActor.cpp`
  - Enabled real Foundation Mob Loot behavior behind `bMobLootCollectionEnabled`.
  - Uses `QueryAndReserveMobLoot` with `ET66MobLootCollectorType::Pet`.
  - Keeps `FT66MobLootQueryFilter::ExclusionSpheres` explicitly on the pet query path, currently empty for the future hero-radius ignore rule.
  - Walks toward the reserved target, calls `CollectReservedMobLoot` when the pet reaches the target, and calls `ReleaseMobLootReservation` when collection is disabled, no-progress timeout expires, max reservation age expires, or the actor ends play.
  - Uses planar distance for the pet's own walk/range gate, then calls Foundation's reserved-collection API. This avoids a ground-following pet missing source/capsule-height drops because Foundation's optional `MaxDistance` check is 3D actor-to-drop distance.
  - After Foundation returns the collection result, forwards it to `UT66RunStateSubsystem::AddCollectedMobLootFromCollection` when available. The pet never calls `AddGold`.
  - Bond still only feeds movement speed through `UpdateMovementTuning` / `ComputeFetchSpeedForBondStagesForAutomation`; it does not alter value, amount, eligibility, collection radius, target count, or sell behavior.

- `Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp`
  - Spawned active pets now set `bMobLootCollectionEnabled = true`.

- `Source/T66/PerformanceSystem/T66MobLootStressHarnessActor.h` and `.cpp`
  - Extended the Mob Loot proof harness with a real `AT66PetActor` functional check:
    - pet reserves a drop,
    - player sweep is blocked from the reserved drop,
    - pet walks to and collects the reserved drop,
    - pet collection grows the collected Mob Loot stack when the Economy endpoint is live,
    - cancellation/destroy releases reservation and lets the player collect the released drop,
    - bond affects only movement speed,
    - ExclusionSpheres seam is present.
  - Manifest keeps explicit fields for no loot-bag use and no gold-on-collection.

- `Source/T66Editor/T66OutgoingTravelerSwarmVFXCommandlet.cpp`
  - Fixed an unrelated UE 5.7 compile blocker by replacing private `Material->ShadingModels` assignment with public `Material->SetShadingModel(MSM_Unlit)`.

## Boundary Checks

- No pet code calls `AddGold`.
- Narrow source search found no `AT66LootBagPickup`, loot-bag registry, or `AddGold` references in the pet/Foundation files touched.
- Manifest reports:
  - `uses_at66_loot_bag_pickup=false`
  - `subsystem_calls_add_gold=false`
  - `economy_collection_did_not_credit_gold=true`
  - `economy_sale_credited_gold=true`

## Verification

- Focused editor compile:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  - Result: succeeded.

- Full staged standalone refresh:
  - `Scripts\StageStandaloneBuild.ps1`
  - Result: succeeded.
  - Refreshed executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Verified both standalone shortcuts target that executable:
    - `C:\UE\T66\T66 Standalone.lnk`
    - `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`

- Packaged proof:
  - Executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Manifest: `C:\UE\T66\Saved\Proofs\PetMobLootFoundation\pet_mob_loot_foundation_manifest.json`
  - Log: `C:\UE\T66\Saved\Proofs\PetMobLootFoundation\pet_mob_loot_foundation.log`
  - Screenshot: `C:\UE\T66\Saved\Proofs\PetMobLootFoundation\pet_mob_loot_foundation.png`
  - Result: `pass=true`, `foundation_pet_actor_pass=true`, `economy_contract_pass=true`.
  - Key proof fields:
    - `reservation_prevented_double_collect=true`
    - `reserved_pet_collect_succeeded=true`
    - `pet_actor_reserved_mob_loot=true`
    - `pet_actor_walked_to_reserved_mob_loot=true`
    - `pet_actor_collected_reserved_mob_loot=true`
    - `pet_actor_player_blocked_from_reserved_drop=true`
    - `pet_actor_release_on_cancel_succeeded=true`
    - `pet_actor_query_uses_exclusion_spheres_seam=true`
    - `pet_bond_affects_movement_speed_only=true`
    - `pet_actor_collect_drops=1`
    - `pet_actor_collect_quantity=5`
    - `pet_actor_collect_sell_value=5`
    - `pet_actor_economy_stack_after_collect=5`

## Notes For Cross-Review

- The user allowed deferring the collected-stack proof if Chat 3's stack was not live. It is live in current source, so I proved that path too. The pet still does not credit gold; sale credit remains Economy-owned.
- The planar pet range gate is deliberate: Foundation's optional `CollectReservedMobLoot` distance parameter checks 3D actor/drop distance. A ground-following pet can be correctly at the drop in X/Y while the drop retains source height, so the pet gates by planar walk distance and then invokes Foundation's reserved collection with no Foundation max-distance override.
- No git operations were run.

</codex_draft>
