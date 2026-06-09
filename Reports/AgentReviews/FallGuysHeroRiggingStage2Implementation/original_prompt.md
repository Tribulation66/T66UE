# Original User Request

Okay, you have my approval. Go ahead with the implementation.

# Prior Approved Plan Being Implemented

The approved implementation is Stage 2 of the physics-first hero roadmap:

- Stage 1 physics ownership is complete.
- Stage 2 now becomes a physics-first Hero 1 Chad rigging/animation standard and implementation.
- The original Hero Active Ragdoll MVP moves to Stage 3.
- Hero 1 Chad is the MVP.
- The character is being implemented into the actual game, not just a TestRoom-only prototype. TestRoom is only a proof route because it has an obstacle/trap.
- Replace the stale raw FriendSlop/humanoid rigging philosophy with a Fall Guys-like physics-first hero standard.
- Build/wire Hero 1 Chad with Idle, Walk, Jump, Leap, and useful get-up/recovery pose targets if helpful.
- Remove Roll as the forward ability concept and replace it with Leap.
- Defer Stage 3 active-ragdoll runtime components: capsule stays, mesh always simulatable, pelvis/hip constraint, PhysicalAnimationComponent, reaction profiles.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Implement the approved Stage 2 physics-first Hero 1 Chad rigging/animation foundation: update the roadmap/docs, replace the stale FriendSlop/raw humanoid rigging standard with the new physics-first hero standard, create/import/wire Hero 1 Chad with Idle/Walk/Jump/Leap and useful recovery pose targets, and rename the old Roll concept to Leap where needed for the Hero 1 movement asset path. Stage 3 active-ragdoll runtime components, PAC/hip constraints, reaction profiles, and broad all-hero migration remain out of scope.
Stop condition: Stage 2 docs/assets/code/data are updated, current proof is attempted and reported, Claude cross-check is incorporated, and any blockers are documented without making unrelated destructive changes.

# Current Repo Caveats Observed By Codex

- The working tree already has modified files in this area before new edits, including:
  - `Content/Data/CharacterVisuals.csv`
  - `Source/T66/Data/T66DataTypes.h`
  - `Source/T66/Gameplay/T66HeroBase.h/.cpp`
  - `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
  - `Source/T66/Gameplay/T66PlayerController_Movement.cpp`
  - `Model Generation/Rigging and Animation/README.md`
  - `Model Generation/Rigging and Animation/07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`
  - `Model Generation/Rigging and Animation/Tools/*animated_toonstyle*`
- Codex must not revert those unrelated/current changes. Work with them.
- Existing Hero 1 Chad rigging outputs exist under:
  - `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/Rigging`
- Existing rig report says the rig has useful skeleton/QA but is a deterministic spike, not production animation rig.
- Existing animation manifest has `Idle/Walk/Jump/Roll`, Quaternius-derived roles, no Leap.
- Current code/data still uses Roll naming.

# Requested Claude Output

Produce an independent implementation-scope review before Codex edits production files:

1. Flag risks or missing owner docs Codex should inspect before editing.
2. Say whether docs-first then asset/code pass is the correct sequencing.
3. Identify the smallest safe Stage 2 implementation slice if full Blender/Unreal import becomes blocked.
4. Call out anything that must not be touched in this pass.

Read-only review only. Do not mutate files.
