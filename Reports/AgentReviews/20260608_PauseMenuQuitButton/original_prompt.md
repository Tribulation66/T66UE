Original user request:

Oh ok, I actually want to to add a button in the pause menu that says Quit right now we only have Save and Quit so add that button. So do that right now.

Task contract:

Working task:
Operator: Codex
Validator: Claude, if the current repo state still selects it and the helper is available
Scope: add a separate `Quit` button to the pause menu while keeping the existing `Save and Quit` action intact.
Stop condition: pause-menu code is updated, focused verification is run or clearly caveated, and exact proof is reported.

Relevant repo rules:
- Do not use native goal tools.
- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Codex is Operator and Claude is Validator according to `.t66/operator-state.json`.
- UI owner docs apply: `UI/UI_AGENTS.md` and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.
- Use existing shutdown owner for quit/pre-exit behavior.
