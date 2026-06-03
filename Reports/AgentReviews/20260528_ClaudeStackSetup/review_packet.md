# Claude/Codex Stack Setup Implementation Review Packet

## Working Goal

Implement the approved Claude/Codex process shift for T66, excluding quota-denominator math for now: make Claude Opus 4.8 the default heavy operator path, preserve Codex/Claude validation roles, update `AGENTS.md` and supporting scripts/docs so new agents follow the new process safely, and verify the changes without touching Mini scope.

## User Constraints And Assumptions

- The user said to leave the denominator/usage-limit inference issue for later.
- Interpret "set Claude as the denominator" as "make Claude the default heavy operator for now" because the denominator math was explicitly deferred.
- User explicitly asked for `AGENTS.md` changes so new agents do not miss the new process.
- Do not include Mini/minigame systems.
- Preserve the cross-validation/critique loop. The operator can shift, but the other model must remain validator/critic.
- Use Claude through local Claude Code subscription CLI, not Anthropic API billing.

## Live Instructions And Files Checked

- `AGENTS.md`
- `Scripts/README.md`
- `Scripts/pending_issues_Scripts.md`
- `Reports/AGENTS.md`
- `Reports/README.md`
- `Scripts/Invoke-ClaudePlanReview.ps1`
- `Scripts/Test-ClaudeReviewVerdictParser.ps1`
- Repo sweep for `CLAUDE.md`: none found.
- `claude --help` confirmed `--model`, `--effort`, `--permission-mode`, `--add-dir`, `--allowedTools`, `--tools`, and `--output-format`.
- `claude auth status --help` confirms the subcommand supports `--json` and that JSON is the default. The preflight will call `claude auth status --json`.
- `claude auth status --json` reports first-party `claude.ai` subscription login; output contains PII and must not be copied into docs or artifacts except sanitized summaries.
- Sanitized schema fixture from the local CLI:

```json
{
  "loggedIn": true,
  "authMethod": "claude.ai",
  "apiProvider": "firstParty",
  "email": "<redacted>",
  "orgId": "<redacted>",
  "orgName": "<redacted>",
  "subscriptionType": "max"
}
```

## Prior Approved Planning Evidence

- `Reports/AgentReviews/20260528_ClaudeCodexStackPlan/20260528T200833-pass2/claude_review_pass2.md` approved the no-change strategy.
- `Reports/AgentReviews/20260528_ClaudeCodexStackPlan/claude_opus48_probe.json` showed explicit local `--model claude-opus-4-8` use in `modelUsage`.
- Anthropic official model docs list API ID `claude-opus-4-8`.

## Proposed Implementation

### 1. Update `AGENTS.md`

Add a new subsection after `Claude Cross-Review`, tentatively named `Claude/Codex Operator Stack`.

Add this literal subsection after `Claude Cross-Review`:

```markdown
### Claude/Codex Operator Stack

- When the user asks to conserve Codex usage, asks for Claude-led work, or the task would benefit from independent direct repo inspection, make Claude Code with `claude-opus-4-8` the default heavy operator for planning, investigation, and proposal generation.
- Operator routing changes who does the heavier analysis or proposal work; it does not remove the validator/critic requirement. The non-operator model must still review or critique the operator's plan/output under the review rules above.
- In the active Codex workspace, Codex remains responsible for the working goal, repo integration, applying edits, final verification, and user-facing completion report unless the user explicitly changes that.
- The baseline Claude direct-review profile is read-only: `--model claude-opus-4-8 --effort low --permission-mode plan --allowedTools Read,Grep,Glob --add-dir C:\UE\T66`.
- The baseline Claude operator profile is also read-only unless a later reviewed packet explicitly widens it: `--model claude-opus-4-8 --effort high --permission-mode plan --allowedTools Read,Grep,Glob --add-dir C:\UE\T66`.
- Claude must not use `Edit`, `Write`, unrestricted `Bash`, `bypassPermissions`, direct production asset writes, Unreal Python invocation, or editor automation unless a task-specific reviewed plan names the exact tool profile and the user has approved that broader access.
- Claude Blender access must use the local/user-scoped Blender MCP configuration and be smoke-tested before production use. Do not assume Codex MCP configuration automatically grants Claude the same tools.
- Claude Unreal/Niagara visibility must start from direct file reads and existing Unreal-owned capture/dump artifacts. Desktop screenshots, raw GUI observation, or private viewport history are not acceptance proof.
- Review packets and done reports must state which model was operator, which model was validator, what helper/tool profile was used, and where the operator/review artifacts were saved.
- Quota denominator inference and automatic usage routing are deferred until a separate usage-ledger/router pass is reviewed and implemented.
```

