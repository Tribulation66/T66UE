Implementation complete. All approved simplifications are in place, validated by parse-check and six preflight shapes (none invoked Claude).

**Files changed**
- `Scripts/Invoke-ClaudeDirectRead.ps1` — timeout policy, `-Preflight`, manifest capability fields, token sidecar, verification-freshness prompt rules.
- `Scripts/Invoke-ClaudeReadOnlyOperator.ps1` — new approval-free read-only Operator wrapper.
- `OPERATOR_VALIDATOR_PROTOCOL.md` — timeout/preflight/read-only docs, manifest field docs, verification-freshness invariant, evidence-preservation section.
- `AGENTS.md` — read-only wrapper + preflight pointer and verification-freshness rule (scoped narrowly).

**Behavior delivered**
1. Default timeout guard kept; FullOperator default raised to `1200s`, read-only `180s`; `-TimeoutSeconds 0` = explicit unbounded (now blocks instead of timing out instantly).
2. `-Preflight` prints effective config and exits before any Claude/CLI/auth/API-key call; no prompt file needed; approval status reported non-fatally.
3. `EffectiveToolSurface` / `MutatingCapability` / `ApprovalRequired` added to both manifests; legacy `AllowedTools` retained.
4. `Invoke-ClaudeReadOnlyOperator.ps1` forces `-Mode Operator -ToolProfile ReadOnly`.
5. Verification-freshness hard rule in prompt headers + both docs.
6. `claude_tokens.json` sidecar (success + failure), recorded as `TokenSummaryPath`; Claude output untouched to preserve verdict parsing.
7. Evidence-preservation proof-class list added to the protocol.

**Validation**: both scripts parse clean; preflight read-only / FullOperator+approval / wrapper / unbounded / no-approval / explicit-timeout all behaved correctly without spending Claude tokens.

Completion packet: `Reports/AgentReviews/20260529_ClaudeHelperSimplification/claude_completion_packet.md`.

This is an Operator work artifact, not a greenlight — no git commit/push was performed, leaving validation and the final report to Codex. Key caveats for you: the unbounded `WaitForExit()` path was only static/preflight-validated (not run against a live unbounded Claude process), and `Reports/AGENTS.md` was deliberately left untouched as out of write scope.
