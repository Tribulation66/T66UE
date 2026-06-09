# Physics Reaction Profiles

## Purpose

Physics reaction profiles define how actors respond to physical hits, obstacles, launches, and environment contact.

They are broader than trap damage. Traps may use reaction profiles, but reaction profiles belong to Physics.

## Current Profile Surface

The current implemented Hero 1 profile is `FT66HeroPhysicsProfile` in:

```text
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h
```

Current fields:

- `bEnabled`
- `bHero1ChadOnly`
- `SimulationRootBodyName`
- `PelvisBodyName`
- `ReactionCooldownSeconds`
- `RagdollLaunchSpeedMax`
- `RagdollLaunchUpSpeed`
- `RagdollSettleSpeed`
- `RagdollSettleHoldSeconds`
- `RagdollMaxSeconds`
- `RagdollBlendOutSeconds`
- `FloorTraceUpDistance`
- `FloorTraceDownDistance`

Obstacle callers may request a higher vertical launch through `ApplyPhysicsReaction`. The runtime uses at least `RagdollLaunchUpSpeed` and clamps requested upward speed to a bounded multiple of that profile value, so launch pads and bumpers can read differently without each trap owning its own ragdoll state.

Current runtime states:

- `Normal`: ordinary capsule/CharacterMovement play.
- `Ragdoll`: full skeletal body simulation after a qualifying hit.
- `GettingUp`: floor-aware capsule placement, get-up animation, and restoration to walking.

## TestRoom Wipeout Arm

The TestRoom arm currently builds a requested velocity from:

```text
t66.TestRoom.WipeoutArmLaunchXY = 10500
t66.TestRoom.WipeoutArmLaunchZ = 750
```

Default route:

1. build requested launch vector
2. try `UT66HeroPhysicsComponent::ApplyPhysicsReaction`
3. fall back to `UT66KnockbackComponent` only if hero physics does not apply

Important: the requested wipeout-arm vector is not the final body velocity by itself. `UT66HeroPhysicsComponent` clamps horizontal speed with `RagdollLaunchSpeedMax` and uses `RagdollLaunchUpSpeed` for the current hit-triggered ragdoll behavior.

## Legacy Fallback

`UT66KnockbackComponent` remains as legacy fallback support. The old knockback profile and PAC toggles in `T66GameMode_TestRoom.cpp` are not the final Hero 1 physics profile.

Current default PAC toggle for the legacy wipeout-arm path:

```text
t66.TestRoom.WipeoutArmEnablePhysicalAnimation = 0
```

`UT66HeroPhysicsComponent` currently does not use PAC.

## Future Data Profile Intent

Future data should express:

- reaction profile ID
- target actor class or budget tier
- contact source type
- impulse direction source
- radial/tangent/normal blend
- horizontal force
- vertical force
- launch clamp
- velocity-change mode vs mass-scaled impulse mode
- input suppression rules
- recovery profile
- repeat-hit cooldown
- camera/follow behavior
- debug/proof labels

## Future Obstacle Relationship

Production traps currently own damage, activation, and progression. Physics reactions should eventually let obstacles and traps request a physical response without each obstacle inventing its own launch math.

Examples:

- rotating arm: radial/tangential shove plus ragdoll window
- bumper: rebound impulse plus short control loss
- flipper/launcher: vertical launch plus airborne control profile
- fan: sustained directional force
- conveyor: sustained lateral velocity/force bias
- seesaw: moving surface and balance challenge

## Historical State Names

These names appeared in always-on active-ragdoll planning, but are not current runtime states:

- `Balanced`
- `Staggered`
- `KnockedDown`
- `Recovering`

Do not use them in proof criteria unless a new source implementation adds them.

## Anti-Lookalike Gate

Wrong result:

- one bigger launch vector that looks dramatic for one frame

Required discriminator:

- multi-frame evidence that the body simulates, travels, collides/rebounds/slides, and recovers through get-up without snapping or losing the player.
