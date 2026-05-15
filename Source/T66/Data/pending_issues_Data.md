# Pending Issues - Data

## Enemy Family, Role, And Archetype Redundancy

- Severity tag: [Major]
- What's wrong: `FT66EnemyData` now carries `FamilyID`, `RoleID`, and `Archetype` in `Source/T66/Data/T66DataTypes.h`. The production roster needs `Archetype` for the 10 mechanical labels, while existing spawn/class resolution still depends on `FamilyID`; `RoleID` is currently kept as a compatibility duplicate of `FamilyID`.
- Why it's out of scope now: This pass must preserve existing consumers while importing the 50-mob roster and mapping unsupported archetypes to fallback families.
- What fixing it would entail: Build the missing archetype classes and migrate spawn/class selection to the new `Archetype` field, then deprecate or remove `FamilyID` and `RoleID` after all consumers have moved.
