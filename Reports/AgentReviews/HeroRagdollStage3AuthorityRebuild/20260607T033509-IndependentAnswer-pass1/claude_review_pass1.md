I have enough grounding to produce the independent answer. This is an architecture/plan answer the models can resolve internally; no user-only decision is blocking.

Result: OK

## Independent Answer

The current `T66HeroPhysicsComponent` has the authority inverted from the agreed model, which directly explains both symptoms. Two competing authority paths fight each other:

1. **Pelvis is kinematic, not simulated.** `ConfigureMeshPhysics` sets `ComponentTransformIsKinematic` + `SkipAllBones` and calls `SetAllBodiesBelowSimulatePhysics(SimulationRootBodyName, true, /*bIncludeSelf=*/false)`. The `false` leaves the pelvis/root **kinematic** while only children simulate. But the agreed architecture requires an **always-on simulated pelvis** with a **separate kinematic hip anchor** following the capsule.
2. **Wipeout barely moves the hero** because `ApplyPhysicsReaction` does `AddImpulse` on `PelvisBodyName` (`T66HeroPhysicsComponent.cpp:200`) — a kinematic body ignores impulses. The reaction also overwrites `Movement->Velocity` on the capsule (`:209`), so the visible motion is whatever the capsule does, not ragdoll physics.
3. **Normal-movement stretch/spazz** comes from the kinematic root teleporting with the capsule each frame while PAC local drive pulls simulated children toward animation targets, plus the `ResyncPelvisBodyToActor` / `FollowActorToPelvisBody` teleport loops (`:696`, `:776`) that snap bodies and even the actor every tick. Those teleport loops are precisely the "actor-to-pelvis follow loops should not be the normal authority path" the prompt rules out.

### Rebuild plan (repo-grounded)
1. **Invert body ownership.** Make the pelvis a simulated body at blend weight 1 (`SetAllBodiesBelowSimulatePhysics(Pelvis, true, /*bIncludeSelf=*/true)` or simulate pelvis explicitly). Drop `ComponentTransformIsKinematic`/`SkipAllBones` in favor of a mesh that follows simulation (`SimulationUpatesComponentTransform` style) since the pelvis is now the simulated source.
2. **Add a real kinematic hip anchor.** Keep the existing `HipAnchorComponent` (sphere attached to capsule) but make it the only thing that tracks the capsule. The capsule stays gameplay/nav authority; the anchor is its kinematic proxy.
3. **Constraint anchor→simulated pelvis.** `ConfigureHipAnchorConstraint` already constrains `(Mesh, PelvisBodyName) ↔ HipAnchor`; this becomes meaningful only once the pelvis actually simulates. Tune linear/angular drive (the existing `AnchorLinear*`/`AnchorAngular*` profile fields) so the capsule "carries" the pelvis without hard-locking it.
4. **PAC as muscle only.** Keep `bIsLocalSimulation = true` child pose drive (`:445-463`) but ensure it never drives the pelvis as the gameplay authority — pose drive shapes the body, the anchor constraint positions it.
5. **Delete the teleport/follow loops** as the normal path. Remove `FollowActorToPelvisBody` from the per-tick state machine and demote `ResyncPelvisBodyToActor` to a bounded safety-only recovery (the `MaxPelvisCapsuleDistance` guard), not a steady-state mechanism.
6. **Reaction routes to simulated bodies.** `ApplyPhysicsReaction` should `AddImpulse` to the now-simulated pelvis and bodies below at the hit location, temporarily lower pose/anchor multipliers (the existing stagger/knockdown multipliers), and apply only a **bounded capsule shove** for gameplay displacement — not overwrite capsule velocity outright.
7. **Keep `UT66KnockbackComponent` as legacy fallback** (Hard Rule) until the rebuilt path passes proof.

