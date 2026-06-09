Original user request:

Okay, so go ahead and rebuild Stage 3 around the authority model you and Claude agree on, so that we have the correct system, and then update all the docs to remove the old outdated information on how we used to do it with the new process, and then we'll test out with this new infrastructure that the two of you built.

Working task:
Operator: Codex
Validator: Claude
Scope: Rebuild Stage 3 Hero 1 active-ragdoll around a single authority model, update stale physics docs to describe the new process, and verify the new infrastructure in the playable/TestRoom path.
Stop condition: Code/docs are changed, focused verification is run, and Codex reports what passed, what failed, and what still needs tuning.

Repo rules:
- Read C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Treat this as a read-only independent answer. Do not edit files.
- Current operator is Codex; validator is Claude.
- Physics owner docs are under Gameplay/Physics, especially PHYSICS_AGENTS.md, MASTER_PHYSICS.md, HeroPhysicsModel.md, PhysicsReactionProfiles.md, and PhysicsAssetPipeline.md.
- Movement/trap docs may be relevant: Gameplay/Movement/MASTER_MOVEMENT.md and Gameplay/Traps/MASTER_TRAPS.md.

Current reported failure to solve:
- Prior flattening was reduced, but normal movement now stretches/spazzes as if the mesh is being pulled.
- The wipeout arm barely moves the character and does not feel like active ragdoll physics.
- The user wants a comprehensive authority-model rebuild, not another narrow tuning patch.

Architecture to review:
- Capsule remains gameplay/input/navigation authority.
- Skeletal mesh has an always-on simulated pelvis/central body for active ragdoll.
- A kinematic hip anchor follows the capsule.
- A physics constraint connects hip anchor to simulated pelvis.
- Physical Animation Component acts as muscle/pose target drive, not the gameplay authority.
- Obstacles apply physical reaction profiles: body impulse at hit/contact location, temporary loosening of pose/anchor, bounded capsule shove for gameplay displacement.
- Actor-to-pelvis teleport/follow loops should not be the normal authority path.

Please provide:
1. Independent implementation plan for this Stage 3 rebuild, repo-grounded.
2. The top risks or code traps Codex should avoid.
3. Any doc updates that must happen in the same change.
4. Focused verification you expect before calling the pass done.
