# Pending Issues - Data

## Enemy Family, Role, And Archetype Redundancy

- Severity tag: [Major]
- What's wrong: `FT66EnemyData` carries `FamilyID`, `RoleID`, and `Archetype` in `Source/T66/Data/T66DataTypes.h`. After the Enemy Roster Restructure the live `Content/Data/Enemies.csv` uses only the supported `Archetype` values `Melee`, `Rush`, `Flying`, and `Ranged` — the speculative special labels (`Exploder`, `Stutterer`, `Burrower`) were dropped, so no roster row maps to an unsupported archetype. The remaining redundancy is structural: spawn/class resolution still keys off `FamilyID`, and `RoleID` is still kept as a compatibility duplicate of `FamilyID`.
- Why it's out of scope now: This pass is a roster/data import; collapsing the three overlapping fields touches every spawn/class consumer and is deferred to a later data-structure cleanup pass.
- What fixing it would entail: Migrate spawn/class selection to a single source-of-truth field (likely `Archetype`), then deprecate or remove `FamilyID` and `RoleID` after all consumers have moved. New special archetypes, if reintroduced, are a separate gameplay-implementation pass.
