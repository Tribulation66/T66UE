Result: OK

## Independent Answer

**The architecture call is NO — heroes should not use the Physical Animation Component for ragdoll.**

The user's own criterion settles it: PAC exists to drive simulated physics bodies *back toward an animated/target pose* (it's a corrective, "pull-toward-intended-pose" mechanism). That is precisely the "drag you to a certain location / come back to a specific place" behavior the user wants to eliminate. The user explicitly extended the chaos principle to "everything... even the bosses." So the answer is a clean NO, and the work is to disable hero PAC at the source, not just in the TestRoom profile.

Reality on the ground actually makes this almost a no-op functionally: the default profile ships with `bDetachMeshDuringRagdoll = true` (T66KnockbackComponent.h:87), and the runtime path at T66KnockbackComponent.cpp:147-159 *force-disables* `bEnablePhysicalAnimation` and sets `PhysicalAnimationDriveMode = Disabled` whenever the mesh is detached. So on the active hero ragdoll path, PAC already never runs — the profile defaults (`bEnablePhysicalAnimation = true`, `PhysicalAnimationDriveMode = CoreChain`) are misleading dead intent.

Recommended scoped implementation for Codex:
1. Flip the `FT66KnockbackProfile` defaults to make the decision explicit and self-documenting: `bEnablePhysicalAnimation = false` (line 144) and `PhysicalAnimationDriveMode = ET66KnockbackPhysicalAnimationDriveMode::Disabled` (line 147). This makes "heroes don't use PAC" the authored default everywhere, not a runtime override that fights the profile.
2. Leave the runtime detached-path force-disable in place (defense-in-depth; harmless once defaults agree).
3. Focused compile verification of the T66 module after the edit.

Do **not** rip out the PAC infrastructure (component pointer, `GetOrCreatePhysicalAnimationComponent`, `ApplyPhysicalAnimationDrive`, the enum, the profile fields). Disabling via defaults is the minimal, reversible change and keeps the door open if a non-detached, PAC-driven use ever appears for another actor class.

## Evidence Checked
- `T66KnockbackComponent.h:47-166` — `FT66KnockbackProfile`, PAC fields default enabled (`bEnablePhysicalAnimation=true`, `DriveMode=CoreChain`), `bDetachMeshDuringRagdoll=true`.
- `T66KnockbackComponent.cpp:147-159` — runtime force-disables PAC on the detached path.
- `T66KnockbackComponent.cpp:296-300` — detach occurs whenever `bDetachMeshDuringRagdoll` (the default).
- `cpp:1199-1272` / `1141-1199` — PAC creation/drive gated on `bEnablePhysicalAnimation` and non-`Disabled` drive mode.
- Original prompt + task contract.

## Questions Or Blockers
None that require the user. The user already supplied the deciding principle ("let the chaos do its thing," no return-to-place, applies to everything including bosses), so this is an internal architecture call Codex can execute.

## Caveats
- PAC infrastructure (component, drive functions, enum, profile fields) remains in the codebase after this change — be honest about that; this is a disable, not a removal.
- The change is partly cosmetic on the active path (detach already kills PAC at runtime), but it removes the contradiction between authored defaults and actual behavior, which matters if anyone later sets `bDetachMeshDuringRagdoll=false`.
- This decision is scoped to heroes' ragdoll/knockback. The user's "even bosses" comment about not returning to a fixed place is a related but separate behavior (anchoring/follow targeting), not strictly PAC — don't silently fold boss-positioning changes into this PAC task without confirming scope.
- I did not run a compile; verification is Codex's stop-condition step.
