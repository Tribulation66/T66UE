User request:

Ok great, so you did find the descrapancy I was worried about, so the reason we have that discrepency is ebcause before we went in the direction of the flat style, we had an imagegen based chrome approach, which we got rid of. So the first thing we need to do is fully clean that up and remove that, as well as everything that is not part of the clear and obejctive pipeline and seems to be relics or unrelated instructions we can get rid of. Let me know when done.

Task contract:

Working task: Clean up T66 UI documentation/process instructions by removing the obsolete imagegen-based chrome pipeline and stale/unrelated relic instructions, keeping a clear objective flat/Slate UI pipeline.
Operator: Codex
Validator: Claude
Scope: Inspect and edit live repo UI routers, UI instructions, UI reference/process docs, and directly related stale handoff/audit references. No runtime UI implementation, build, capture, release, or git operations unless required by the documentation cleanup.
Stop condition: Updated docs are internally consistent, obsolete imagegen-chrome references are removed from active routing, stale or deleted relic docs no longer conflict with the active pipeline, Claude has reviewed, and Codex reports exact files changed plus verification.

Relevant repo rules:
- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator and Claude is Validator per .t66/operator-state.json.
- Do not use native goal tools.
- Use UI/UI_AGENTS.md as the UI folder router.
- Use Reports/AGENTS.md for review artifacts.
- Preserve the distinction between obsolete imagegen UI chrome and still-allowed content artwork/icon-stub generation in the flat pipeline.
- This is a documentation/process cleanup, not a visual/UI runtime implementation pass.
