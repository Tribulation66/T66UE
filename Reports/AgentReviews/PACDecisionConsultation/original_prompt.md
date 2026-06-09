User question:

But will PAC eventually be enabled or not?

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: answer whether Physical Animation Component should eventually be enabled again in the T66 ragdoll architecture, based on the current implementation and project constraints.
Stop condition: give a clear recommendation, including what stays pure ragdoll now and what must be true before PAC is reintroduced.

Relevant repo/process rules:
- C:\UE\T66\AGENTS.md requires Codex operator / Claude validator loop for T66 prompts.
- Current operator state is Codex operator, Claude validator.
- This is consultation only; no code changes are requested.
- Current code deliberately disables PAC when a knockback profile detaches the mesh for ragdoll. The question is architecture direction, not a tuning request.
