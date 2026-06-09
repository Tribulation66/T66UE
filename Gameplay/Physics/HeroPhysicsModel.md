# Hero Physics Model

## Scope

This file describes the Hero 1 Chad physics model as implemented now and the asset expectations that support it.

For the chronological path of attempted approaches, read `HISTORY.md`.

## Current Hero 1 Runtime Model

Current runtime owner:

```text
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h
Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp
```

Current states:

```text
Normal
Ragdoll
GettingUp
```

The live model is hit-triggered full ragdoll. It is not always-on active ragdoll.

## Normal Play

Normal play is standard playable character movement:

- capsule is collision authority
- `CharacterMovementComponent` is locomotion authority
- skeletal mesh is attached to the capsule
- skeletal mesh runs the normal animation assets
- skeletal mesh has no collision and no body simulation
- component tick is disabled

This avoids normal-movement stretching and makes the playable pawn stable before any obstacle hit.

## Ragdoll Entry

`ApplyPhysicsReaction` enters ragdoll only from `Normal`.

Entry sequence:

1. Clamp requested hit velocity through the `FT66HeroPhysicsProfile`.
2. Suppress movement/look input and auto-attack.
3. Stop movement and set movement mode to `MOVE_None`.
4. Disable capsule collision.
5. Detach mesh with world transform preserved.
6. Enable mesh collision and full skeletal simulation.
7. Set all body physics blend weight to 1.
8. Apply launch velocity to simulated bodies.
9. Apply mass-scaled pelvis impulse at the hit location.
10. Enter `Ragdoll` and enable tick.

`ConfigureRagdollMesh` currently uses full skeletal simulation with `SimulationUpatesComponentTransform`.

## Ragdoll Update

During `Ragdoll`:

- pelvis world transform is read from the resolved pelvis body
- actor XY is moved to pelvis XY so the follow target stays near the body
- settle speed and settle hold determine early get-up
- max ragdoll seconds force get-up if the body does not settle
- debug samples can log capsule, pelvis, mesh, velocity, distance, impulse, and simulated body count

## Get-Up

`GettingUp` is entered from `Ragdoll`.

Get-up sequence:

1. Read pelvis transform and determine face-up/face-down.
2. Trace down to find floor.
3. Place capsule under pelvis/floor position.
4. Disable skeletal simulation and CCD.
5. Re-enable capsule collision/profile.
6. Attach mesh back to capsule.
7. Restore the default mesh relative transform.
8. Play front/back get-up animation when available.
9. Blend physics weight down.
10. Restore `MOVE_Walking` and clear suppression.

## Source Model And Asset Path

Raw source:

```text
Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb
```

Physics-first output root:

```text
Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/PhysicsFirstHero/
```

Current runtime row:

```text
Content/Data/CharacterVisuals.csv
Hero_1_Chad
```

Current row expectation:

- physics-first skeletal mesh
- physics-first `Idle`, `Walk`, `Jump`, and `Leap`
- mesh yaw currently `-90`
- static demo-skin row remains separate

## Rigging Requirements

Use:

```text
Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md
```

Required skeleton:

```text
root
  pelvis
    spine_01
      spine_02
        spine_03
          neck_01
            head
          clavicle_l
            upperarm_l
              lowerarm_l
                hand_l
          clavicle_r
            upperarm_r
              lowerarm_r
                hand_r
    thigh_l
      calf_l
        foot_l
          ball_l
    thigh_r
      calf_r
        foot_r
          ball_r
```

Required rigging gates:

- one deform armature
- one root
- real pelvis under root
- three nonzero spine bones
- distinct head, arm, and leg chains
- no helper/control bones exported as deform bones
- no near-zero required bones
- no unweighted vertices
- maximum four influences per vertex
- normalized weights
- bend proof at shoulders, elbows, hips, knees, ankles, and neck
- four-axis facing proof
- exported FBX re-import validation

## Animation Requirements

Current clip set:

- `Idle`
- `Walk`
- `Jump`
- `Leap`
- `GetUp_Front`
- `GetUp_Back`

Style rules:

- simple, readable, slightly bouncy poses
- no root motion for normal movement clips
- no `Run` clip requirement in the Hero 1 MVP
- avoid high-frequency limb motion that fights ragdoll entry/readability
- preserve Chad silhouette and generated texture identity

## PhysicsAsset Requirements

Current hit-triggered full ragdoll requires:

- pelvis body present and resolvable
- simulation root body present and resolvable
- enough body coverage for readable full-body tumble
- joint limits that avoid inversion
- mass/inertia distribution that does not collapse into a flat plank
- floor contact that does not snag permanently
- bodies that can tolerate all-body simulation during the hit window
- get-up animations that visually cover the return from ragdoll to capsule control

Current seed report:

```text
Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_physics_asset_report.json
```

## Retired Or Future Runtime Concepts

Do not treat these as current without source changes:

- always-on active ragdoll
- PAC/Physical Animation Component muscle drive in `UT66HeroPhysicsComponent`
- kinematic hip anchor
- anchor-to-pelvis constraint
- `Balanced`, `Staggered`, `KnockedDown`, `Recovering`
- normal-runtime pelvis/body resync loops

These ideas are documented in `HISTORY.md` as historical/future context.
