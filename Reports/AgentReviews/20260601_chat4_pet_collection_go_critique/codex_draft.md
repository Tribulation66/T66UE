Verdict: not clean to implement the full requested slice yet.

The Foundation collector side is now real and fits the pet seam, but the required collect-to-credit boundary is not present in live source. Implementation should stop before runtime edits until the user confirms where Chat 3's Economy source-aware Mob Loot credit path lives, or explicitly narrows this pass to collection-only without the required gold-credit proof.

Confirmed clean:
- The live API exists in `T66WorldSystemsAPI.h`: `FT66MobLootCollectorRef`, `ET66MobLootCollectorType::Pet`, `FT66MobLootQueryFilter`, `ExclusionSpheres`, `QueryAndReserveMobLoot`, `ReserveMobLoot`, `ReleaseMobLootReservation`, and `CollectReservedMobLoot`.
- `UT66MobLootSubsystem` implements the runtime and reservation mechanics.
- Reservation prevents another collector from sweeping the drop: `CollectMobLootAt` skips slots reserved by a different collector.
- `ReleaseMobLootReservation` is collector-matched and can be used on unreachable/cancel.
- `CollectReservedMobLoot` is collector-matched and distance-gated with `MaxDistance`.
- `ExclusionSpheres` is not just declared; `IsEligibleForQuery` rejects drops inside those spheres, so the future "ignore drops within hero radius" rule is a filter-population change.
- The pet seam can snap onto the API: `AT66PetActor::TryFollowFoundationMobLootTarget` is a disabled stub, and `SpawnPetForPlayer` still sets `bMobLootCollectionEnabled = false`.
- Bond is movement-only in current pet code: `UpdateMovementTuning` applies bond only to `CurrentFollowSpeed`/`ReturnFollowSpeed`; no live pet code touches amount, rate, radius, target count, eligibility, or value.
- Targeted search over `T66PetActor.h/.cpp` finds no loot-bag actor or loot-bag registry references.

Blocking issue:
- I found no live source-aware Economy credit path for Mob Loot. `CollectReservedMobLoot` and `CollectMobLootAt` return `FT66MobLootCollectResult::GoldValueCollected`; `ReleaseSlot` accumulates that value into the result and diagnostics, but does not credit RunState/Economy.
- The only live callers of `CollectReservedMobLoot` / `CollectMobLootAt` are the performance stress harness checks. They prove reservation/collection behavior but do not route the returned value to gold.
- `UT66RunStateSubsystem::AddGold` exists, but it is generic and logs `Source=Gambler`; it is not the source-aware Mob Loot credit path described in the prompt.

Implementation path after user unblocks credit:
1. Add pet reservation state to `AT66PetActor` (`ReservedMobLootHandle`, target position, timeout/no-progress tracking).
2. On tick, if enabled and no active reservation, build a `FT66MobLootQueryFilter` with pet origin/search radius and optional `ExclusionSpheres`, then call `QueryAndReserveMobLoot` using a `FT66MobLootCollectorRef` with `CollectorType=Pet`.
3. Move toward the reserved position using bond-scaled movement only.
4. On arrival, call `CollectReservedMobLoot(Handle, PetCollector, MaxDistance)`.
5. Hand the returned `GoldValueCollected` to the Economy-owned source-aware credit entry point once identified; do not call generic `AddGold` from pet code.
6. Release the reservation on timeout, no-progress/unreachable, owner loss, EndPlay, or target cancellation.
7. Turn `bMobLootCollectionEnabled` on for spawned active pets.

Question for user:
- Where is Chat 3's source-aware Mob Loot gold credit entry point in live code, or is it still unmerged? If it is not present, choose whether to wait for Chat 3 or explicitly approve a narrower collection-only implementation with gold-credit proof deferred.

Verification performed:
- Read root/process docs, role state, gameplay/report routers, pending issues.
- Read `T66WorldSystemsAPI.h`, `T66MobLootSubsystem.h/.cpp`, `T66PetActor.h/.cpp`, `T66GameMode_Spawning.cpp`, `T66AchievementsSubsystem`/`T66DataTypes` bond paths, stress harness, and RunState gold source.
- Searched live source for reservation/collection consumers, Mob Loot crediting, source-aware gold methods, and loot-bag references in pet files.
- Ran Claude independent read-only answer; it returned NEEDS_USER on the missing Economy credit path.
