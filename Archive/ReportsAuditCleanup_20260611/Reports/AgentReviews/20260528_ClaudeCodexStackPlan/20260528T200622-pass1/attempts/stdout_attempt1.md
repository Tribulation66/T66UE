Verdict: REVISE

## Blockers

None hard. The plan is no-change and scoped to planning, so nothing prevents Codex from iterating on the packet itself.

## Major Issues

- **Opus 4.8 verification is weaker than the packet implies.** Evidence cited is (a) the local CLI accepting `--model claude-opus-4-8` and (b) `modelUsage` echoing that name. Neither rules out alias resolution, silent fallback, or unreleased-model stubs — CLI acceptance is not proof the served model is a distinct Opus 4.8. The packet lists two Anthropic URLs as "checked" but never quotes what they actually returned (HTTP status, headline, listed model IDs). Without that, the Phase 1 pin is built on an unverified premise. Codex should record the fetched page status and a quoted line confirming the model ID, or downgrade the claim to "CLI exposes the name; served model identity unverified."
- **Phase 6 router asymmetry needs sharpening.** "Assign validator/critic work to the model with lower remaining budget" is right only when validator load is small. Direct-read validator passes with `--add-dir` and tool calls can burn the low-budget side fast and starve the next reset. The routing rule should weight by *expected token cost per role*, not just remaining percentage. Today's example (Claude 100%, Codex 19% until 2026-05-30) is fine; the general rule isn't.
- **Phase 6 ledger inputs are underspecified.** "User-supplied before/after percentages and reset dates" — how Codex consumes those (CLI flag, prompt field, sidecar JSON, manual paste into a ledger file) is not defined. Without that contract, the router can't be implemented later without revisiting design.
- **Direct-read Claude scope creep risk.** Phase 2 lists "read/search/glob plus `--add-dir C:\UE\T66`" but doesn't name the exact `--allowedTools` allowlist or whether `--permission-mode` is `plan` vs `acceptEdits` (read-only). The current AGENTS.md strategic-partner constraint demands this be explicit, not "limited tools first."

## Minor Issues

- Phase 1 says "JSON output for accounting" but `claude --output-format json` in this codebase has not been smoke-tested against `Invoke-ClaudePlanReview.ps1`'s current verdict parser. Plan should note the parser test must precede the JSON switch, not follow it.
- Phase 4 reuses the official Blender MCP path. Good, but the packet doesn't address whether Claude Code's MCP scope (user vs project) avoids leaking the Codex `config.toml` path semantics. A one-line note on scope choice would close it.
- Phase 5 says "future narrow Niagara dump helper" without naming a target asset/route to validate against. `CaptureT66NiagaraMRQIsolation.ps1` exists; the helper proposal should either name the first asset to dump or defer the helper entirely until a real use case arises.
- Phase 7 file list is presented as "likely" but reads as a commitment. Should be explicitly marked "candidates, each subject to separate plan review before edit."
- Risks list omits: subscription auth drift (Max login can lapse) and CLI version pinning (2.1.150 today; behavior of `--model` may change). Worth one line each.

## Clarifying Questions

- For the Opus 4.8 verification, did the `anthropic.com/news/claude-opus-4-8` fetch return 200 with a published article, or 404/redirect? Quote the title or status.
- Is the validator role allowed to consume real tokens via direct-read, or must validator passes remain packet-only by policy? This determines the Phase 6 routing math.
- Should the usage ledger be a file in `Reports/` (durable) or session-local? The packet implies durable but doesn't say.
- For Phase 3 Claude operator, does "no direct file edits" mean Claude cannot use `Edit/Write` tools at all (allowlist excludes them), or that it can but must run in `--permission-mode plan`?

## Required Verification

Before any later implementation, the packet's verification list is mostly correct but should add:
- Quoted evidence that Opus 4.8 is a real, served model — not just a CLI-accepted name. (Probe with a known model-identifying prompt, or cite Anthropic docs verbatim.)
- Parser tests pass *before* JSON output is enabled in the live helper.
- Allowlist string for direct-read Claude is recorded and reviewed.
- Ledger schema (fields, file location, write trigger) is fixed before the router is built.

## Rationale

The strategy aligns with T66 instructions: planning-only, preserves cross-validation, defers to Unreal-owned capture proof, reuses the official Blender MCP, and treats subscription denominators as empirical. Those are the right calls. The reason for REVISE rather than APPROVE is concentrated in Phase 1 (Opus 4.8 evidence is circumstantial) and Phase 6 (router rule and ledger contract are not yet implementable). Both are Codex-fixable by tightening the packet — quoting fetched-URL contents, specifying the allowlist and permission-mode, defining ledger inputs, and weighting routing by role-expected cost — without needing a user product decision. Once those are tightened, the plan should be safe to APPROVE.

