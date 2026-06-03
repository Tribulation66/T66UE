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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ClaudeCodexStackPlan\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Claude/Codex Stack Planning Review Packet

## Working Goal

Produce a no-change implementation strategy for evolving T66's Claude/Codex workflow: verify the claimed Opus 4.8 target, plan direct-read/tool-enabled Claude Code usage, design a quota-aware operator/validator stack, and identify repo/process files that would need changes without making them yet.

## User Constraints

- Planning only. Do not implement process/script/doc changes yet.
- Start from live repo and machine state, not stale assumptions.
- Preserve the core cross-validation loop: one model can be heavy operator, but the other still validates/criticizes.
- Avoid Anthropic API billing; use local Claude Code subscription CLI.
- Default scope excludes Mini/minigame systems.

## Repo Instructions Consulted

- `AGENTS.md`
- `Reports/AGENTS.md`
- `Reports/README.md`
- `Scripts/README.md`
- `Scripts/Invoke-ClaudePlanReview.ps1`
- `Scripts/Invoke-CodexPlanReview.ps1`
- `Model Generation/Tools/BlenderLabMCP/BLENDER_LAB_MCP_INSTRUCTIONS.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/pending_issues_Combat.md`

## Live Evidence

- `ANTHROPIC_API_KEY`, `CLAUDE_CODE_USE_BEDROCK`, and `CLAUDE_CODE_USE_VERTEX` are unset in Process/User/Machine scopes.
- Local Claude command resolves to `C:\Users\DoPra\.local\bin\claude.exe`.
- `claude --version` reports `2.1.150 (Claude Code)`.
- `claude auth status` reports a first-party `claude.ai` login with Max subscription; no API provider was active.
- `claude --help` exposes `--model`, `--effort`, `--tools`, `--add-dir`, `--output-format`, `--permission-mode`, `--mcp-config`, plugin controls, and allowed/disallowed tool controls.
- `claude mcp list` currently has Google app connectors only; no Blender MCP server is configured for Claude.
- Codex already has a Blender MCP server in `C:\Users\DoPra\.codex\config.toml` pointing to `C:\Users\DoPra\.codex\tools\blender_mcp_official\mcp\.venv\Scripts\blender-mcp.exe`.
- Official Anthropic sources checked:
  - `https://www.anthropic.com/news/claude-opus-4-8`
  - `https://docs.anthropic.com/en/docs/about-claude/models/overview`
- Explicit local probe using `--model claude-opus-4-8 --output-format json` succeeded and wrote `Reports/AgentReviews/20260528_ClaudeCodexStackPlan/claude_opus48_probe.json`.
- That probe's `modelUsage` names include `claude-opus-4-8`, so the local Claude Code CLI accepts and uses that model when pinned explicitly.
- A prior no-model JSON probe showed `claude-opus-4-7[1m]` in `modelUsage`, so the current unpinned helper should not be assumed to use Opus 4.8.
- Local Claude Code JSON for the explicit Opus 4.8 probe includes per-run input/cache/output token fields and model-level context/max-output fields, but not account quota denominator or remaining subscription usage.
- Existing T66 Niagara process already has reproducible Unreal-owned capture routes, including `Scripts/CaptureT66NiagaraMRQIsolation.ps1`; process docs reject desktop screenshots as final visual proof.

## Codex Proposed Strategy

### Phase 1: Pin Claude Review Model And Capture Usage JSON

Update `Scripts/Invoke-ClaudePlanReview.ps1` later, not now, to accept model and effort parameters and default normal reviews to `--model claude-opus-4-8`. Prefer JSON output for accounting, but keep strict first-non-empty-line verdict enforcement against extracted reviewer text.

Expected script changes later:

- Add `-Model`, default `claude-opus-4-8`.
- Add `-Effort`, with conservative default for validator work.
- Add optional JSON raw artifact output:
  - raw Claude JSON
  - extracted review markdown/text
  - normalized usage summary JSON/markdown
- Update verdict parser tests so JSON extraction cannot create false greenlights.
- Verify with a smoke probe whose `modelUsage` includes `claude-opus-4-8`.

### Phase 2: Add Direct-Read Claude Review Mode

Add a separate helper, or a clearly separate mode, for Claude direct repo review. It should use Claude Code with read-only permissions and limited tools first: read/search/glob plus `--add-dir C:\UE\T66`.

This should not replace packet review entirely. Packet review remains useful when the question is "is Codex's exact packet sound?" Direct-read review is for questions where Claude should independently inspect the repo and cite files.

### Phase 3: Add Claude Operator Mode

Add a separate operator/helper profile for Claude heavy lifting. Start with no direct file edits: Claude may inspect files and produce a plan, patch proposal, command plan, or reviewer notes; Codex applies/integrates only after validation. Later, direct edits can be considered behind explicit user approval and a narrow allowlist.

Profiles:

- Claude operator: high or max effort, direct repo read, allowlisted command/capture tools where needed, no unbounded destructive commands.
- Claude validator: low/medium effort, direct repo read or packet-only, no writes.
- Codex operator: current interactive agent or separate Codex CLI worker when appropriate.
- Codex validator: separate local Codex CLI helper, not the active Codex reasoning pass.