The subsection will make clear:

- Claude Opus 4.8 is the default heavy operator when the user asks to conserve Codex usage, asks for Claude-led work, or the task benefits from independent direct repo inspection.
- Codex remains responsible for goal management, repo integration, user-facing reporting, and final verification in this active Codex workspace unless the user explicitly changes that.
- The non-operator model must still validate/critique. The stack change moves heavy lifting, not the review invariant.
- Claude operator baseline is read-only/direct-read first: `--model claude-opus-4-8 --effort high --permission-mode plan --allowedTools Read,Grep,Glob --add-dir C:\UE\T66`.
- Claude direct reviewer baseline is cheaper: same read-only tool set, `--effort low`.
- Claude must not use `Edit`, `Write`, unrestricted `Bash`, `bypassPermissions`, or direct production asset writes unless a later per-task plan/review explicitly approves that tool profile.
- Blender access for Claude uses the local/user-scoped Blender MCP configuration and must be smoke-tested before production use.
- Unreal/Niagara access for Claude should start from existing Unreal-owned capture/dump scripts. Desktop screenshots or raw GUI observation are not acceptance proof.
- New agents must record operator/validator roles and artifacts in review packets/done reports.
- Usage-denominator inference and automatic quota routing are explicitly deferred.

Also add Accepted Process Registry rows for:

- `Claude/Codex operator stack`
- `Claude direct-read and tool access`

These rows should not weaken existing Claude cross-review rules.

Draft registry rows:

```markdown
| Claude/Codex operator stack | User asks to shift heavy work between Claude/Codex, conserve one model's usage, or use Claude as primary worker | Keep cross-validation mandatory; see `Claude/Codex Operator Stack` above for the canonical role rules. Use Claude Code `claude-opus-4-8` as default heavy operator for now, with Codex retaining goal/integration/final-report responsibility in this workspace. Quota denominator inference remains deferred. | Report operator, validator, model, helper/profile, artifact paths, and verification. Operator artifacts are not greenlights unless a valid review verdict separately approves them. |
| Claude direct-read and tool access | Claude needs direct repo/file, Blender, Unreal, or Niagara visibility | Use `Scripts\Invoke-ClaudeDirectRead.ps1` for whole-repo read-only access; see `Claude/Codex Operator Stack` above for the canonical baseline profile. Use user-scoped Blender MCP after smoke verification. Use Unreal-owned capture/dump scripts for Unreal/Niagara evidence; do not accept desktop screenshots or raw GUI observation as proof. | Direct-read artifact path, post-run clean working-tree check, Blender MCP list/get output when used, and Unreal-owned artifact paths for visual/editor evidence. |
```

### 2. Update `Scripts/Invoke-ClaudePlanReview.ps1`

Keep the existing packet-review helper, but pin the model and add safe preflights:

- Add parameter `-Model`, default `claude-opus-4-8`.
- Add parameter `-Effort`, default `low`.
- Pass `--model $Model --effort $Effort` into the Claude CLI invocation.
- Add a preflight that runs `claude auth status --json`, parses JSON, and fails closed unless output reports:
  - `"loggedIn": true`
  - `"authMethod": "claude.ai"`
  - `"apiProvider": "firstParty"`
