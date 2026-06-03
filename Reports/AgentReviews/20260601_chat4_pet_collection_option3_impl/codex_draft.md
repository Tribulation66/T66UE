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
