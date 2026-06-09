# Original User Request

User reports that after the hero-only FriendSlop knockback/ragdoll infrastructure pass:

- The mesh still has a drag/smear/stretch effect during ragdoll.
- After knockdown, camera/player-control behavior feels disconnected: the camera can move but the character/model is not moving with it, and the camera/model location relationship is not working.
- We need to keep improving ragdoll quality, not treat this as small numeric tuning.

User provided screenshots showing the FriendSlop mesh flattened/stretched on the ground and the camera/ring/crosshair separated from the ragdolled model near the wipeout arm.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: diagnose and implement the next hero-scoped infrastructure fixes for the ragdoll smear/drag and camera/controller disconnect while keeping the core knockback architecture reusable later.
Stop condition: root causes are addressed in code/data where possible, focused compile and Unreal-owned TestRoom proof are run, staged standalone validation is attempted if gameplay runtime changes are made, and remaining visual/rigging limitations are stated honestly.

# Relevant Repo Rules

- Root router is `AGENTS.md`.
- Codex is Operator, Claude is Validator per `.t66/operator-state.json`.
- Every prompt goes through independent Validator answer then cross-review when Claude helper is available.
- Gameplay owner: `Gameplay/GAMEPLAY_AGENTS.md`; read `Gameplay/README.md`, relevant Movement and Camera docs.
- Runtime gameplay changes require compile/build verification and staged standalone validation when playable standalone is affected.
- This is visual/physics/animation process-governed work; preserve method class: skeletal ragdoll / Chaos / Physical Animation Component, not fake static lookalike.
- User wants hero scope for now, but infrastructure should not block future bosses/elites.

# Current Implementation Context To Inspect

- `Source/T66/Gameplay/T66KnockbackComponent.{h,cpp}` owns the new profile-driven skeletal ragdoll path.
- `AT66HeroBase` owns `CameraBoom` / `FollowCamera`, plus raw `PlayAnimation()` paths.
- `AT66PlayerController` owns movement input, camera input/zoom, and combat camera queries.
- TestRoom wipeout arm now routes through `GetKnockbackComponent()->ApplyKnockbackLaunch(...)`.

# Requested Validator Output

Give an independent repo-grounded answer:

1. What likely causes the visible drag/smear and what should be fixed in Unreal vs Blender/rig/PhysicsAsset?
2. What likely causes the camera/model disconnect and what code path should own ragdoll camera following?
3. What implementation changes should Codex make now, scoped to hero but reusable later?
4. What verification should be required before reporting success?

Do not mutate files.
