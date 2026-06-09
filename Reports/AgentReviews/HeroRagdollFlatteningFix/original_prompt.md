# User Prompt

Ok go ahead and do it, use the blender mcp if it would help you keep working until we no longer have the flattening effect you cannot stop until you solve this problem go ahead and use /goal if it would help the long task, i know usually we dont allow it but im allowing it for this.

# Task Contract

Operator: Codex
Validator: Claude

Scope: Implement and verify the Hero 1 PhysicsFirst PhysicsAsset/rig tuning needed to eliminate the active-ragdoll flattening effect under the TestRoom `heroactiveragdollproof` impact. Use Blender MCP only if live inspection or rig-side correction is needed; otherwise fix the Unreal PhysicsAsset/physics configuration directly.

Stop condition: Stop only when current proof shows Hero 1 remains visually coherent through impact and recovery, or when a hard engine/tool limitation blocks further progress and is documented with evidence.

# Process Notes

- User explicitly approved goal usage for this task; a native goal was created for long-running task tracking.
- Follow `Gameplay/Physics/PHYSICS_AGENTS.md`, `Gameplay/Physics/PhysicsAssetPipeline.md`, and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Do not use the retired Animated ToonStyle / AccuRig / bakeoff rig paths.
- Runtime physics proof must use Unreal-owned capture/video, not desktop screenshots.
- The intended fix class is PhysicsAsset/rig stability: body primitive volume, joint limits, self-collision policy, mass/inertia/solver stability, then PAC/anchor tuning last.

# Current Evidence

- `Source/T66/Gameplay/Physics/pending_issues_Physics.md` documents flattening/spiky silhouette and repeated body resyncs.
- `Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_physics_asset_report.json` records 18 bodies / 17 constraints but not primitive radii, angular limits, self-collision policy, solver iterations, or inertia.
- `UT66HeroPhysicsComponent` already initializes active sim below pelvis, PAC drive, hip/pelvis anchor, active-first obstacle routing, and recovery states.
