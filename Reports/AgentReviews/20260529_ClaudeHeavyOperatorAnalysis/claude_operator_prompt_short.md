# Claude Heavy Operator Analysis Prompt - Short

You are Claude Code acting as the T66 heavy Operator for a Tier 1 process analysis. Produce a concise operator artifact for Codex to validate. Do not edit files.

Goal:
Analyze token/workload split in the Codex/Claude stack and recommend how Claude can become the true 70-80% heavy Operator while Codex remains Validator/Integrator.

Current confirmed facts:
- `.t66\operator-state.json` is valid: `Operator=Claude`, `Validator=Codex`.
- `AGENTS.md` defines Operator as the model doing heavier planning, investigation, proposal generation, implementation design, or approved write/editor work.
- `AGENTS.md` says Codex remains responsible for working goal, repo integration, applying edits, final verification, and user-facing completion report unless explicitly changed.
- Baseline Claude Operator profile is read-only: `claude-opus-4-8`, high effort, plan mode, `Read,Grep,Glob`, whole-repo read access.
- `AGENTS.md` forbids Claude `Edit`, `Write`, unrestricted `Bash`, `bypassPermissions`, direct production asset writes, Unreal Python, or editor automation unless a task-specific reviewed plan names the exact tool profile and the user approves broader access.
- Automatic quota denominator inference and automatic usage routing are deferred.
- `Scripts\Invoke-ClaudeDirectRead.ps1` supports `-AllowedTools`, `-PermissionMode`, and `-AllowBroaderTools`, but the baseline is intentionally read-only.

Answer these directly:
1. What categories consume the most tokens in this workflow?
2. Is the user's 80/20 Operator/Validator assumption fundamentally right?
3. Can read-only Claude Operator runs realistically create a 70-80% workload shift?
4. What smallest safe change makes Claude the true heavy Operator?
5. Should Claude directly edit files? If yes, under what bounded modes/gates?
6. What should Codex validate after Claude does heavy work?
7. What metrics prove the split without quota denominator inference?

Output:
- Operator: Claude
- Validator: Codex
- Recommended target split
- Bottom line
- Findings
- Proposed operating modes
- Required changes
- Risks
- Measurement plan
- Codex validation checklist

Keep it under 1200 words. Do not browse or inspect broadly; use at most `AGENTS.md` and `Scripts\Invoke-ClaudeDirectRead.ps1` if needed.
