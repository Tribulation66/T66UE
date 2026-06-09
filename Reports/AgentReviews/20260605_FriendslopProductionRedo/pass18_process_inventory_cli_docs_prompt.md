# Pass18 FriendslopStyle Process Doc Update - Validator Prompt

Original user request:

> Ok so go ahead and do that, also one very important rule for every iteration of generation, this has to be done by deploying a codex CLI program and run the imagegen through it, instead of in the chat in the codex app, we need to officialize there is a way to do this without any API, we have this in the process docs somewhere already, make the changes you mentioned above and this change to our docs, than give me a prompt for a fresh chat to do an iteration, which means going through every element regenerating what needs to be regenerated placing it and telling me of the elements how many had to be regenerated and then giving me the side by side of the reference image and the product.

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: update FriendslopStyle UI process docs so every screen uses a full per-element inventory loop, every pass evaluates every element, failures route to regenerate/rebuild, and all image generation for iteration work is run through a local Codex CLI worker using account-backed imagegen with no API key; then provide a fresh-chat prompt for the next Main Menu iteration.
Stop condition: docs edited, basic text verification run, Claude validator consulted, and a reusable prompt returned.
```

Relevant repo rules:

- Do not use native goal tools.
- Codex is Operator, Claude is Validator per `.t66/operator-state.json`.
- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `UI/UI_AGENTS.md`, `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`, `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`, and `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` is the single FriendslopStyle process authority.
- Image generation for repo-bound Friendslop iteration work should be formalized as separate local Codex CLI workers using account-backed built-in imagegen; no `OPENAI_API_KEY` or API scripts.

Please independently answer:

1. Which active docs should Codex edit?
2. What exact rule shape should be encoded for the full-screen element inventory loop?
3. What exact rule shape should be encoded for Codex CLI imagegen workers?
4. What pitfalls should Codex avoid in the final fresh-chat handoff prompt?

Return `Result: OK` if Codex can implement internally, or `Result: NEEDS_USER` only for a real user-only decision.
