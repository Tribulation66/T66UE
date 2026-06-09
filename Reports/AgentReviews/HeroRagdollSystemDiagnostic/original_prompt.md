Working task:
Operator: Codex
Validator: Claude
Scope: Read-only deep diagnostic of why the current Hero 1 active-ragdoll setup still stretches/spazzes during movement and barely reacts to obstacle hits; include Unreal-grounded examples/patterns and decide what system shape is likely correct before any implementation.
Stop condition: Deliver a repo-grounded diagnostic with 5-10 likely causes, evidence from current files/proof, external Unreal reference patterns, and a recommended architecture direction. No code or asset changes.

User request:
The user reports that the previous flattening defect is not truly solved. The model no longer lays flat when hit, but now stretches/spazzes during movement as if constantly being pulled somewhere. The TestRoom rotating platform/arm barely moves the hero on impact and does not feel like Fall Guys-style ragdoll/physics. The user explicitly wants no implementation now. They want Codex and Claude to step back, diagnose the whole system, avoid single-setting tunnel vision, find 5-10 reasons it is not working, and compare against Unreal examples/settings/patterns where ragdoll/active-ragdoll mechanics work.

Repo rules:
- Read-only diagnostic. Do not edit code/assets.
- Use live repo state, current physics docs, current movement docs, and current proof artifacts.
- Validator should produce an independent repo-grounded answer first, then Codex will synthesize.
- Focus on system architecture: PAC vs constraints vs collision vs movement authority vs animation/rigging, not just one patch.

High-value files to inspect:
- C:\UE\T66\Gameplay\Physics\PHYSICS_AGENTS.md
- C:\UE\T66\Gameplay\Physics\MASTER_PHYSICS.md
- C:\UE\T66\Gameplay\Physics\HeroPhysicsModel.md
- C:\UE\T66\Gameplay\Physics\PhysicsReactionProfiles.md
- C:\UE\T66\Gameplay\Movement\MASTER_MOVEMENT.md
- C:\UE\T66\Source\T66\Gameplay\Physics\T66HeroPhysicsComponent.h
- C:\UE\T66\Source\T66\Gameplay\Physics\T66HeroPhysicsComponent.cpp
- C:\UE\T66\Source\T66\Gameplay\Physics\pending_issues_Physics.md
- C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_TestRoom.cpp
- C:\UE\T66\Source\T66\Gameplay\T66HeroBase.cpp
- C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Movement.cpp
- C:\UE\T66\Source\T66\Gameplay\Movement\T66HeroMovementTypes.h
- C:\UE\T66\Content\Data\CharacterVisuals.csv
- C:\UE\T66\Reports\AgentReviews\HeroRagdollFlatteningFix\physicsfirst_asset_final_reportonly.json

Specific evidence to check:
- Does active ragdoll have one clear motion authority, or do capsule movement, simulated bodies, PAC, constraints, animation, and manual actor follow all fight?
- Is the obstacle using real physics contact or a geometric/timer launch?
- Are WorldDynamic/PhysicsBody collision responses configured so obstacles can physically hit the mesh/bodies?
- Is the requested obstacle velocity being clamped or converted to CharacterMovement instead of physics?
- Does the pelvis/root simulation choice match the hip-anchor/world-anchor model in the docs?
- Does PlayAnimation/simple animation asset playback provide a stable animated target for PAC while bodies simulate?
- Are proof artifacts only showing the previous flattening case rather than movement + hit + recovery?

Requested answer:
- 5-10 likely root causes, with repo evidence.
- Opinion on whether PAC should be used and under what architecture.
- Unreal examples/patterns/settings to emulate, with source names/links if known.
- Recommended next architecture/verification plan, but no implementation.
