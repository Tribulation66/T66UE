# Pending Issues - Enemies

## Missing Production Archetype Classes

- Severity tag: [Major]
- What's wrong: The production roster includes `Exploder`, `Strafer`, `Stutterer`, `Turret`, `Burrower`, and `Necromancer` archetypes, but there are no `AT66ExploderEnemy`, `AT66StraferEnemy`, `AT66StuttererEnemy`, `AT66TurretEnemy`, `AT66BurrowerEnemy`, or `AT66NecromancerEnemy` classes under `Source/T66/Gameplay/Enemies`. Those mobs are mapped through `FamilyID` to existing fallback classes so they can spawn, but they exhibit fallback behavior only. Necromancers do not summon, Exploders do not explode, Burrowers do not burrow, Turrets are not stationary, Strafers do not strafe, and Stutterers do not stutter.
- Why it's out of scope now: This pass is a roster/data/asset migration, not a six-archetype gameplay implementation pass.
- What fixing it would entail: Implement six enemy subclasses or shared behavior components, add class resolution by `Archetype`, tune per-archetype parameters, and add runtime tests for each behavior.
