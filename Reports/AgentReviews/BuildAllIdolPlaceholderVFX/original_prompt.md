# Original User Request

The user approved the primitive-shape concept plan and now wants implementation.

Relevant current request:

> Ok go for it and build these all out, for the no weapon go ahead and just make it a white ball hitting one enemy like a punch

## Task Contract

Working task: Implement the approved temporary basic-shape placeholder VFX for all 20 idols plus No Weapon as a white single-target punch ball.
Operator: Codex
Validator: Claude
Scope: Use the existing temporary/basic-shape placeholder infrastructure where possible. All 20 current idols: Fire/Ice/Electricity/Nature/Wind x DOT/AOE/Pierce/Bounce. No Weapon should be a white ball that hits one enemy like a punch. This is not final Niagara polish or imagegen.
Stop condition: Code/data changes are made, compile/build/runtime verification is attempted, and skipped visual proof is reported.

## Repo Process Constraints

- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Codex is Operator; Claude is Validator.
- This is combat VFX-adjacent and process-governed. The user has explicitly approved a temporary primitive/basic-shape placeholder lane. Do not present this as final Niagara production VFX.
- Prefer existing temporary projectile/placeholder infrastructure over inventing a parallel path.
- No destructive git operations.

## Approved Concept Direction

- Fire AOE: explosion. Fire DOT: body burn. Fire Pierce: flame lance. Fire Bounce: snapping embers.
- Ice AOE: frost nova, no spikes. Ice DOT: frozen body. Ice Pierce: icicle spear. Ice Bounce: shard scatter.
- Electricity AOE: lightning strikes from above in radius. Electricity DOT: shocked body. Electricity Pierce: forward lightning bolt. Electricity Bounce: chain spark.
- Nature AOE: branch/root pokes in radius. Nature DOT: spore poison. Nature Pierce: root forward on ground. Nature Bounce: seeds.
- Wind AOE: tornado moving in a horizontal circle, damage radius smaller than movement circle. Wind DOT: dust devil on enemy. Wind Pierce: skinny tornado forward. Wind Bounce: little tornadoes bouncing.
- Rarity order: Black -> Red -> Yellow -> White, increasing intensity.
- Element colors: Fire red/orange, Ice ice-blue, Electricity purple, Nature green, Wind grey, No Weapon white.

## Requested Validator Output

Give an independent repo-grounded implementation review: likely owning files, risks, and verification that Codex should perform. Return `Result: OK` if implementable without user decision, or `Result: NEEDS_USER` only if a user-only decision blocks implementation.
