# Codex Validation - Claude Heavy Operator Analysis

## Scope

Validate Claude's operator artifact for the user question:

> What actually consumes heavy token usage, and how should the T66 stack make Claude the true 70-80% heavy Operator while Codex remains Validator/Integrator?

## Live Evidence

- Canonical state: `.t66/operator-state.json` reports `Operator=Claude`, `Validator=Codex`.
- `AGENTS.md` currently defines Claude Operator as heavy planning/investigation/proposal generation, but leaves Codex responsible for goal tracking, repo integration, edits, final verification, and user-facing completion reports.
- `AGENTS.md` currently keeps the baseline Claude Operator profile read-only unless a reviewed plan and user approval widen it.
- `Scripts/Invoke-ClaudeDirectRead.ps1` supports broader tools and permission modes, but blocks broader tools unless `-AllowBroaderTools` is explicit.
- Claude operator artifact: `Reports/AgentReviews/ClaudeDirectRead/20260529T010103-20260529_ClaudeHeavyOperatorAnalysisComplete-Operator/claude_direct_read_operator.md`.
- Claude manifest: `Reports/AgentReviews/ClaudeDirectRead/20260529T010103-20260529_ClaudeHeavyOperatorAnalysisComplete-Operator/manifest.json`.
- Claude CLI JSON probe showed per-run usage data is available from `--output-format json`, including `usage`, `modelUsage`, token counts, cache creation/read tokens, and cost fields.
- `ANTHROPIC_API_KEY` was false in Process, User, and Machine scopes after the Claude runs.

## Validation Result

Codex agrees with Claude's central finding: the user's 80/20 assumption is directionally right for cognitive work, but incomplete as a strict token/accounting claim.

Heavy usage is mostly caused by:

1. Fixed CLI/model/system-context overhead per run.
2. Repo/process context ingestion.
3. Deep reasoning over inspected files.
4. Duplicate re-reading by the non-operator after handoff.
5. Verification/debug loops.
6. Long final reports and repeated process restatement.

The decisive point is that file editing itself is not usually the dominant token cost. Giving Claude `Edit`/`Write` only to raise Claude's share would add risk without addressing the largest driver: duplicated context ingestion.

## Recommended Process Direction

Use two operating modes:

1. **Claude Patch-Ready Operator, read-only baseline**
   - Claude owns deep investigation and writes a patch-ready operator packet.
   - Packet must include exact file paths, intended diffs or before/after anchors, commands to run, expected verification evidence, and known risks.
   - Codex validates only cited anchors and applies/verifies, avoiding a full duplicate investigation.
   - This is the safest way to approach a 70-80% Claude heavy-work split without widening permissions.

2. **Claude Isolated Text-Edit Operator, optional later**
   - Only after user approval and a reviewed plan.
   - Run Claude in a temporary worktree or isolated patch workspace.
   - Allowed scope: low-risk text files only, with enumerated path allowlist.
   - Prohibited scope: `.uasset`, generated binaries, production asset writes, Unreal Python, editor automation, unrestricted shell, bypass permissions.
   - Codex reviews the generated diff and performs final repo integration and verification.

## Required Changes If Implemented Later

- Update `AGENTS.md` with a "Claude Operator Packet Contract".
- Add a "Codex Validator Budget" rule: Codex validates cited anchors first and avoids full re-reading unless the packet is underspecified, risky, or contradicted by repo state.
- Add a helper option or new helper for Claude JSON output so manifests capture per-run `usage` and `modelUsage`.
- Add a per-task usage ledger under `Reports/AgentReviews/<TaskSlug>/usage_ledger.json`.
- Optionally add a separate isolated text-edit helper after a reviewed plan and user approval.

## Risks

- A weak Claude packet forces Codex to re-investigate, destroying the target split.
- Too many small Claude runs waste fixed overhead; prefer one substantial operator run per Tier 1 task.
- Direct edit permissions increase safety risk without guaranteed token savings.
- A strict 80/20 validator target can undercut validation quality if treated as a hard cap.

## Codex Recommendation

Proceed first with the read-only patch-ready Operator model and usage instrumentation. Do not enable direct Claude edits yet. Revisit text-only isolated edits only if measurement proves Codex re-read/application work remains too high after the packet contract is enforced.
