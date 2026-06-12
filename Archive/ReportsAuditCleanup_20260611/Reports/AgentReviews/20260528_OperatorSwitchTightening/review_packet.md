# Claude/Codex Operator Switch Tightening Review Packet

## Working Goal

Clarify and tighten the T66 Claude/Codex operator-switch process: answer whether Claude has verified Unreal/Niagara editor modification access, then update process docs so user commands like `Make Claude operator` and `Make Codex operator` unambiguously swap operator and validator roles while preserving review gates.

## User Request

- Confirm whether we already proved Claude can enter the Unreal Editor/Niagara editor and inspect or modify those assets directly.
- Make the operator switch simple enough that the user can say `Make Codex operator` or `Make Claude operator` in chat, and the process understands that the validator becomes the other model.

## Current Repo Evidence

- `AGENTS.md` already defines a `Claude/Codex Operator Stack`.
- `AGENTS.md` currently says Claude's baseline direct-review and operator profiles are read-only: `--permission-mode plan --allowedTools Read,Grep,Glob --add-dir C:\UE\T66`.
- `AGENTS.md` currently forbids Claude from using `Edit`, `Write`, unrestricted `Bash`, `bypassPermissions`, direct production asset writes, Unreal Python invocation, or editor automation unless a task-specific reviewed plan names the broader profile and the user approves it.
- `AGENTS.md` currently says Claude Unreal/Niagara visibility starts from direct file reads and existing Unreal-owned capture/dump artifacts, and that desktop screenshots/raw GUI observation/private viewport history are not acceptance proof.
- `Scripts/README.md` says both Claude direct-read profiles default to read-only plan mode and do not allow editor automation or Unreal Python unless a separate reviewed profile widens access.
- `Reports/AgentReviews/ClaudeDirectRead/20260528T203657-StackSetupSmokeFinal-Operator` verifies Claude direct-read operator mode for `AGENTS.md`; it is explicitly `OperatorArtifactNotGreenlight`.
- `Reports/AgentReviews/ClaudeDirectRead/20260528T203713-StackSetupSmokeFinal-Review` approves that direct-read smoke and notes the prompt forbids edits, shell, Unreal Python, editor automation, and asset writes.
- `claude mcp list` and `claude mcp get blender` currently show a user-scoped `blender` MCP entry connected.
- I found no evidence that Claude has been granted or smoke-tested live Unreal Editor, Niagara editor, Unreal Python, or direct asset-modification access.

## Codex Conclusion Before Review

The correct answer to the first user question is: no, we have not confirmed Claude can directly enter or modify Unreal/Niagara editor state. We have confirmed Claude Code direct repo read access and current Blender MCP connectivity. For Unreal/Niagara, the approved path remains direct file inspection plus Unreal-owned capture/dump evidence unless a future task-specific reviewed plan and explicit user approval widens Claude's tool profile.

## Proposed Documentation Changes

Edit `AGENTS.md` in `### Claude/Codex Operator Stack`:

- Add exact chat command handling:
  - `Make Claude operator` sets Claude as operator and Codex as validator/integrator.
  - `Make Codex operator` sets Codex as operator and Claude as validator/reviewer.
  - Unless the user explicitly names a different validator, the validator is always the other model.
- Require the active agent to restate the active roles in one short procedural sentence after an operator-switch command, then proceed without asking for extra confirmation just to apply the switch.
- State that switching operators changes role routing only. It does not widen tool permissions, bypass review gates, authorize editor automation, authorize direct writes, or change the current Unreal/Niagara evidence rules.
- State that if the intended validator is unavailable, agents must follow existing fallback-review rules and report the degraded role pairing before proceeding.
- Update the registry row trigger for `Claude/Codex operator stack` to include the exact `Make Claude operator` and `Make Codex operator` phrases.

Edit `Scripts/README.md` under `## Claude/Codex Stack Helpers`:

- Add an `Operator switch commands` subsection with the same command mapping.
- State again that the switch is role routing only and does not enable Claude editor automation, file writes, Unreal Python, or shell/MCP access beyond the reviewed profile.

## Scope

In scope:

- Root process doc `AGENTS.md`.
- Script helper doc `Scripts/README.md`.
- Final answer explaining what was and was not confirmed.

Out of scope:

- No Unreal, Niagara, Blender, or MCP production task.
- No Claude editor automation.
- No new tool profile for Claude writes, shell, Unreal Python, or editor control.
- No usage-denominator or quota router implementation.
- No Mini/minigame work.

## Risks And Controls

- Risk: The new command phrases could be read as authorization to give Claude broader access.
  - Control: Explicitly say operator switching does not widen permissions or bypass review gates.
- Risk: The role switch could leave both models thinking they are validator or operator.
  - Control: Make the inverse validator rule mandatory and require a short role restatement after switching.
- Risk: A future agent could treat Claude Blender MCP verification as Unreal/Niagara editor verification.
  - Control: Final answer and docs distinguish direct repo read and Blender MCP from unconfirmed Unreal/Niagara editor access.

## Verification Plan

After approval and edits:

- Run `git diff --check -- AGENTS.md Scripts/README.md`.
- Run narrow `Select-String` checks for `Make Claude operator`, `Make Codex operator`, and role-routing language in both docs.
- Report the Claude review artifact path and verification commands in the final answer.

## Review Request

Review this answer-and-doc-edit plan before Codex edits the process docs or gives the final substantive answer. Focus on whether the answer is accurate from the evidence, whether the proposed role-switch language avoids widening access accidentally, and whether the verification plan is sufficient for a process-doc-only change.

Required first line: `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.
