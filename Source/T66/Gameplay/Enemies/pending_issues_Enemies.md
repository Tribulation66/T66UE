# Pending Issues - Enemies

## Missing Production Archetype Classes [RESOLVED 2026-05-29]

- Severity tag: [Resolved]
- Original concern: The roster was thought to require non-ranged special archetypes `Exploder`, `Stutterer`, and `Burrower` backed by dedicated `AT66ExploderEnemy`/`AT66StuttererEnemy`/`AT66BurrowerEnemy` classes that did not exist, leaving those mobs in fallback behavior only.
- Resolution: The Enemy Roster Restructure dropped those speculative special archetypes. The live `Content/Data/Enemies.csv` now uses only the `Melee`, `Rush`, `Flying`, and `Ranged` archetypes, each backed by an existing class, so there is no roster entry that resolves to a missing archetype class. No fallback-behavior gap remains.
- Note: If `Exploder`/`Stutterer`/`Burrower` (or reintroduced ranged subsections such as `Turret`/`Strafer`/`Necromancer`) return later, treat that as a new, deliberate gameplay-implementation pass rather than reviving these stale assumptions.
