# T66 Gameplay Physics

This folder is the start point for gameplay physics work: hero ragdoll reactions, PhysicsAsset readiness, obstacle hit response, and the longer-term bouncy player feel.

## Start Here

Read in this order:

1. `CURRENT_STATE.md`
2. `HISTORY.md`
3. `MASTER_PHYSICS.md`
4. `HeroPhysicsModel.md`
5. `PhysicsReactionProfiles.md`
6. `PhysicsAssetPipeline.md`
7. `Archive/README.md`
8. `../../Source/T66/Gameplay/Physics/pending_issues_Physics.md`

## Current Implemented State

As of 2026-06-07, the game uses Hero 1 Chad hit-triggered full ragdoll, not always-on active ragdoll.

The live implementation is `UT66HeroPhysicsComponent`:

```text
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp
```

Normal play is owned by the capsule and `CharacterMovementComponent`. The skeletal mesh is attached to the capsule, animated, collisionless, and non-simulating.

On a qualifying obstacle hit, the component detaches the mesh, disables movement, keeps the capsule as a query-only damage hurtbox, simulates all skeletal bodies, applies launch velocity/body impulse, moves the actor XY toward the pelvis while ragdolled, places the capsule under the pelvis for get-up, reattaches the mesh, blends out, and restores walking.

The public names still use "ActiveRagdoll" in a few places for compatibility, but the current behavior is hit-triggered full ragdoll:

```text
t66.HeroPhysics.EnableActiveRagdoll
t66.TestRoom.WipeoutArmUseHeroActiveRagdoll
```

## Not Currently Implemented

These were explored or planned, but they are not the live runtime behavior:

- always-on simulated wobble during normal walking
- `Balanced` / `Staggered` / `KnockedDown` / `Recovering` active-ragdoll states
- PAC-driven child-body muscle during normal play
- capsule-following hip-anchor constraint
- per-tick mesh-to-capsule active pose frame with pelvis held by hip anchor

Do not document or tune those as current unless the code is rebuilt to match them.

## Live Source Map

- Hero ragdoll component:
  - `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
  - `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- Legacy knockback fallback:
  - `Source/T66/Gameplay/T66KnockbackComponent.h`
  - `Source/T66/Gameplay/T66KnockbackComponent.cpp`
- TestRoom wipeout arm:
  - `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
- Current Hero 1 visual row:
  - `Content/Data/CharacterVisuals.csv`
- Current Hero 1 source rigging/output path:
  - `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/PhysicsFirstHero/`

## Current Proof Route

Use the project-owned capture path for runtime proof:

```text
Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof
```

Docs-only changes do not require compile, staged standalone, or capture. Runtime physics changes do.

## Boundary

This folder owns the gameplay physics contract. It does not own raw Blender authoring, ToonStyle import, trap progression, or movement input binding, but those systems must follow the physics contract when they participate in obstacle reactions or Hero 1 ragdoll behavior.
