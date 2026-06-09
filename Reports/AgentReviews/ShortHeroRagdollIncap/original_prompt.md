User request:

Okay, the next thing I want you to do is to reduce the debilitated state, the ragdoll state where movement isn't working. It should be after, during contact, obviously a ragdoll, but then after contact, you should only be half a second, and then you're back up and can move again. Let's make that change.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: shorten the hero wipeout/ragdoll incapacitation window so control returns about 0.5s after contact, without changing broader ragdoll infrastructure.
Stop condition: identify the controlling duration path, patch the relevant defaults/CVars, run focused build and staged standalone refresh, and report any remaining feel-tuning caveat.

Relevant repo/process rules:
- C:\UE\T66\AGENTS.md requires Codex operator / Claude validator loop for T66 prompts.
- Current operator state is Codex operator, Claude validator.
- Runtime gameplay changes need focused compile verification and staged standalone validation when they affect the playable standalone.
- Current code restores movement only when `RestoreFromKnockback()` runs after active ragdoll and recovery blend-out, so the tuning must account for min incap, forced ragdoll max, settle hold, and blend-out.
