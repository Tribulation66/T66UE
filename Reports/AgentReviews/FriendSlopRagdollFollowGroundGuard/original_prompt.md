User request:
There is still that bug where when im hit on the outer side of the obstacle, my hero moves back to close to the spawn point and the camera remains unnatched to the hero, meanwhile one thing we need a hard gate against is part of the body going halfway underneath the ground. Lets solve these two problems now.

Working task:
Operator: Codex
Validator: Claude
Scope: Fix two active TestRoom ragdoll bugs: outer-arm hits must keep the actor/camera attached to the thrown hero instead of snapping back near spawn, and ragdoll bodies must have a hard floor-penetration guard.
Stop condition: Root cause is patched in runtime code, focused build and TestRoom/staged proof are run, and any remaining manual feel checks are clearly stated.

Repo/process rules:
- C:\UE\T66 root AGENTS.md is authoritative.
- Codex is Operator, Claude is Validator from .t66/operator-state.json.
- No native goal tools.
- Gameplay runtime changes require compile/build verification and staged standalone validation.
- Prefer shared/runtime infrastructure over TestRoom-only symptom masks when the bug is in ragdoll/knockback behavior.
- Do not use Anthropic API; local Claude Code CLI only. ANTHROPIC_API_KEY was false in Process/User/Machine.

Context:
- User is testing the TestRoom center-pivot wipeout arm and Hero_1 FriendSlop skeletal ragdoll path.
- Previous fix replaced the arm's overly tall vertical hit band with capsule-vs-cylinder overlap. Current bug is separate: on outer-side arm hits, the hero visually ends near the arm while actor/camera follow target appears near spawn or disconnected.
- User also wants a hard gate that prevents ragdoll body parts from going halfway underneath the ground.

Please inspect current source read-only and provide:
1. Probable root cause(s).
2. Recommended code-level fix location(s).
3. Any verification hooks/logs that should be added or used.
4. Risks or things Codex should avoid.
