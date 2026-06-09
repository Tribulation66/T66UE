# Original Prompt

Ok I want you to do the hero selection and companion selection screens next.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Run the FriendslopStyle implementation loop for the Hero Selection and Companion Selection screens, preserving existing content/layout and not regenerating shared top-bar chrome unless a screen-local implementation requires it.
Stop condition: Both screens have reference art, family breakdowns, generated runtime elements, source wiring, and current compile/capture/dump evidence, or a process blocker is reached that T66 rules require stopping on.

# Repo Rules And Constraints

- Start from live repo state under `C:\UE\T66`.
- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `UI/UI_AGENTS.md`, `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`, and `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- Use Codex as Operator and Claude as Validator per `.t66/operator-state.json`.
- Claude must be invoked through the local helper, with no Anthropic API key in Process/User/Machine environment scope.
- FriendslopStyle visual work must use account-backed image generation through separate local Codex CLI workers, not the main chat image tool and not API-key scripts.
- Preserve live labels, player data, scores, localization, and interaction behavior in Slate/code rather than baking them into PNGs.
- Shared frontend top bar is already shared chrome and should not be regenerated for these screen-local passes.
- Codex reports wiring/functionality PASS/FAIL and visual evidence paths; the user owns final visual judgment for FriendslopStyle.
- Use Unreal-owned capture/dump scripts for proof.
