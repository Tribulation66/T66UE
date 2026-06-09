# Current Physics State

Last updated: 2026-06-09.

## One-Line State

Hero 1 Chad currently uses a hit-triggered full skeletal ragdoll on qualifying physics hits, with capsule/CharacterMovement authority during normal play and a get-up return to walking. Enemy damage below the configured percent threshold uses non-disabling knockback instead of ragdoll.

## Current Runtime Owner

```text
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp
```

The component name and CVars still use "ActiveRagdoll" for compatibility, but the live model is hit-triggered full ragdoll:

```text
t66.HeroPhysics.EnableActiveRagdoll = 1
t66.TestRoom.WipeoutArmUseHeroActiveRagdoll = 1
```

## Normal State

Runtime state:

```text
ET66HeroPhysicsRuntimeState::Normal
```

Normal play behavior:

- capsule collision profile is `Pawn`
- `CharacterMovementComponent` owns locomotion
- skeletal mesh is attached to the capsule
- mesh physics simulation is off
- mesh collision is disabled
- component tick is disabled

`InitializeForHero` resolves the PhysicsAsset bodies and configures this normal state, but it does not keep bodies simulating during normal movement.

## Hit Reaction State

Enemy damage reaction gate:

- successful enemy damage always increases the hero damage percent
- enemy damage at or below `t66.HealthPercent.EnemyDisableStartPercent` uses `LaunchCharacter` knockback only
- enemy damage above that threshold calls `UT66HeroPhysicsComponent::ApplyPhysicsReaction`
- enemy throw distance and disable duration scale from `t66.HealthPercent.EnemyDisableStartPercent` to `t66.HealthPercent.EnemyDisableFullPercent`
- non-enemy physics reactions keep their existing `ApplyPhysicsReaction` path and duration curve

Runtime state:

```text
ET66HeroPhysicsRuntimeState::Ragdoll
```

On `ApplyPhysicsReaction`, the component:

- ignores the request if already ragdolled or still in cooldown
- clamps horizontal launch through `FT66HeroPhysicsProfile::RagdollLaunchSpeedMax`
- uses `FT66HeroPhysicsProfile::RagdollLaunchUpSpeed` for upward launch
- suppresses movement/look input and auto-attack
- stops and disables `CharacterMovementComponent`
- changes capsule collision to query-only so it remains a damageable hurtbox without physically blocking movement while downed
- detaches the mesh with world transform preserved
- enables mesh collision against world/static/dynamic/physics bodies
- sets `PhysicsTransformUpdateMode` to `SimulationUpatesComponentTransform`
- simulates all skeletal bodies
- sets all body physics blend weight to 1
- sets all body linear velocity to the launch velocity
- applies a mass-scaled impulse to the resolved pelvis body at the hit location
- enables component tick

During ragdoll, `UpdateRagdoll` moves the actor XY to the pelvis world location so camera/follow does not stay behind at the old capsule XY.

## Get-Up State

Runtime state:

```text
ET66HeroPhysicsRuntimeState::GettingUp
```

The component enters get-up when the pelvis settles below the profile speed threshold for the hold duration or when the max ragdoll time expires.

Get-up behavior:

- trace down from the pelvis to find floor
- place the capsule under the pelvis/floor position
- stop skeletal physics simulation
- restore full capsule collision
- attach the mesh to the capsule
- restore the cached normal relative transform
- choose front/back get-up animation from pelvis orientation
- blend physics weight down during get-up
- restore walking movement mode
- clear suppression and return to `Normal`

## Current Hero 1 Asset State

Hero 1 Chad uses the physics-first FriendSlop skeletal row in:

```text
Content/Data/CharacterVisuals.csv
```

Current row summary:

- skeletal mesh: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst`
- animations: physics-first `Idle`, `Walk`, `Jump`, and `Leap`
- mesh yaw: `-90`
- static demo-skin row remains separate

## TestRoom Obstacle Route

The TestRoom wipeout arm is in:

```text
Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp
```

Default routing:

- `t66.TestRoom.EnableWipeoutArmTrap = 1`
- `t66.TestRoom.WipeoutArmUseHeroActiveRagdoll = 1`
- impact first calls `Hero->GetHeroPhysicsComponent()->ApplyPhysicsReaction(...)`
- legacy knockback runs only when the hero physics component is unavailable or does not apply

The wipeout arm builds a requested launch vector from `WipeoutArmLaunchXY` and `WipeoutArmLaunchZ`. The hero physics component then clamps and applies its own ragdoll launch profile.

## What Is Not Current Runtime

The following are historical or future-target ideas, not current implementation:

- always-on active ragdoll during normal walking
- hip-anchor constraint component
- `UPhysicalAnimationComponent` muscle drive in `UT66HeroPhysicsComponent`
- `Balanced`, `Staggered`, `KnockedDown`, or `Recovering` states
- component-transform kinematic active mesh sync during normal simulation
- normal-runtime pelvis/body resync or `EmergencyPelvisResync`

If any future pass reintroduces those mechanisms, update this file and the live code together.

## Open Work Pointer

Current known non-final work is tracked in:

```text
Source/T66/Gameplay/Physics/pending_issues_Physics.md
```

The short version: the infrastructure is playable enough to test, but the final Fall Guys-like feel still needs tuning and proof.
