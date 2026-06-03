# Pending Issues - World

## Backrooms Textures Are Intentionally Preserved

- Severity tag: [Minor]
- What's wrong: The old-prop audit can report `/Game/World/Backrooms/Textures/T_Backrooms_Door`, `T_Backrooms_Floor`, and `T_Backrooms_Wall` as zero-referencer assets even though the user has explicitly identified the Backrooms door, floor, and wall textures as used content to keep.
- Why it's out of scope now: The hygiene cleanup deleted the approved legacy cliff material chain in Gate B, but the Backrooms textures are intentionally preserved and must not be folded into automated zero-reference cleanup.
- What fixing it would entail: Do not delete these textures without a new exact user approval and a fresh runtime proof that current Backrooms presentation no longer depends on them.
