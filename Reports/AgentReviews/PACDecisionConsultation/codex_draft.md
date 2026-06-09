Draft answer:

PAC should not be enabled on the current detached ragdoll path. That is not just a temporary toggle; it is the correct architecture for the way we stabilized the wipeout-arm ragdoll. The current path detaches the skeletal mesh during ragdoll, follows the actor/camera to the simulated body center, and hard-clamps floor penetration. PAC tries to drive simulated bodies toward an animation pose, so using it while the mesh is detached risks stale target-space/body-space conflicts and reintroduces the exact kind of body/actor divergence we just fixed.

The recommendation is:

- Current hero/trap launch ragdoll: keep pure Chaos ragdoll, PAC off.
- Recovery: use animation/pose blending after physics is restored, not PAC during the detached flight.
- Future active-ragdoll mode: keep the existing PAC infrastructure, but only enable it in a separate attached-ragdoll profile where the mesh remains attached and pose buffers/target transforms are valid.

So the answer is "not for this path, but possibly later for a different path." PAC remains useful for bosses, elites, partial staggers, get-up blends, or attached active-ragdoll behaviors if pure ragdoll cannot provide the Fall Guys-style controlled flop. It should come back only after pure ragdoll is stable and only behind an explicit profile/config gate.

Acceptance rule I would use:

Enable PAC later only if all of these are true:
1. `bDetachMeshDuringRagdoll` is false for that profile.
2. The mesh has valid pose buffers and a known-good PhysicsAsset.
3. Actor/camera follow remains stable under repeated hits.
4. PAC visibly improves the feel versus pure ragdoll in a capture.
5. It stays limited to low-count actors: hero, bosses, maybe elites.

This preserves the scalable infrastructure: pure ragdoll as the baseline gameplay knockdown, PAC as an optional higher-fidelity attached mode.