### Top risks / traps for Codex
- **Half-inversion**: simulating the pelvis but leaving `ComponentTransformIsKinematic` or the teleport loops in place will reproduce the same fight. The two authority models must not coexist.
- **Impulse on kinematic body** silently no-ops — verify the pelvis is actually simulating before trusting reaction logs.
- **Constraint frame/limits**: a too-stiff anchor drive re-creates the rigid "pulled mesh" feel; too loose and the hero floats away from the capsule. This is the main tuning axis.
- **Mesh transform update mode** must match the new owner (simulation-driven), or you get double-application of capsule + simulation.
- **Hardcoded feel values**: keep everything in `FT66HeroPhysicsProfile`/data, per Hard Rule "prefer data-authored physics profiles."
- **Hero-1-Chad gating** (`ShouldEnableForHero` requires mesh name contains `Hero_1_Chad` + `PhysicsFirst`) — confirm the TestRoom hero actually matches or the rebuild silently never initializes.

### Doc updates required in the same change
- `Gameplay/Physics/HeroPhysicsModel.md` — rewrite the "Stage 3 Runtime Component Plan" to describe simulated-pelvis + kinematic-anchor authority and explicitly retire the kinematic-pelvis/actor-follow approach.
- `Gameplay/Physics/MASTER_PHYSICS.md` and `PhysicsReactionProfiles.md` — update reaction model (impulse to simulated bodies, pose/anchor loosening, bounded capsule shove) and remove stale teleport/follow-loop description.
- `PhysicsAssetPipeline.md` — confirm pelvis-simulated/CCD/mass-dominant gates still hold under always-on simulation.
- Check `Gameplay/Movement/MASTER_MOVEMENT.md` and `Gameplay/Traps/MASTER_TRAPS.md` for any references to the old "capsule velocity overwrite" reaction and align them.
- Record any out-of-scope findings in `Source/T66/Gameplay/Physics/pending_issues_Physics.md`.

### Focused verification before calling the pass done
- Focused compile of the T66 target.
- TestRoom direct-entry run with `t66.HeroPhysics.DebugLog 1`: confirm `Init OK` with **pelvis simulated** (not kinematic), and that `PelvisCapsuleDist` stays bounded during normal movement (no growing divergence / resync spam).
- Normal locomotion capture (Unreal-owned, multi-frame) showing **no stretch/spazz**.
- Wipeout/obstacle hit capture showing the body actually reacting (impulse visible on simulated bodies), then recovering through Staggered→Recovering→Balanced.
- Staged standalone refresh + shortcut verification since playable behavior changes (Hard Rule + AGENTS staged-build process).
- Report passed / failed / still-needs-tuning per the stop condition, with log + frame evidence, not desktop screenshots.

## Evidence Checked
- `AGENTS.md` (process router, staged-build and Operator/Validator rules).
- `Gameplay/Physics/PHYSICS_AGENTS.md`, `HeroPhysicsModel.md` (owner docs, Hard Rules, Stage 3 plan).
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h` / `.cpp` (full current implementation — authority config, reaction path, teleport/follow loops).
- Invocation sites: `T66HeroBase.cpp`, `T66GameMode_TestRoom.cpp`, `T66PlayerController_Overlays.cpp` (not deep-read; confirms wiring points exist).

## Questions Or Blockers
None that require the user. The authority model is already specified in the prompt and matches the owner docs; Codex can implement and verify internally. (Not deep-read, so Codex should confirm during implementation: the exact reaction call sites in `T66HeroBase.cpp` / TestRoom, and that the live TestRoom hero mesh name satisfies the `Hero_1_Chad`+`PhysicsFirst` gate.)

## Caveats
- Read-only pass; I did not compile or run. Symptom diagnosis is inferred from code reading, but the kinematic-pelvis-vs-simulated-children conflict and impulse-on-kinematic-body no-op are strongly supported by the source.
- I did not read every prior review report under `Reports/AgentReviews/HeroRagdoll*`; if the "agreed authority model" recorded there differs in any detail from the prompt's architecture block, the prompt and owner docs should win, but Codex should reconcile.
- Tuning values (anchor stiffness, impulse scale) are feel-dependent and will need iteration with capture proof; the plan fixes the architecture, not the final numbers.
