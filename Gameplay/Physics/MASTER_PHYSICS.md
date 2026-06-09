# Master Physics

## Purpose

This document is the policy center for gameplay physics in T66. It owns the current runtime contract, the target feel direction, and proof gates for physics-driven gameplay.

For the exact current implementation, read `CURRENT_STATE.md` first.

## Current Runtime Contract

The current implemented Hero 1 Chad model is hit-triggered full ragdoll for qualifying physics reactions. Enemy damage has an additional percent gate: below the enemy disable threshold it uses non-disabling knockback, and above the threshold it enters the full ragdoll reaction.

Normal play:

- capsule and `CharacterMovementComponent` own input, movement, navigation, camera target, and collision
- skeletal mesh is attached to the capsule
- skeletal mesh is animated, collisionless, non-simulating, and not ticking physics

On qualifying hit:

- `UT66HeroPhysicsComponent::ApplyPhysicsReaction` routes the hit
- movement/look input and auto-attack are suppressed
- movement is stopped and disabled
- capsule collision is reduced to query-only so enemy/trap damage can still target the downed hero without using the capsule as a physical blocker
- mesh detaches and simulates all skeletal bodies
- body velocity and pelvis impulse are applied
- actor XY follows the simulated pelvis while ragdolled
- get-up places the capsule under the settled pelvis/floor position
- mesh simulation stops, mesh reattaches, get-up animation blends, and walking returns

Enemy damage threshold:

- enemy touch/projectile-style deliveries always add damage percent
- at or below `t66.HealthPercent.EnemyDisableStartPercent`, enemy damage applies knockback only and does not suppress control
- above `t66.HealthPercent.EnemyDisableStartPercent`, enemy damage enters `ApplyPhysicsReaction`
- enemy launch scale and disable duration ramp from `t66.HealthPercent.EnemyDisableStartPercent` through `t66.HealthPercent.EnemyDisableFullPercent`
- death remains owned by the run-state damage percent reaching 100

The current states are:

```text
Normal
Ragdoll
GettingUp
```

## Naming Caveat

Several current APIs and CVars still say "ActiveRagdoll":

```text
TryInitializeActiveRagdoll
ShutdownActiveRagdoll
t66.HeroPhysics.EnableActiveRagdoll
t66.TestRoom.WipeoutArmUseHeroActiveRagdoll
```

Those names are compatibility names. In the current source, they gate and route hit-triggered full ragdoll reactions, not always-on active-ragdoll locomotion.

## Target Feel

The design target is still Fall Guys-like physical play: bouncy, readable obstacle reactions, tumble/rebound, and quick recovery. Implementation values must be T66-authored and tuned. Do not copy hidden or proprietary values from external games.

The current hit-triggered system is the working MVP foundation. A future always-on wobble/active-ragdoll locomotion layer may be revisited, but it is not current behavior.

## Historical Baseline

The following were attempted or planned and are now historical unless reimplemented:

- legacy launch/knockback-only obstacle response
- passive raw FriendSlop ragdoll import tests
- PAC/partial-simulation stabilization attempts
- pure-Chaos/PAC-off early hit ragdoll
- always-on PAC/hip-anchor authority model

See `HISTORY.md` for details and evidence paths.

## Current Primary Mechanisms

These mechanisms define the current implemented identity:

- capsule-backed normal locomotion
- full skeletal simulation only during the hit reaction window
- body launch and pelvis impulse on obstacle impact
- actor/camera XY follow while ragdolled
- floor-aware capsule placement for get-up
- front/back get-up animation selection
- quick return to normal movement
- legacy knockback fallback when the hero physics component cannot apply

## Future Mechanisms

These are future or experimental, not current:

- always-on simulated wobble during idle/walk
- local PAC pose drive during normal movement
- hip-anchor constraint from capsule to pelvis
- state model with `Balanced`, `Staggered`, `KnockedDown`, and `Recovering`
- data-authored reaction profile table for all obstacles and mobs
- cheaper physics variants for mobs/companions

Any future implementation must update `CURRENT_STATE.md`, `HISTORY.md`, and the source together.

## Anti-Lookalike Rules

These do not satisfy the physics target by themselves:

- a bigger one-shot `LaunchCharacter`
- a dramatic single-frame knockback with no body simulation
- an animated wobble with no simulated bodies
- a static screenshot of a hit
- current docs claiming PAC/hip-anchor behavior without matching source code

The discriminator is multi-frame proof: the body must react through simulation, travel with readable displacement, collide/rebound/slip in the environment, then recover without snapping or losing the player.

## Ownership Boundaries

- Physics owns the shared body-simulation and reaction contract.
- Movement owns input binding and baseline locomotion controls, but any movement behavior that changes physics state must follow this Physics contract.
- Traps own damage/progression/spawning. Physics owns the body reaction when traps or obstacles physically affect an actor.
- Model Generation owns raw asset/Blender process files. Physics owns runtime readiness criteria for hero rigging, PhysicsAsset, and ragdoll proof.

## Verification Policy

Runtime physics work needs:

- focused compile
- staged standalone verification when playable behavior changes
- Unreal-owned capture/video, not desktop screenshots
- frame/log evidence for temporal mechanisms
- explicit FULL/PARTIAL status for any required mechanism
- for Hero 1 ragdoll proof, `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof`
- logs showing TestRoom wipeout-arm routing through hero physics when that path is under test

Docs-only changes do not require compile or staged verification.

## Roadmap

1. Stage 1: physics ownership/routing/docs.
2. Stage 2: Hero 1 Chad physics-first rigging and animation foundation with `Idle`, `Walk`, `Jump`, and `Leap`.
3. Stage 3: current Hero 1 hit-triggered full-ragdoll MVP.
4. Stage 4: tune current hit response, get-up, mass/inertia, and obstacle displacement until video proof reads correctly.
5. Stage 5: data-authored physics reaction profiles.
6. Stage 6: revisit always-on wobble/active-ragdoll locomotion only with a fresh implementation plan and proof gate.
7. Stage 7: obstacle/environment integration and cheaper mob variants.
