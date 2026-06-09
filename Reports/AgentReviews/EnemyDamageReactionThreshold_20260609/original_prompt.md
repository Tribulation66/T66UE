User request:

> I want to change the physics a bit, not all damage from enemies should throw you and disable you, damage from enemies should only give you knockback and increase your % and then once above 50% damage starts throwing you and disabling you with the distance you are thrown and the duration of the disable scaling up from 50% to 99. At 100 you die

Working task:
Operator: Codex
Validator: Claude
Scope: Change enemy damage behavior so enemy hits always add percent and apply knockback, but only start throw/disable reactions once the hero is above 50%, scaling throw distance and disable duration from 50% through 99%, with death still at 100%.
Stop condition: Live physics/damage ownership is inspected, the behavior is implemented in the correct runtime path, current compile/gameplay proof is run, staged standalone is refreshed if the playable build is affected, Validator review is complete, and token usage is reported.

Relevant repo rules:
- Use live repo state, not stale memory.
- Codex is Operator and Claude is Validator per `.t66/operator-state.json`.
- Claude must be invoked through the local helper after confirming no `ANTHROPIC_API_KEY` is set.
- Runtime physics/gameplay changes require focused compile, staged standalone validation when playable behavior changes, and Unreal-owned capture/log proof.
- Current physics owner is `UT66HeroPhysicsComponent`; normal play is capsule/CharacterMovement, qualifying hit reactions enter hit-triggered full ragdoll.
- Movement has a separate non-ragdoll launch/knockback layer through `LaunchCharacter`.
