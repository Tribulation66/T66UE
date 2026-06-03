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
