User request:
Ok the thing we should have is that no trap should be static, meaning the bumper should be going up and down, and the wall bumper should be going out and back. The swining axe and rotating arm are fine, for the other two lets simplify them one is a floor bumper and the other a wall bumper both have the same movement, but the placement is different. So make those changes not just in the test room but the trap itself.

Working task:
Operator: Codex
Validator: Claude, if the local validator path is available
Scope: update the production trap actors so the floor bumper and wall bumper are moving traps rather than static, simplify the prior launch-pad/bumper pair into floor-bumper and wall-bumper variants, and make the TestRoom use those updated trap types.
Stop condition: code changes are implemented, focused build/staged runtime verification is attempted, and any blocker or unverified behavior is reported.

Relevant repo rules:
- Start from live repo state and current folder instructions.
- `Gameplay/GAMEPLAY_AGENTS.md` owns trap runtime changes; runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect playable standalone.
- `Gameplay/Traps/MASTER_TRAPS.md` owns the trap subsystem and obstacle trap family docs.
- Codex is current Operator and Claude is Validator per `.t66/operator-state.json`.
- Claude Validator runs must be read-only and must use local Claude Code subscription auth, not Anthropic API billing.
