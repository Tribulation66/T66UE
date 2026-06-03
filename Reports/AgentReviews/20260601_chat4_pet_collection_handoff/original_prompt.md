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
