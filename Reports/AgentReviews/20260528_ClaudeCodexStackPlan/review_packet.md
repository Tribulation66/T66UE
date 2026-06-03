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
- The Anthropic news page fetch returned the page titled `Introducing Claude Opus 4.8`, dated May 28, 2026. It says Anthropic is upgrading Claude Opus to `Claude Opus 4.8`, that it is available today, and that developers can use `claude-opus-4-8` via the Claude API.
- The Anthropic model overview fetch returned `Models overview`. Its latest-models table lists `Claude Opus 4.8`, Claude API ID `claude-opus-4-8`, and says Opus 4.8 is the recommended starting point for the most complex tasks.
- Explicit local probe using `--model claude-opus-4-8 --output-format json` succeeded and wrote `Reports/AgentReviews/20260528_ClaudeCodexStackPlan/claude_opus48_probe.json`.
- That probe's `modelUsage` names include `claude-opus-4-8`, so the local Claude Code CLI accepts and uses that model when pinned explicitly.
- A prior no-model JSON probe showed `claude-opus-4-7[1m]` in `modelUsage`, so the current unpinned helper should not be assumed to use Opus 4.8.
- Local Claude Code JSON for the explicit Opus 4.8 probe includes per-run input/cache/output token fields and model-level context/max-output fields, but not account quota denominator or remaining subscription usage.
- Existing T66 Niagara process already has reproducible Unreal-owned capture routes, including `Scripts/CaptureT66NiagaraMRQIsolation.ps1`; process docs reject desktop screenshots as final visual proof.

## Codex Proposed Strategy

### Phase 1: Pin Claude Review Model And Capture Usage JSON

Update `Scripts/Invoke-ClaudePlanReview.ps1` later, not now, to accept model and effort parameters and default normal reviews to `--model claude-opus-4-8`. Parser coverage must be updated and pass before JSON output is enabled in the live helper. Prefer JSON output for accounting after that, but keep strict first-non-empty-line verdict enforcement against extracted reviewer text.

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

Add a separate helper, or a clearly separate mode, for Claude direct repo review. The first reviewed profile should be read-only:

- `--permission-mode plan`
- `--add-dir C:\UE\T66`
- `--allowedTools Read,Grep,Glob`
- no `Bash`, `Edit`, `Write`, notebook, network, browser, or MCP tool access in the baseline direct-read reviewer

Any broader profile, including allowlisted shell commands or MCP access, should be a separate named profile with its own review.

This should not replace packet review entirely. Packet review remains useful when the question is "is Codex's exact packet sound?" Direct-read review is for questions where Claude should independently inspect the repo and cite files.

### Phase 3: Add Claude Operator Mode

Add a separate operator/helper profile for Claude heavy lifting. Start with no direct file edits: Claude may inspect files and produce a plan, patch proposal, command plan, or reviewer notes; Codex applies/integrates only after validation. Later, direct edits can be considered behind explicit user approval and a narrow allowlist.

Profiles:

- Claude operator: high or max effort, direct repo read, allowlisted command/capture tools where needed, no unbounded destructive commands.
- Claude validator: low/medium effort, direct repo read or packet-only, no writes.
- Codex operator: current interactive agent or separate Codex CLI worker when appropriate.
- Codex validator: separate local Codex CLI helper, not the active Codex reasoning pass.

### Phase 4: Give Claude Blender Access Via MCP

Reuse the existing official Blender MCP installation rather than inventing a second stack. Configure Claude Code to use the same MCP server path and env. Start with a local/user-scoped Claude MCP entry because the current paths are user-machine absolute; promote to project scope only if the repo gets a portable setup wrapper or environment-variable-based config.

Validation later:

- `claude mcp list` shows Blender.
- A read-only Claude task can summarize the active Blender scene or blend file through the MCP.
- Any production Blender workflow still follows the existing Blender Lab MCP instructions.

### Phase 5: Give Claude Unreal/Niagara Visibility Through Reproducible Bridges

Do not start by giving Claude raw GUI control as the acceptance path. The T66 process requires Unreal-owned capture/proof. First expose existing allowlisted Unreal scripts and commandlets that open assets, dump metadata, and produce reproducible PNG/MP4/JSON artifacts Claude can inspect directly.

For Niagara specifically:

- Let Claude read the VFX process docs and target assets.
- Let Claude invoke or request `CaptureT66NiagaraMRQIsolation.ps1`.
- Defer any new Niagara dump helper until a specific VFX task names a target Niagara system and required evidence. Do not build speculative editor tooling without a target.
- Require route/view/zoom/background/system/timing/output evidence before using visual conclusions.
- If true editor-entry is needed, create a documented Unreal editor bridge that can open a target asset, dump graph/renderers/parameters, and trigger Unreal-owned captures.

### Phase 6: Add Usage Ledger And Operator Router

Build a lightweight usage ledger and router. Inputs are per-run token JSON plus user-visible before/after percentages and reset dates. Outputs are routing recommendations: which model should be operator and which should be validator.

Durable ledger contract for later implementation:

- Location candidate: `Reports/AgentReviews/AgentStackUsageLedger/usage_ledger.jsonl`
- Each helper writes one row after a Claude or Codex worker/reviewer run.
- Router reads the latest rows and an explicit user/account snapshot.
- Account snapshot can be passed by CLI flags on the later router/helper:
  - `-ClaudeRemainingPercent`
  - `-ClaudeResetAt`
  - `-CodexRemainingPercent`
  - `-CodexResetAt`
  - optional `-ClaudeRemainingPercentAfter` and `-CodexRemainingPercentAfter` for calibration
- Ledger row fields should include timestamp, agent, model, role, effort, helper name, artifact paths, input tokens, cache creation tokens, cache read tokens, output tokens, reported cost if present, visible remaining percent before/after when provided, reset date, and notes.

The user's denominator inference is mathematically plausible only if the visible percentage maps linearly to the same token bucket with enough precision. In practice, subscription usage may be rounded, model-weighted, cache-weighted, message/session-window based, rolling-window based, or affected by concurrent sessions. So the inferred denominator should be treated as an empirical routing slope, not a contractual weekly token limit.

Routing rule:

- Preserve cross-validation always.
- Estimate expected token cost by role before routing: direct-read validator can be expensive, while packet-only validator is usually cheaper.
- Assign heavy planning/implementation/operator work to the model with more surplus against its daily burn target after expected role cost.
- Assign validator/critic work to the lower-budget model only when the validator profile is expected to fit that model's remaining daily allowance; otherwise use a cheaper packet-only validator profile or ask the user to approve spending.
- Never use the active Codex pass as its own independent validator.

With the user's example, Codex at 19 percent remaining until May 30 and Claude at 100 percent remaining means Claude should be heavy operator and Codex should be validator/integrator until the Codex reset.

### Phase 7: Durable Process Updates Later

Candidate files to change after approval. Each candidate should still be subject to its own focused plan/review before edit:

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
- Treat Claude subscription/auth status as drift-prone; recheck before using Claude worker/reviewer helpers.
- Treat Claude Code version and CLI flag behavior as drift-prone; record `claude --version` in smoke artifacts.

## Verification Needed Before Later Implementation Is Complete

- Claude review smoke with explicit `modelUsage` showing `claude-opus-4-8`.
- Parser tests pass for strict verdicts from both text and JSON-extracted output.
- Direct-read Claude smoke uses the reviewed baseline profile `--permission-mode plan --allowedTools Read,Grep,Glob --add-dir C:\UE\T66` and cites live repo files without edits.
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
