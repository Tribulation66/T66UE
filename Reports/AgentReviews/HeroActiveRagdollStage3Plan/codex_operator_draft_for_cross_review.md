# Codex Operator Draft: Hero Active Ragdoll Stage 3 Plan

## Task

Plan the next implementation stage only: Hero Active Ragdoll MVP for Hero 1 Chad, now Stage 3 after the PhysicsFirst rigging/animation stage. Wait for user green light before implementation.

## Current Repo-Grounded Basis

- `Gameplay/Physics/README.md` makes Hero 1 Chad the first active-ragdoll target and identifies Stage 3 as unimplemented.
- `Gameplay/Physics/MASTER_PHYSICS.md` defines the target as capsule-backed always-on active ragdoll with local PAC pose drive, kinematic hip/pelvis anchor, impact loosen, impulse/rebound, and recovery ramp.
- `Gameplay/Physics/HeroPhysicsModel.md` already names the intended runtime component shape: `UT66HeroPhysicsComponent`, `FT66HeroPhysicsProfile`, `FT66HeroPhysicsRuntimeState`, and states `Balanced`, `Staggered`, `KnockedDown`, `Recovering`.
- `Gameplay/Physics/PhysicsAssetPipeline.md` says the Stage 2 PhysicsAsset is a seed and requires runtime proof before it is accepted for always-on active-ragdoll play.
- `Gameplay/Physics/PHYSICS_AGENTS.md` says old pure-Chaos/PAC-off work is historical and `UT66KnockbackComponent` must remain a legacy fallback until the new path proves itself.

## Proposed Stage 3 Goal

Create the first playable Hero 1 Chad active-ragdoll path in the real game, using TestRoom's wipeout arm as the first proof obstacle. The MVP is not a standalone prototype: it should be a runtime physics layer that can become the standard hero physics model.

Stage 3 is complete only if Hero 1 Chad:

1. Runs with an always-simulated skeletal body under capsule-authoritative locomotion.
2. Uses Physical Animation Component pose drive in local simulation, not a hit-only detached ragdoll.
3. Has a kinematic hip/pelvis anchor constrained to the simulated pelvis so the body wobbles, rebounds, and stays recoverable.
4. Reacts to the existing TestRoom wipeout arm through the new active-ragdoll reaction path, with legacy knockback still available as fallback.
5. Recovers from a stagger/knockdown through a stiffness/anchor ramp rather than snap-back.
6. Produces Unreal-owned video/log proof in a staged standalone build.

## Implementation Plan

### 0. Preflight And Asset Verification

- Confirm the Stage 2 Hero 1 Chad PhysicsFirst skeletal mesh, animation clips, and seed PhysicsAsset are the current runtime targets.
- Confirm the Stage 2 PhysicsFirst rig, clips, and seed PhysicsAsset are stable enough to build on before coding begins. Do not build Stage 3 on any mid-change or partially generated foundation.
- Inspect body names, pelvis/root mapping, collision bodies, constraints, mass distribution, and whether the Stage 2 single-influence weighting makes active simulation unreadable.
- Treat single-influence skinning as an explicit Stage 3 decision point: if rigid single-bone weights make continuous simulation visibly wrong, reskining or targeted weight repair becomes part of the MVP instead of deferred polish.
- Treat the current PhysicsAsset as a seed. Tune it if necessary, but do not reroute back to ToonStyle, AccuRig, or the older FriendSlop raw rigging approach.

### 1. Add The Runtime Component Boundary

- Add a dedicated source boundary for the new owned runtime physics layer, preferably `Source/T66/Gameplay/Physics/`.
- Implement `UT66HeroPhysicsComponent` with a small `FT66HeroPhysicsProfile` and `ET66HeroPhysicsRuntimeState`.
- Attach it to `AT66HeroBase`, expose a getter, and keep `UT66KnockbackComponent` intact as fallback.
- Keep the first profile in C++ defaults or narrow config only; full DataTable-driven reaction profiles belong to the later reaction-profile stage.

### 2. Bring Up Always-On Simulated Hero Body

- On Hero 1 Chad initialization, bind the component to the active skeletal mesh and PhysicsAsset.
- Simulate the required bodies continuously, with physics blend weight at 1 for the active body set.
- Configure collision, damping, sleep behavior, and substep-friendly settings so the body remains active and readable during movement.
- Bind `UPhysicalAnimationComponent` to the skeletal mesh and apply local pose-drive settings below the pelvis/root body set.

