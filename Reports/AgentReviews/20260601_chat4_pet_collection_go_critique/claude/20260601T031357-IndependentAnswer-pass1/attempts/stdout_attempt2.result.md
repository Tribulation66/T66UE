Result: NEEDS_USER

## Independent Answer

The pet/Foundation wiring on the collector side is clean and the seam snaps onto the live reservation API — but the prompt's central premise (b), that "Economy (Chat 3) credits Mob Loot via its source-aware path," is **not present in the live tree**. That gap must be resolved by you before the implementation's proof is achievable.

**What is clean (confirmed against live source):**

- **(a) Seam snaps onto live signatures.** `AT66PetActor::TryFollowFoundationMobLootTarget` (T66PetActor.cpp:159) is a stub gated behind `bMobLootCollectionEnabled = false` (T66PetActor.h:45). The published API it must call exists and is usable as-is: `QueryAndReserveMobLoot(Filter, Collector)`, `CollectReservedMobLoot(Handle, Collector, MaxDistance)`, `ReleaseMobLootReservation(Handle, Collector)` (T66WorldSystemsAPI.h:122-125, T66MobLootSubsystem.h:121-124). `FT66MobLootCollectorRef` carries `CollectorType = Pet` (T66WorldSystemsAPI.h:77-87). No rework needed to wire claim→walk→collect.
- **(c) Release-on-unreachable works.** `ReleaseMobLootReservation` exists and is collector-matched (`IsCollectorMatch`), so the pet can release a drop it can't reach without permanently locking it.
- **Bond is movement-only — confirmed.** `UpdateMovementTuning()` (T66PetActor.cpp:71-89) applies `BondMultiplier` solely to `CurrentFollowSpeed`/`ReturnFollowSpeed`. It never touches amount, rate, radius, reservation eligibility, or value. Matches requirement (3).
- **(4) ExclusionSpheres expressible.** `FT66MobLootQueryFilter.ExclusionSpheres` (T66WorldSystemsAPI.h:100) is part of the filter the pet's query passes, so the future "ignore drops in hero radius" rule is a later filter-population change, not a rework. (Verify during impl that `IsEligibleForQuery` actually honors the spheres — declare it a behavior test, not a blocker.)

**The blocker — (b) collect-to-credit boundary:**

There is **no Mob Loot Economy credit path in live source.** Live gameplay only *spawns* drops on death (`SpawnMobLootFromNonBossDeath` at T66EnemyBase.cpp:1712, T66MobBase.cpp:570). The **only** callers of `CollectMobLootAt` / `CollectReservedMobLoot` are in `T66MobLootStressHarnessActor.cpp` (lines 195-268), which assert `GoldValueCollected` but never route it to run gold. `GoldValueCollected` appears only inside the subsystem and the harness — nothing in `Core/RunState` or any Economy file consumes it (grep for `MobLoot` in `Source\T66\Core` returns nothing).

So the proof you require — "gold landing via Economy's path" — cannot be produced today, because that path does not exist in the tree I can see. The Foundation side correctly does **not** credit gold (good, the pet stays a pure collector), but the receiving Economy side is absent.

**Implementation path (if you unblock (b)):**
1. In `TryFollowFoundationMobLootTarget`: build `FT66MobLootQueryFilter` (Origin = pet loc, SearchRadius, optional ExclusionSpheres), call `QueryAndReserveMobLoot` with a `Pet` collector ref; cache the returned `Handle`/`Position`.
2. Interp toward `Position` using `CurrentFollowSpeed` (bond-scaled); on arrival call `CollectReservedMobLoot(Handle, PetCollector, MaxDistance)`.
3. Route the returned `GoldValueCollected` into Economy's source-aware credit entry point (the piece that must exist first).
4. On unreachable/timeout/cancel, call `ReleaseMobLootReservation`.
5. FULL cook/stage (no `-SkipCook`); staged proof of reserve→walk→collect, gold via Economy, bond affecting only speed, and player-sweep-collects-0 against a pet-reserved drop.

## Evidence Checked
- T66WorldSystemsAPI.h (full) — API types, CollectorType, ExclusionSphere, CollectResult.
- T66MobLootSubsystem.h / .cpp — reservation/collect/release signatures, `GoldValueCollected` accumulation (line 690).
- T66PetActor.h / .cpp (full) — disabled seam, bond-movement-only tuning.
- Grep across Source for collect→credit consumers: only stress harness + spawn-on-death callers; no Economy/RunState Mob Loot crediting.

## Questions Or Blockers
- **Where is Chat 3's source-aware Economy credit path for Mob Loot?** It is not in the live tree (no `MobLoot` references in `Source\T66\Core`, no consumer of `GoldValueCollected` outside the stress harness). Is it on an unmerged branch, or not yet built? The required staged proof ("gold landing via Economy's path") is impossible until this exists — only you can confirm its state or authorize proceeding without it.

## Caveats
- I could not exhaustively prove a negative; I searched `Core`, `RunState`, Economy-named files, and all `GoldValueCollected` references. If the Economy entry point lives under a name without "MobLoot"/"Economy"/"Credit", point me to it and I'll re-validate (b).
- ExclusionSpheres confirmed only as a filter field passed through; whether `IsEligibleForQuery` currently applies it should be checked during implementation (test item, not a blocker).
- This is a read-only critique; no files were modified and no mutating/git commands were run.