- Do not save raw auth output because it contains email/org PII.
- Include `Model`, `Effort`, and `ClaudeVersion` in the helper result object.
- Add a parser-only test hook, similar in spirit to `-ParseReviewPathOnly`, so `Scripts\Test-ClaudeReviewVerdictParser.ps1` can verify valid and invalid auth JSON fixtures without logging out of Claude or saving PII.
- Do not run a separate model probe before every review. If `claude-opus-4-8` is unavailable to the current Claude Code subscription or CLI build, the actual review attempt must fail closed, keep stdout/stderr attempt artifacts, surface the model-resolution error in the thrown failure, and never fall back to another Claude model silently.
- Keep `--permission-mode plan`, `--output-format text`, strict first-line verdict parsing, malformed verdict fail-closed behavior, timeout retries, and existing `ANTHROPIC_API_KEY` guard.

JSON usage accounting is intentionally not being added in this pass because the user deferred denominator/accounting work.

### 3. Add `Scripts/Invoke-ClaudeDirectRead.ps1`

Create a new reusable helper for direct file access by Claude Code.

Baseline behavior:

- Required `-PromptPath`.
- Required `-Mode Review|Operator`. No default, so a caller cannot accidentally run a non-verdict operator task when it meant to request review.
- Default model `claude-opus-4-8`.
- Default effort:
  - `Review` -> `low`
  - `Operator` -> `high`
- Always defaults to `--permission-mode plan`.
- Defaults allowed tools to `Read,Grep,Glob` through `--allowedTools`. Do not also pass `--tools`; `--allowedTools` is the source of truth for this helper's baseline.
- Adds `C:\UE\T66` via `--add-dir`.
- Writes prompt, stdout, stderr, and a small run manifest under `Reports/AgentReviews/ClaudeDirectRead/<timestamp>-<task>`.
- Uses timeout/retry behavior equivalent to `Invoke-ClaudePlanReview.ps1`, with default timeout `180` seconds and `2` attempts. Operator tasks can raise timeout explicitly.
- For `Review` mode, require strict first non-empty line verdict semantics and report greenlight status.
- For `Operator` mode, do not require verdict; the output is an operator artifact/proposal, not a greenlight. The manifest must include `ArtifactKind=OperatorArtifactNotGreenlight`.
- Add optional `-ReviewedOperatorRun <path>` for `Review` mode. When present, the review manifest records the operator run it validates. Operator manifests remain non-greenlights and require a corresponding review artifact before implementation or completion claims can rely on them.
- No `Edit`, `Write`, unrestricted `Bash`, or MCP tools in the baseline.

This helper gives Claude direct reading access now while keeping production edits and broader tool access out of scope until separately reviewed.

### 4. Register Direct-Read Reports Routing

Update:

- `Reports/AGENTS.md`
- `Reports/README.md`

Register `Reports/AgentReviews/ClaudeDirectRead/` as the durable home for direct-read Claude operator/reviewer runs, with the same raw-run retention expectations unless a run contains a durable promoted summary elsewhere.

### 5. Configure Claude Blender MCP

After review approval, add a user-scoped Claude MCP entry named `blender` using the existing official MCP executable:

- Command: `C:\Users\DoPra\.codex\tools\blender_mcp_official\mcp\.venv\Scripts\blender-mcp.exe`
- Environment:
  - `BLENDER_MCP_HOST=localhost`
  - `BLENDER_MCP_PORT=9876`
  - `BLENDER_PATH=C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`

Use user scope, not project scope, because these are machine-specific absolute paths. Verify with `claude mcp list` and `claude mcp get blender`.

No Blender production task will be run in this pass.

This config write is included in this reviewed pass because the user explicitly asked to set up Claude access, but no Blender production action or concurrent Codex/Claude Blender use is in scope. If the MCP add command fails or reveals an existing conflicting `blender` entry, stop and report the conflict instead of overwriting silently.