### 3. Add Hip/Pelvis Anchor Ownership

- Create a hidden kinematic primitive anchor that rides with the capsule/root at the intended hip location.
- Add a physics constraint between the anchor and the skeletal mesh pelvis body.
- Tune linear/angular limits and drive strength so the capsule remains authoritative while the mesh can lag, wobble, lean, rebound, and recover.
- Add divergence guards and logging for pelvis-vs-capsule distance so failures are visible instead of silent.

### 4. Build The First Runtime State Machine

- Implement `Balanced`, `Staggered`, `KnockedDown`, and `Recovering` in the new component.
- Balanced: normal locomotion with always-on wobble.
- Staggered: short loosened pose drive/anchor after moderate impacts, partial control retained.
- KnockedDown: stronger loosened drive plus larger impulse response, temporary control/combat suppression.
- Recovering: ramp pose drive and anchor stiffness back to balanced, ideally using the Stage 2 get-up/recover animation poses as the authored recovery target.

### 5. Bridge TestRoom Wipeout Arm To The New Path

- In the verified TestRoom wipeout-arm hit path, replace the current first-choice legacy call to `UT66KnockbackComponent::ApplyKnockbackLaunch(...)` with `UT66HeroPhysicsComponent::ApplyPhysicsReaction(...)` for Hero 1 Chad when the active-ragdoll component is enabled and initialized. The current hook is in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` around the wipeout-arm impact branch that builds `LaunchVelocity` and applies knockback.
- Fall back to `UT66KnockbackComponent::ApplyKnockbackLaunch(...)` if the new component is disabled, not ready, or the actor is not the Stage 3 target.
- Use a narrow CVar or profile flag for rollback during tuning.

### 6. Movement, Input, Camera, And Leap Integration

- Keep the capsule as the movement authority; the active ragdoll affects readability and reaction, not navigation ownership in this MVP.
- Feed state into existing movement/input suppression only where required for `KnockedDown` and recovery.
- Keep walk/jump/leap animation playback driven by capsule state, while PAC and the hip anchor supply the physical wobble and impact response.
- Check camera follow against capsule/mesh divergence so the view does not snap to a thrown mesh.

### 7. Proof Harness And Verification

- Add or extend an Unreal-owned proof mode for active-ragdoll capture instead of relying on desktop screenshots.
- Capture at least three proof beats: balanced walk/idle wobble before impact, wipeout-arm contact with loosen/impulse/rebound, and recovery back to controllable state.
- Log state, drive strength, anchor strength, pelvis/capsule distance, body simulation status, and recovery timers.
- Run focused compile, staged standalone refresh, shortcut target verification, and standalone smoke/video proof.

## Acceptance Gates

The MVP should be reported as FULL only if all required mechanisms are present with multi-frame evidence:

- Always-on body simulation before impact.
- Local PAC pose drive.
- Hip/pelvis anchor constraint.
- Impact loosen plus impulse/rebound.
- Recovery/get-up stiffness ramp.
- Capsule-authoritative locomotion preserved.
- Unreal-owned staged standalone video/log proof.

Anything missing should be reported as PARTIAL, not complete.

## Out Of Scope For Stage 3

- Migrating all heroes, mobs, bosses, or all obstacles.
- Building the full reaction profile/data-table system.
- Removing `UT66KnockbackComponent`.
- Renaming transitional `FriendSlopRaw` or `PhysicsFirst` paths unless a path blocks runtime binding.
- Final skinning polish for all animations unless Hero 1 deformation makes the proof unreadable.
- Broad Leap/Dash/Roll compatibility cleanup beyond what is necessary for the active-ragdoll state checks.

## Main Risks

- The Stage 2 single-influence skinning may be acceptable for proof but may look too rigid under continuous simulation.
- The seed PhysicsAsset may need meaningful constraint/body tuning before it can support the anchor without collapse.
- Capsule/mesh divergence can break camera readability or floor contact if not guarded.
- Reusing the old knockback component too aggressively would recreate the old hit-only ragdoll path instead of the new always-on active-ragdoll method.

## Green-Light Boundary

No implementation should start until the user approves this Stage 3 plan.
