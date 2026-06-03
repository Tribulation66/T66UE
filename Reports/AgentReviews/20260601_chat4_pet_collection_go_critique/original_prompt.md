CHAT 4 - PET COLLECTION: GO (Foundation Phase 3 is implemented and proven)

Task contract:
Working task:
Operator: Codex
Validator: Claude
Scope: read-only critique of the current live Foundation reservation API against the existing pet seam and Economy boundary; no implementation, no gameplay/source/data/config/save mutations until the user explicitly confirms after this critique.
Stop condition: report whether wiring is clean, risks/blockers, and the exact implementation path if approved.

Original user prompt:
The Mob Loot subsystem with reservation is now real and proven. The reservation API exists: QueryAndReserveMobLoot / ReserveMobLoot / ReleaseMobLootReservation / CollectReservedMobLoot, with FT66MobLootCollectorRef (CollectorType Player/Pet/System) and ExclusionSpheres filtering. Reservation-prevents-double-collection is proven (pet reserved a drop; player sweep collected 0 from it). This unblocks your deferred pet-collection slice. Critique the wiring first, confirm, then proceed if clean.

Scope:
1. Enable the active pet's Mob Loot collection: use QueryAndReserveMobLoot to claim a target (CollectorType=Pet), walk to it, and CollectReservedMobLoot in range. The pet must NOT scan AT66LootBagPickup, use the loot-bag registry, or reimplement collection. Release the reservation if it can't reach the drop.
2. The pet COLLECTS but does NOT credit gold - Foundation returns the value, Economy (Chat 3) credits it via its source-aware path. The pet is just the collector.
3. Bond affects ONLY movement (speed/acceleration toward the reserved target) - never amount, rate, radius, reservation eligibility, or value. Confirm against the existing bond-is-movement-only code.
4. The future "pet ignores drops within the hero's radius" rule is expressible via ExclusionSpheres in the query filter - you don't need to implement that rule now, but confirm your query path can pass an ExclusionSphere so it's a later behavior change, not a rework.

Confirm before implementing:
a) Your pet follow/targeting (built disabled behind the seam) snaps onto the live reservation API signatures.
b) The collect-to-credit boundary: the pet collects via Foundation's API and gold lands through Economy's path, not the pet's.
c) Reservation release on unreachable/cancel works so the pet doesn't permanently lock a drop it can't reach.

New content needs a FULL cook/stage, not -SkipCook. Proof: staged run showing the active pet reserving, walking to, and collecting Mob Loot, gold landing via Economy's path, bond affecting only movement speed, and reservation preventing double-collection with the player. No git operations. Critique first; implement after I confirm.

Relevant repo rules:
- Follow AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator, Claude is Validator from .t66/operator-state.json.
- Mini/minigame scope is excluded.
- Claude must be read-only for validator work.
- Do not mutate gameplay/source/data/config/save files for this critique.
- No git operations.
