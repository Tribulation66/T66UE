You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ClaudeStackSetup\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
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
- `claude auth status` reports first-party `claude.ai` subscription login; output contains PII and must not be copied into docs or artifacts except sanitized summaries.

## Prior Approved Planning Evidence

- `Reports/AgentReviews/20260528_ClaudeCodexStackPlan/20260528T200833-pass2/claude_review_pass2.md` approved the no-change strategy.
- `Reports/AgentReviews/20260528_ClaudeCodexStackPlan/claude_opus48_probe.json` showed explicit local `--model claude-opus-4-8` use in `modelUsage`.
- Anthropic official model docs list API ID `claude-opus-4-8`.

## Proposed Implementation

### 1. Update `AGENTS.md`

Add a new subsection after `Claude Cross-Review`, tentatively named `Claude/Codex Operator Stack`.

The subsection will state:

- Claude Opus 4.8 is the default heavy operator when the user asks to conserve Codex usage, asks for Claude-led work, or the task benefits from independent direct repo inspection.
- Codex remains responsible for goal management, repo integration, user-facing reporting, and final verification in this active Codex workspace unless the user explicitly changes that.
- The non-operator model must still validate/critique. The stack change moves heavy lifting, not the review invariant.
- Claude operator baseline is read-only/direct-read first: `--model claude-opus-4-8 --effort high --permission-mode plan --tools Read,Grep,Glob --allowedTools Read,Grep,Glob --add-dir C:\UE\T66`.
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

### 2. Update `Scripts/Invoke-ClaudePlanReview.ps1`

Keep the existing packet-review helper, but pin the model and add safe preflights:

- Add parameter `-Model`, default `claude-opus-4-8`.
- Add parameter `-Effort`, default `low`.
- Pass `--model $Model --effort $Effort` into the Claude CLI invocation.
- Add a preflight that runs `claude auth status` and fails closed unless output reports:
  - `"loggedIn": true`
  - `"authMethod": "claude.ai"`
  - `"apiProvider": "firstParty"`
- Do not save raw auth output because it contains email/org PII.
- Include `Model`, `Effort`, and `ClaudeVersion` in the helper result object.
- Keep `--permission-mode plan`, `--output-format text`, strict first-line verdict parsing, malformed verdict fail-closed behavior, timeout retries, and existing `ANTHROPIC_API_KEY` guard.

JSON usage accounting is intentionally not being added in this pass because the user deferred denominator/accounting work.

### 3. Add `Scripts/Invoke-ClaudeDirectRead.ps1`

Create a new reusable helper for direct file access by Claude Code.

Baseline behavior:

- Required `-PromptPath`.
- Optional `-Mode Review|Operator`; default `Operator`.
- Default model `claude-opus-4-8`.
- Default effort:
  - `Review` -> `low`
  - `Operator` -> `high`
- Always defaults to `--permission-mode plan`.
- Defaults tools to `Read,Grep,Glob`.
- Adds `C:\UE\T66` via `--add-dir`.
- Writes prompt, stdout, stderr, and a small run manifest under `Reports/AgentReviews/ClaudeDirectRead/<timestamp>-<task>`.
- For `Review` mode, require strict first non-empty line verdict semantics and report greenlight status.
- For `Operator` mode, do not require verdict; the output is an operator artifact/proposal, not a greenlight.
- No `Edit`, `Write`, unrestricted `Bash`, or MCP tools in the baseline.

This helper gives Claude direct reading access now while keeping production edits and broader tool access out of scope until separately reviewed.

### 4. Configure Claude Blender MCP

After review approval, add a user-scoped Claude MCP entry named `blender` using the existing official MCP executable:

- Command: `C:\Users\DoPra\.codex\tools\blender_mcp_official\mcp\.venv\Scripts\blender-mcp.exe`
- Environment:
  - `BLENDER_MCP_HOST=localhost`
  - `BLENDER_MCP_PORT=9876`
  - `BLENDER_PATH=C:\Program Files\Blender Foundation\Blender 5.1\blender.exe`

Use user scope, not project scope, because these are machine-specific absolute paths. Verify with `claude mcp list` and `claude mcp get blender`.

No Blender production task will be run in this pass.

### 5. Update `Scripts/README.md`

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
- New Unreal editor bridge implementation.
- New Niagara dump helpers without a specific VFX task.
- Mini/minigame scope.

## Risks And Mitigations

- Risk: New agents treat Claude operator as permission to skip validation.
  - Mitigation: `AGENTS.md` states role split does not weaken cross-validation.
- Risk: Claude Code writes files or runs commands unexpectedly.
  - Mitigation: baseline helper uses `--permission-mode plan`, `--tools Read,Grep,Glob`, `--allowedTools Read,Grep,Glob`.
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
- Run `Invoke-ClaudePlanReview.ps1` on a tiny approval packet with default params and confirm result includes `Model=claude-opus-4-8`.
- Run `Invoke-ClaudeDirectRead.ps1` in `Operator` mode on a small prompt asking Claude to read `AGENTS.md` and cite one exact line or heading, verifying direct file access without edits.
- Run `Invoke-ClaudeDirectRead.ps1` in `Review` mode on a small packet and confirm strict verdict parsing.
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

</review_packet>