### 6. Update `Scripts/README.md`

Document:

- `Invoke-ClaudePlanReview.ps1` now defaults to Opus 4.8 for packet review.
- `Invoke-ClaudeDirectRead.ps1` for direct-read operator/reviewer work.
- Claude operator vs reviewer profiles.
- Blender MCP setup/verification.
- Unreal/Niagara rule: Claude should use/review Unreal-owned artifacts from scripts; raw GUI/desktop observation does not replace capture evidence.
- Denominator/quota router remains deferred.

## Out Of Scope

- Quota denominator inference and automatic daily/weekly routing.
- JSON usage accounting.
- Direct Claude file writes.
- Unrestricted shell access from Claude.
- Claude editor automation.
- Claude Unreal Python invocation.
- New Unreal editor bridge implementation.
- New Niagara dump helpers without a specific VFX task.
- Mini/minigame scope.

## Risks And Mitigations

- Risk: New agents treat Claude operator as permission to skip validation.
  - Mitigation: `AGENTS.md` states role split does not weaken cross-validation.
- Risk: Claude Code writes files or runs commands unexpectedly.
  - Mitigation: baseline helper uses `--permission-mode plan` and `--allowedTools Read,Grep,Glob`.
- Risk: Review helper silently uses old model.
  - Mitigation: explicit `--model claude-opus-4-8` and returned model metadata.
- Risk: Claude auth drifts to API billing or logged-out state.
  - Mitigation: existing `ANTHROPIC_API_KEY` guard plus new `auth status` fail-closed preflight.
- Risk: Blender MCP configuration collides with Codex or live Blender state.
  - Mitigation: configure only and verify listing in this pass; production use still requires per-task smoke.
- Risk: Unreal/Niagara "seeing" becomes invalid screenshot proof.
  - Mitigation: AGENTS and README reinforce Unreal-owned capture/dump artifact requirement.

## Verification Plan

- Run `powershell -ExecutionPolicy Bypass -File Scripts\Test-ClaudeReviewVerdictParser.ps1` to ensure strict verdict behavior remains intact.
- Include auth-status parser fixture tests in `Scripts\Test-ClaudeReviewVerdictParser.ps1`: valid first-party subscription JSON passes; logged-out/API/provider-mismatch/malformed JSON fail closed without saving PII.
- Run `Invoke-ClaudePlanReview.ps1` on a tiny approval packet with default params and confirm result includes `Model=claude-opus-4-8`.
- Confirm a bad model name fails closed in a bounded smoke run and does not silently fall back. Do not run this against broad prompts.
- Run `Invoke-ClaudeDirectRead.ps1` in `Operator` mode on a small prompt asking Claude to read `AGENTS.md` and cite one exact line or heading, verifying direct file access without edits.
- Run a narrow tracked-file check after each direct-read smoke to confirm the helper did not modify the working tree. Prefer checking touched process/script/doc paths rather than broad Git/LFS scans.
- Run `Invoke-ClaudeDirectRead.ps1` in `Review` mode on a small packet and confirm strict verdict parsing.
- Run `Invoke-ClaudeDirectRead.ps1` in `Review` mode with `-ReviewedOperatorRun` pointing at the operator smoke run and confirm the review manifest links back to it.
- Run `claude mcp list` and `claude mcp get blender` after adding the user-scoped Blender MCP config.
- Use narrow file checks only; no broad Git/LFS scans.

## Review Request

Review this implementation plan before Codex edits files or config. Focus on whether the AGENTS/process changes are sufficient for new agents, whether the helper defaults are safe, whether the Blender MCP setup is scoped correctly, and whether this implements the user's requested process shift without accidentally weakening review gates or entering deferred quota/denominator work.

Return a strict first line:

`Verdict: APPROVE`

or

`Verdict: REVISE`

or

`Verdict: NEEDS_HUMAN_DECISION`

or

`Verdict: BLOCK`

Then explain the reasoning.
