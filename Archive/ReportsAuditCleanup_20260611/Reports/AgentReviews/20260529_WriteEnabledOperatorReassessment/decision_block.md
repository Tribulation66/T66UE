# Decision Block: Claude Operator Permissions

Status: RESOLVED BY USER DECISION

The current Operator/Validator protocol cannot reliably put 70-80% of implementation token work on Claude because Claude is read-only and Codex applies the edits.

## User Decision

The user rejected a text-only Claude Operator profile and chose a broader rule:

Claude should have the same practical work surface as Codex when Claude is Operator, including file edits, shell commands, Unreal, Blender, Niagara, and comparable editor/tooling access, but Claude must first request permission from Codex to make changes. Codex approves or rejects the requested scope. Claude `plan` permission mode should not be used for Operator work.

## Implemented Direction

`Scripts\Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator` now requires a Codex approval artifact whose first non-empty line is exactly `Codex Approval: APPROVE`. Without that artifact, full Operator mode refuses to run before launching Claude.

## Approval Boundary

Codex approval controls scope, not tool availability. If Claude needs to go outside the approved scope, perform destructive Git operations, touch credentials/billing, do broad Git/LFS scans, or contradict repo process rules, Claude must stop and request additional Codex approval.
