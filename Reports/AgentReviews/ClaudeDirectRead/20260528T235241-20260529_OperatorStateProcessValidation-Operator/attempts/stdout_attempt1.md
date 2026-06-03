You dismissed the approval prompt, so I'll leave the plan as-is and summarize the operator artifact here. Proposal is saved at `C:\Users\DoPra\.claude\plans\you-are-claude-rippling-fox.md`.

## Operator findings (proposal for Codex — not a greenlight)

**1. Claude did the heavy analysis.** This run read `AGENTS.md`, `Scripts/README.md`, `Reports/AGENTS.md`, all three helper scripts, and the tray's `OperatorStateService.cs` / `AppPaths.cs` / `OperatorState.cs` / runtime JSON, then grepped repo + tray.

**2. Chat-local vs cross-chat (today).** Operator role is conversation-scoped (`AGENTS.md:176`). The AppData `operator-state.json` is a single machine file, but it is **display-only**: the tray reads/writes it; **no agent reads it at task start** (grep: `operator-state` appears only in `AGENTS.md`, `Scripts/README.md`, report artifacts — never in executed code). So Codex writing it for this task did **not** propagate the role to other chats. The switch is effectively chat-local.

**3. Hard constraint.** Claude direct-read operator runs launch with `--add-dir C:\UE\T66` only (`Invoke-ClaudeDirectRead.ps1:41`), so they **cannot read the AppData file**. A cross-agent state file must live **in the repo**.

**4. Mechanism: not a slash/skill.** Codex has no custom slash commands; slashes/skills/hooks are per-tool and invoked, not automatic. The only artifact every T66 chat reads is `AGENTS.md`. So the durable backbone = **AGENTS.md rule + a repo state file**, with a small writer script. Slash/skill is at best a later convenience wrapper.

**5. Smallest durable fix (recommended):**
- New canonical `C:\UE\T66\.t66\operator-state.json` (reuse tray schema).
- New master `Scripts\Set-T66Operator.ps1` that writes the repo file **and** mirrors AppData (keeps tray correct, no C# change; solves "Claude can't Write" asymmetry).
- `AGENTS.md`: replace `:176` "not persisted yet" with read-at-task-start + write-on-switch + 12h staleness fallback rule; refresh registry row `:205`; add the Tier final-line rule.
- `Scripts/README.md:25-32`: update operator-switch section.
- Tier line: every final answer ends with bold `**Tier 0**` / `**Tier 1**`.

**Key risk:** this makes the role *discoverable*, not runtime-injected — it relies on agents honoring the AGENTS read rule; there is no hook forcing it into a fresh chat. That is the only tool-agnostic option.

The three forks I asked about (canonical location, writer script vs docs-only, exact Tier wording) are still open — I defaulted to repo-canonical + mirror, add the script, and `**Tier N**`. Tell me if you want any of those changed, or hand the plan to Codex to implement.

**Tier 1**

