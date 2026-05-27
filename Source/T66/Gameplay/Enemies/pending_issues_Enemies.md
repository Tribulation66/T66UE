# Pending Issues - Enemies

## Missing Production Archetype Classes

- Severity tag: [Major]
- What's wrong: The production roster still includes non-ranged special archetypes `Exploder`, `Stutterer`, and `Burrower`, but there are no `AT66ExploderEnemy`, `AT66StuttererEnemy`, or `AT66BurrowerEnemy` classes under `Source/T66/Gameplay/Enemies`. Those mobs are mapped through `FamilyID` to existing fallback classes so they can spawn, but they exhibit fallback behavior only. Exploders do not explode, Burrowers do not burrow, and Stutterers do not stutter. Ranged subsections were intentionally collapsed into the single `Ranged` archetype/class path for now; reintroducing ranged subsections later should be a deliberate design pass.
- Why it's out of scope now: This pass is a roster/data/asset migration, not a three-archetype gameplay implementation pass.
- What fixing it would entail: Implement three enemy subclasses or shared behavior components, add class resolution by `Archetype`, tune per-archetype parameters, and add runtime tests for each behavior. If ranged subsections return later, add that as a separate ranged-design migration instead of reviving stale `Turret`, `Strafer`, or `Necromancer` assumptions silently.
