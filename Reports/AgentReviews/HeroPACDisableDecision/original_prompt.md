User request:

Okay, so you got at my question here, which is, will we eventually have heroes using pack? It's a yes or no. If the answer is no, then yeah, disable it in the test room. Disable it, not even just in the test room, disable it for heroes. But if it's yes, we need to figure out how to make it work with pack. But I agree, if pack is dragging you to a certain location, then we shouldn't use it, because the idea is to let the chaos do its thing. But that's basically for everything. I mean, even the bosses, I don't want them to come back to a specific place. I want them to, you know, just be chaotic and go where they need to be.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: make the yes/no architecture call on whether heroes should use Physical Animation Component for ragdoll, and if the answer is no, disable hero PAC rather than only disabling it in the TestRoom profile.
Stop condition: clear decision, scoped implementation if no, compile verification, and honest caveats about PAC infrastructure that remains.

Relevant repo/process rules:
- C:\UE\T66\AGENTS.md requires Codex operator / Claude validator loop for T66 prompts.
- Current operator state is Codex operator, Claude validator.
- Runtime gameplay changes need focused compile verification.
- Existing code has `FT66KnockbackProfile` defaults with PAC enabled, but the active detached path force-disables PAC at runtime.