### Phase 4: Give Claude Blender Access Via MCP

Reuse the existing official Blender MCP installation rather than inventing a second stack. Configure Claude Code to use the same MCP server path and env, probably through a local or project-scoped Claude MCP entry.

Validation later:

- `claude mcp list` shows Blender.
- A read-only Claude task can summarize the active Blender scene or blend file through the MCP.
- Any production Blender workflow still follows the existing Blender Lab MCP instructions.

### Phase 5: Give Claude Unreal/Niagara Visibility Through Reproducible Bridges

Do not start by giving Claude raw GUI control as the acceptance path. The T66 process requires Unreal-owned capture/proof. First expose existing allowlisted Unreal scripts and commandlets that open assets, dump metadata, and produce reproducible PNG/MP4/JSON artifacts Claude can inspect directly.

For Niagara specifically:

- Let Claude read the VFX process docs and target assets.
- Let Claude invoke or request `CaptureT66NiagaraMRQIsolation.ps1` or a future narrow Niagara dump helper.
- Require route/view/zoom/background/system/timing/output evidence before using visual conclusions.
- If true editor-entry is needed, create a documented Unreal editor bridge that can open a target asset, dump graph/renderers/parameters, and trigger Unreal-owned captures.

### Phase 6: Add Usage Ledger And Operator Router

Build a lightweight usage ledger and router. Inputs are per-run token JSON plus user-visible before/after percentages and reset dates. Outputs are routing recommendations: which model should be operator and which should be validator.

The user's denominator inference is mathematically plausible only if the visible percentage maps linearly to the same token bucket with enough precision. In practice, subscription usage may be rounded, model-weighted, cache-weighted, message/session-window based, rolling-window based, or affected by concurrent sessions. So the inferred denominator should be treated as an empirical routing slope, not a contractual weekly token limit.

Routing rule:

- Preserve cross-validation always.
- Assign heavy planning/implementation/operator work to the model with more surplus against its daily burn target.
- Assign validator/critic work to the model with lower remaining budget.
- Never use the active Codex pass as its own independent validator.

With the user's example, Codex at 19 percent remaining until May 30 and Claude at 100 percent remaining means Claude should be heavy operator and Codex should be validator/integrator until the Codex reset.

### Phase 7: Durable Process Updates Later

Likely files to change after approval:

- `AGENTS.md`: add agent-stack routing, direct-read Claude review mode, role split, and usage-ledger policy.
- `Scripts/Invoke-ClaudePlanReview.ps1`: model pin, effort, JSON usage artifact handling.
- `Scripts/Test-ClaudeReviewVerdictParser.ps1`: JSON/extracted-review parser coverage.
- New `Scripts/Invoke-ClaudeRepoReview.ps1`: direct-read read-only reviewer.
- New `Scripts/Invoke-ClaudeWork.ps1` or `Scripts/Invoke-AgentWorker.ps1`: operator profile.
- New `Scripts/Invoke-AgentStack.ps1`: routing wrapper using ledger plus user-supplied remaining percentages.
- `Scripts/Invoke-CodexPlanReview.ps1`: optional role labels and usage-ledger integration.
- `Scripts/README.md`: document the helper roles.
- `Reports/README.md`: document usage ledger/report routing if durable report folders are added.
- Claude MCP config or setup doc/script: add Blender MCP using the existing official server path.
- Optional Unreal/Niagara helper scripts: read-only asset dump/capture wrappers for Claude review.

## Risks And Guardrails

- Do not assume unpinned Claude reviews are Opus 4.8.
- Do not let JSON output weaken strict verdict parsing.
- Do not expose credentials or user-private Claude auth details in artifacts.
- Do not let direct-read Claude turn into unreviewed direct-write Claude without explicit approval.
- Do not treat desktop screenshots or raw GUI viewing as valid Unreal/Niagara proof when T66 requires Unreal-owned capture artifacts.
- Do not treat inferred subscription denominators as exact limits.
- Do not let usage routing remove the validator role from either model.

## Verification Needed Before Later Implementation Is Complete

- Claude review smoke with explicit `modelUsage` showing `claude-opus-4-8`.
- Parser tests pass for strict verdicts from both text and JSON-extracted output.
- Direct-read Claude smoke cites live repo files without edits.
- Blender MCP smoke from Claude can perform a read-only scene/file summary.
- Unreal/Niagara read-only smoke produces reproducible capture or dump artifacts through T66 scripts.
- Usage ledger records at least one Claude run and one Codex run with role labels and user-supplied before/after percentages.
- Agent router recommendation matches the known example: Claude operator, Codex validator/integrator while Codex has 19 percent remaining and Claude has 100 percent remaining.

## Review Request

Review the strategy above as a read-only reviewer. Focus on flawed assumptions, missing repo/process files, unsafe scope, inadequate verification, contradictions with T66 instructions, and whether this is a sound no-change answer for the user.

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
