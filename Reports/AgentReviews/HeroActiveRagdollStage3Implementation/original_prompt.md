# Original User Request

User approved implementation with:

```text
ok go
```

This approval follows the Stage 3 plan for the Hero 1 Chad active-ragdoll MVP.

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Implement Stage 3 Hero 1 Chad active-ragdoll MVP from the approved plan: owned hero physics component, always-on simulated body, PAC pose drive, hip/pelvis anchor, TestRoom obstacle bridge, and proof hooks as needed. Keep legacy knockback fallback.
Stop condition: Implement the scoped MVP, verify with the strongest feasible compile/runtime evidence, report any partial proof honestly.

## Relevant Repo Rules

- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Codex is Operator and Claude is Validator from `.t66/operator-state.json`.
- This is a process-governed runtime physics task. Use `Gameplay/Physics/PHYSICS_AGENTS.md`, `MASTER_PHYSICS.md`, `HeroPhysicsModel.md`, `PhysicsReactionProfiles.md`, and `PhysicsAssetPipeline.md`.
- The target method is capsule-backed always-on active ragdoll for Hero 1 Chad, with local PAC pose drive and a kinematic hip/pelvis anchor constraint.
- Keep `UT66KnockbackComponent` as legacy/prototype fallback until the new active-ragdoll path proves itself.
- Do not revive retired ToonStyle, AccuRig, pure-Chaos/PAC-off, or animation-only fake-wobble paths.
- Runtime physics changes require focused compile and, when feasible, staged standalone plus Unreal-owned multi-frame proof. Desktop screenshots are not valid proof.
- No Git operations are requested.
