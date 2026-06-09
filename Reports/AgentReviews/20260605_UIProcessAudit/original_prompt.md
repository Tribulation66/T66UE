# Original User Request

Hello I want to work on the UI for my game, we basically changed the art direction to a friendslop sort of game following kind of the visual direction of PEAK, not in terms of theme but in terms of UI elements shapes, and visual language. However this process of using codex to build the UI is not simple and weve tried a lot so first have you and claude go over the materials and information we have for UI and let me know if our instructions and processes and guidelines are unified and simple, or if we have multiple things saying multiple things and its not organized. The pass' objective is for you two to familiarize yourself with our UI processes as well as check if there are any problems before we start.

# Task Contract

Working task: Read-only audit of T66 UI process/material documentation, focused on whether UI instructions are unified, simple, current, and organized enough for upcoming Friendslop/PEAK-like UI direction work.
Operator: Codex
Validator: Claude
Scope: Inspect live repo UI routers, UI instructions, process docs, reference docs, relevant pending issues, and supporting source routing where needed. No code/content/runtime changes. Report problems, contradictions, overlaps, stale directions, missing ownership, and recommended cleanup sequence.
Stop condition: Return a synthesized Codex and Claude assessment with evidence paths, caveats, verification performed/skipped, and token usage.

# Relevant Repo Rules

- Start from live repo state, current folder instructions, current assets, current scripts, and current machine state.
- Root router: C:\UE\T66\AGENTS.md.
- Operator/Validator protocol: C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md.
- UI router: C:\UE\T66\UI\UI_AGENTS.md.
- Reports router: C:\UE\T66\Reports\AGENTS.md.
- Current role state from C:\UE\T66\.t66\operator-state.json: Codex Operator, Claude Validator.
- This is a read-only review/familiarization pass. Do not mutate files.
- Use local Claude Code CLI via Invoke-ClaudePlanReview.ps1 only; no Anthropic API billing.
- The user wants a practical assessment, not a giant inventory dump.

# Suggested Areas To Inspect

- UI\UI_AGENTS.md
- UI\README.md
- UI\Instructions\*.md
- UI\Reference\*.md
- UI\Processes\*.md
- UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md
- UI\Checklists\pending_issues_Checklists.md
- Source\T66\UI\pending_issues_UI.md
- Any obvious current UI source/asset routing docs needed to judge whether instructions are stale or contradictory.

