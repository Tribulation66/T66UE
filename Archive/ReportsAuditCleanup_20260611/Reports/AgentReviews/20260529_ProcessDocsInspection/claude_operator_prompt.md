# Claude Operator Prompt: T66 Process Docs Inspection

Working task:
Inspect the updated T66 agent/operator-validator process files and summarize process changes that affect future agent work.

Operator:
Claude

Validator:
Codex

Scope:
Read-only inspection and summary. No code, doc, config, asset, Git, build, staging, or workflow edits.

Stop condition:
Produce a concise process-change summary with exact file/line anchors and any practical implications for future work.

Relevant files to inspect:
- C:\UE\T66\AGENTS.md
- C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md
- C:\UE\T66\.t66\operator-state.json
- C:\UE\T66\Scripts\Set-T66Operator.ps1
- C:\UE\T66\Reports\AGENTS.md

Instructions:
- Start from live repo files only.
- Do not edit files.
- Do not run mutating commands.
- Do not inspect Mini/minigame paths.
- Identify the main changed process rules compared with the older Claude-review/manual-goal workflow.
- Include exact path:line anchors for each load-bearing claim.
- Include current Operator/Validator state.
- Include any caveats or places where agents could easily trip over the new process.
- Include token usage if the helper exposes it; otherwise say unavailable.
