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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Claude Unreal/Niagara Smoke Review Packet

## Working Goal

Confirm whether Claude Code can directly access Unreal Editor/Niagara for T66 by designing and running a safe smoke test, without granting production writes unless a reviewed plan and user approval allow it.

## User Request

Pablo said: "Ok lets go ahead and confirm the editor/niagra/unreal question now."

Interpretation: run a bounded capability confirmation now. The target is not a production VFX change and not permission to give Claude unrestricted shell/editor access.

Safe assumptions:

- No Mini/minigame scope.
- No production asset modifications.
- No direct `Content/` writes or package saves.
- No desktop screenshots as proof.
- A successful smoke should distinguish what is confirmed from what remains unconfirmed.

## Current Repo Instructions And Evidence

- `AGENTS.md` lines 157-173 define the Claude/Codex operator stack.
- `AGENTS.md` line 169 forbids Claude `Edit`, `Write`, unrestricted `Bash`, `bypassPermissions`, direct production asset writes, Unreal Python invocation, or editor automation unless a task-specific reviewed plan names the exact tool profile and the user has approved broader access.
- `AGENTS.md` line 171 says Claude Unreal/Niagara visibility starts from direct file reads and existing Unreal-owned capture/dump artifacts.
- `Scripts/README.md` lines 51-55 repeats that the baseline Claude profiles are read-only and that Unreal/Niagara proof should use Unreal-owned artifacts.
- `Scripts/pending_issues_Scripts.md` lines 10-15 documents that some headless Interchange imports can crash after saving, so this smoke must avoid imports and package saves.
- `Reports/AGENTS.md` routes review/proof artifacts under `Reports/AgentReviews`.
- `Scripts/CaptureT66NiagaraEditorIsolation.ps1` and `Scripts/CaptureT66NiagaraMRQIsolation.ps1` show existing Unreal-owned Niagara proof seams, but full visual capture is heavier than needed for this capability smoke.
- `claude --help` shows `--allowedTools` supports command-pattern restrictions such as `Bash(git *)` and permission modes include `dontAsk`, `plan`, and `bypassPermissions`.

## Proposed Capability Smoke

Create two temporary report-owned smoke files under:

`C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke\`

1. `ClaudeUnrealNiagaraSmoke.py`
   - Runs inside Unreal Python.
   - Uses `unreal.EditorAssetLibrary` and `unreal.AssetRegistryHelpers`.
   - Verifies Unreal Python is live.
   - Records engine version.
   - Checks whether `/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash` exists.
   - Loads that asset if present and records its loaded class.
   - Lists Niagara-related Python API symbols.
   - Lists Niagara-like asset registry rows under `/Game/VFXLab`.
   - Writes `unreal_niagara_smoke_report.json` under the same report folder.
   - Does not call save APIs, create assets, import assets, or mutate packages.

2. `RunClaudeUnrealNiagaraSmoke.ps1`
   - Runs `C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe`.
   - Arguments: `C:\UE\T66\T66.uproject -run=pythonscript -script=<report py> -unattended -nop4 -nosplash`.
   - Captures stdout/stderr/logs under the report folder.
   - Enforces a timeout.
   - Fails if the JSON report is missing or does not show `success=true`, `unreal_python_live=true`, and a Niagara class/API signal.

Then run Claude Code directly, not through the baseline direct-read helper, because this is the specific broader profile being tested:

```powershell
$prompt = Get-Content -Raw C:\UE\T66\Reports\AgentReviews\20260528_ClaudeUnrealNiagaraSmoke\claude_smoke_prompt.md
$prompt | claude -p --no-session-persistence --permission-mode dontAsk --max-turns 6 --model claude-opus-4-8 --effort low --output-format text --allowedTools "Read,Bash(powershell -NoProfile -ExecutionPolicy Bypass -File C:/UE/T66/Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke/RunClaudeUnrealNiagaraSmoke.ps1*)" --add-dir C:\UE\T66
```

Prompt Claude to run exactly:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File C:/UE/T66/Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke/RunClaudeUnrealNiagaraSmoke.ps1
```

The only broadened Claude tool is `Bash(...)`, restricted to this one wrapper command prefix. Do not allow `Edit`, `Write`, unrestricted `Bash`, `bypassPermissions`, or production asset paths.

## What Success Means

If the Claude-run command succeeds and the JSON report shows Unreal Python executed and Niagara asset/API signals were inspected from inside Unreal, we can confirm:

- Claude Code can be given a bounded profile that invokes Unreal editor commandlet automation.
- Claude can inspect Unreal/Niagara state through Unreal Python and report artifacts.
- This is stronger than second-hand Codex packets for Unreal/Niagara inspection.

It still does not confirm:

- Claude can visually operate the GUI Niagara editor.
- Claude can safely modify production Niagara assets.
- Claude has unrestricted editor access.
- Claude can replace Unreal-owned capture/video proof for visual acceptance.

Those would need separate reviewed profiles and smoke tests.

## Risks And Controls

- Risk: Claude gets broad shell access.
  - Control: one `Bash(...)` command pattern only; no `bypassPermissions`; no `Edit`/`Write`.
- Risk: the smoke mutates production assets.
  - Control: Unreal Python script is read-only and writes only JSON report/logs under `Reports/AgentReviews`.
- Risk: success is overstated.
  - Control: final answer must explicitly classify confirmed headless/editor-commandlet access versus unconfirmed GUI/editor modification.
- Risk: Unreal commandlet hangs.
  - Control: wrapper timeout and kill.

## Verification Plan

Before Claude run:

- Verify `ANTHROPIC_API_KEY` is unset in Process/User/Machine.
- Run `git diff --check -- Reports/AgentReviews/20260528_ClaudeUnrealNiagaraSmoke`.

After Claude run:

- Inspect Claude stdout/stderr artifacts.
- Inspect `unreal_niagara_smoke_report.json`.
- Confirm no production asset writes were requested by checking only the report folder outputs and the narrow smoke artifacts.
- If the command fails due to allowed-tool syntax or Claude refusal, report that Claude editor/Niagara automation is not confirmed through this profile.

## Review Request

Review whether this is a safe and sufficient capability confirmation plan under `AGENTS.md`, especially whether the exact broadened Claude profile is narrow enough and whether the success/failure language avoids overstating GUI Niagara/editor modification access.

Required first line: `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.

</review_packet>
