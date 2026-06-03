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
