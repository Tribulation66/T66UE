# User Prompt

Okay, what's the solution you and Claude recommend to avoid the flattening effect? I imagine this is a very, um... This is an issue that occurs often, so I'm sure there's also very clear solutions to it. So what's the guaranteed, confirmed, not guessing solution to avoid a model becoming flat with ragdoll physics?

# Task Contract

Operator: Codex
Validator: Claude

Scope: Answer only, no implementation. Recommend the confirmed solution path for preventing Hero 1 active-ragdoll flattening in T66, grounded in the current Stage 3 proof, the physics docs, and known Unreal physics mechanics.

Stop condition: Provide a clear answer that separates guaranteed mechanical requirements from tuneable values, identifies what must be fixed next, and does not overclaim a magic parameter.

# Relevant Local Context

- `Source/T66/Gameplay/Physics/pending_issues_Physics.md` documents the current flattening/spiky silhouette and repeated body resyncs.
- `Gameplay/Physics/PhysicsAssetPipeline.md` says Stage 2 PhysicsAsset output is seed evidence only and Stage 3 must validate/tune under active simulation, PAC, pelvis anchoring, recovery, and obstacle contact.
- `UT66HeroPhysicsComponent` already initializes active simulation below pelvis, physical animation drive, and a hip/pelvis anchor.
- Stage 3 proof showed `ActiveTried=1`, `ActiveApplied=1`, `LegacyApplied=0`, with state transitions present but body visual stability partial.
