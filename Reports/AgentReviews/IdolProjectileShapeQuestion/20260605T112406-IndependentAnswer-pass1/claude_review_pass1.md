I have enough evidence. The traveler mapping is definitive.

Result: OK

## Independent Answer

**No — there are not 20 visually distinct projectile (traveler) shapes.** There are 20 idol *visual profile IDs*, but the flying projectile carrier resolves to only **4 distinct mesh shapes**.

In `T66OutgoingTravelerPoolSubsystem.cpp:58-80`, all 20 profile IDs (5 elements × 4 categories) map to exactly 4 mesh slots, keyed purely by category:
- AOE → slot 0 (Sphere)
- Pierce → slot 1 (Cone)
- DOT → slot 2 (Cylinder)
- Bounce → slot 3 (Cube)

The slot-to-primitive mapping is confirmed at `T66OutgoingTravelerPoolSubsystem.cpp:276-291` (`GetMeshIndexForTemporaryProjectileShape`: Sphere=0, Cone=1, Cylinder=2, Cube=3).

So while the projectile flying toward a target, what distinguishes the 20 idols is **color** (the per-element `Color`/tint uploaded at `:946-949`), not silhouette. The shape silhouette only carries 4 values (one per category). Fire-AOE and Ice-AOE are the same sphere in different colors; the same is true within each category across all 5 elements.

Codex's draft is accurate on the core point. The richer per-element arrangements it mentions (explosion rings, frost discs, tornados) live in `T66CombatVFX.cpp` and are **activation/impact** effects, not the in-flight projectile silhouette — so they don't change the answer to the literal question.

**Bottom line:** 20 color/profile variants, 4 distinct in-flight shapes. If the requirement is 20 visibly unique silhouettes before impact, that is a real gap — the traveler profile table needs distinct mesh indices per ID rather than per category.

## Evidence Checked
- `Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.cpp` — 20-entry profile table → 4 mesh slots (`:58-80`); mesh-index resolver (`:276-311`); per-traveler color upload (`:946-949`).
- `Source/T66/Gameplay/T66CombatVFX.cpp` — exists; 158 primitive/arrangement references (impact/activation layer, not carrier shape).
- `Reports/AgentReviews/IdolProjectileShapeQuestion/codex_draft.md` — cross-checked against live code; consistent.

## Questions Or Blockers
None requiring the user — this is a confirmable read-only finding. The only open product decision (whether 20 distinct silhouettes are *required*) is implied by the original prompt, not something needing escalation here.

## Caveats
- I did not open the `.uasset` Niagara system to confirm exactly which 4 static meshes are bound to slots 0–3; the C++ mapping to Sphere/Cone/Cylinder/Cube is taken from the shape enum resolver, which is authoritative for index assignment but not the final mesh art.
- Codex's draft did not report token usage, which the stop condition requests — Codex should append that before sending.
- "Visually different" is partly subjective; my conclusion is on silhouette/mesh count, which is objective. Color does make all 20 distinguishable, just not by shape.
