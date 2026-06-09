# Physics History

This file records the physics approaches attempted so new agents can tell current implementation from old experiments.

## Current State Summary

As of 2026-06-07, Hero 1 Chad uses a hit-triggered full ragdoll on qualifying obstacle impact.

Normal movement is capsule/CharacterMovement driven. The mesh is animated, collisionless, and non-simulating. On hit, the mesh detaches, simulates all bodies, receives launch velocity and pelvis impulse, then the actor/capsule is recovered under the settled pelvis and control is restored through get-up.

The current source owner is:

```text
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp
```

## Attempt 1: Legacy Knockback Launch

Main owner:

```text
Source/T66/Gameplay/T66KnockbackComponent.h
Source/T66/Gameplay/T66KnockbackComponent.cpp
```

What it did:

- applied launch/knockback behavior as a gameplay displacement
- supported the early TestRoom wipeout-arm experiments
- included fallback settings and old PAC toggles used by the legacy path

Why it was not enough:

- it read as a launch spike, not body-driven obstacle physics
- it did not create the desired tumble, floor rebound, or physical get-up identity
- it could not be the final Fall Guys-like player feel by itself

Current status:

- legacy fallback support only
- do not tune it as the final Hero 1 physics feel unless the user explicitly reverts direction

## Attempt 2: Raw FriendSlop Ragdoll Import And Passive Physics

Evidence folders:

```text
Reports/AgentReviews/FriendSlopUnrealRagdollImport/
Reports/AgentReviews/FriendSlopRagdollReassessment/
```

What it did:

- brought the raw Hero 1 Chad source into a skeletal/PhysicsAsset path
- proved a skeletal body could be simulated in Unreal
- exposed early body layout, mass, floor contact, camera/follow, and flattening failure modes

Why it was not enough:

- passive ragdoll was not playable movement
- the early rig/PhysicsAsset setup was too fragile for repeated obstacle hits
- visual stability and camera/follow behavior were not acceptable

Current status:

- historical evidence and PhysicsAsset lessons only

## Attempt 3: PAC And Partial Active-Ragdoll Experiments

Evidence folders:

```text
Reports/AgentReviews/FriendSlopPACStabilization/
Reports/AgentReviews/HeroPACDisableDecision/
```

What it tried:

- use Physical Animation Component or partial simulation to preserve pose while allowing reaction
- explore PAC settings on the wipeout-arm path

Why it was not enough:

- PAC settings alone did not solve authority, flattening, stretch, or obstacle displacement
- the path still lacked a stable single authority model between capsule, mesh component, and simulated bodies

Current status:

- old PAC toggles remain in the TestRoom legacy fallback code
- `UT66HeroPhysicsComponent` does not currently use `UPhysicalAnimationComponent`

## Attempt 4: Pure-Chaos / PAC-Off Hit Ragdoll

Evidence folders:

```text
Reports/AgentReviews/HeroPACDisableDecision/
Reports/AgentReviews/ShortHeroRagdollIncap/
Reports/AgentReviews/FriendSlopRagdollCameraFollow/
Reports/AgentReviews/FriendSlopRagdollFollowGroundGuard/
```

What it did:

- disabled PAC for the hit reaction
- simulated the skeletal body as Chaos ragdoll
- iterated follow/camera/floor guards and short incapacitation

What it taught:

- full skeletal simulation gave a clearer physical read than launch-only knockback
- the actor/camera must not remain at the old capsule XY while the ragdoll moves
- recovery needs floor-aware capsule placement
- PhysicsAsset and body orientation errors can create flattening or unstable motion

Why it was not final:

- the early version still had flattening, poor displacement, and recovery/follow problems
- it was a prototype path, not yet a clean component contract

Current status:

- evolved into the current hit-triggered full ragdoll component

## Attempt 5: Always-On PAC/Hip-Anchor Authority Model

Evidence folders:

```text
Reports/AgentReviews/FallGuysPhysicsArchitectureAssessment/
Reports/AgentReviews/FallGuysFullPhysicsRoadmapReview/
Reports/AgentReviews/HeroActiveRagdollStage3Plan/
Reports/AgentReviews/HeroActiveRagdollStage3Implementation/
Reports/AgentReviews/HeroRagdollSystemDiagnostic/
Reports/AgentReviews/HeroRagdollStage3AuthorityRebuild/
```

What it proposed:

- capsule remains gameplay authority
- mesh component stays kinematic and attached to capsule authority
- simulated pelvis/body chain is constrained to a capsule-following hip anchor
- PAC drives child bodies as muscle
- states such as `Balanced`, `Staggered`, `KnockedDown`, and `Recovering`

Why it was superseded for now:

- the current live code no longer contains that hip-anchor/PAC/state-machine implementation
- the working implementation after the latest fixes is hit-triggered full ragdoll with get-up
- keeping the always-on authority model in current docs caused new-agent confusion

Current status:

- archived as a future design direction and historical attempt
- do not treat it as implemented until the source code has the hip anchor, PAC, and state machine again

## Current Implemented Approach: Hit-Triggered Full Ragdoll

Main owner:

```text
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp
```

Key implementation facts:

- current states are `Normal`, `Ragdoll`, and `GettingUp`
- normal state is capsule/CharacterMovement authority
- normal mesh is attached, non-simulating, and collisionless
- `ApplyPhysicsReaction` enters ragdoll only from `Normal`
- ragdoll mesh uses full skeletal simulation and `SimulationUpatesComponentTransform`
- body launch is clamped by `FT66HeroPhysicsProfile`
- actor XY follows pelvis while ragdolled
- get-up places capsule under pelvis/floor and restores the normal mesh transform
- TestRoom wipeout arm routes to this component first by default

Current proof status:

- user verified the latest PIE behavior works after the build/facing fixes
- staged standalone was refreshed in the preceding pass
- final feel tuning remains open

## Current Do-Not-Confuse List

These terms may still appear in names for compatibility, but they do not mean the always-on model is currently live:

- `EnableActiveRagdoll`
- `WipeoutArmUseHeroActiveRagdoll`
- `ShutdownActiveRagdoll`
- `TryInitializeActiveRagdoll`

Interpret those names as the current Hero 1 hit-triggered ragdoll route unless the source changes.
