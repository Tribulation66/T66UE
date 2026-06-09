Original user request:

Ok the next thing I want you to do is to build a tool tip system, what I basically want to do is if your mouse hovers over anything, it should have a tool tip explaining it, like permenant or temporary powerups, if I tooltip over a stat of an item in the vendor, over a game, in the gambler, this is going to be very extensive, so go ahead and think with claude what the best infrastructure for this would be. And give me an extensive list of what could have a tooltip. Under this philosophy

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: design the tooltip infrastructure strategy and produce an extensive tooltip coverage list across UI/gameplay/economy/gambler/powerup surfaces; no code edits until architecture and rollout scope are accepted.
Stop condition: Claude and Codex produce repo-grounded infrastructure recommendations, Codex synthesizes the best plan, and the user receives the coverage list plus concrete implementation phases.

Relevant repo rules:
- Do not use native goal tools.
- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Codex is Operator and Claude is Validator according to `.t66/operator-state.json`.
- UI owns frontend Slate tooltips and verification. Read `UI/UI_AGENTS.md` and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.
- Gameplay/economy/gambler content must be validated against live source/data before implementation.
- This is a planning/infrastructure pass only; no mutation beyond AgentReview artifacts.
