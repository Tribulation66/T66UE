Codex Approval: APPROVE

## Approved Task

Implement the user's requested process simplification: T66 agents must not use native goal tools at all. If the host displays, injects, or exposes goal state anyway, that state is non-authoritative telemetry only and must be ignored for T66 process control.

## Approved Scope

- Edit `AGENTS.md` to remove the current "unless the user explicitly asks for one" exception and replace it with a hard no-native-goal-tool rule.
- Edit `OPERATOR_VALIDATOR_PROTOCOL.md` only if needed to keep the detailed Operator/Validator process consistent with the hard no-goal rule.
- Do not edit scripts unless a directly contradictory goal-tool instruction is found in a script comment or helper output.
- Do not edit gameplay code, assets, Unreal content, build output, usage widget code, or unrelated process sections.

## Approved Tool Surface

Claude FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1`, limited to file edits and narrow text/search validation inside the approved scope.

## Required Process Rules

- Follow root `AGENTS.md` and `Reports\AGENTS.md`.
- Keep `AGENTS.md` as the short router and `OPERATOR_VALIDATOR_PROTOCOL.md` as the detailed authority.
- Do not use Claude plan mode.
- Do not use native goal functions.
- Preserve unrelated user changes.
- Do not run broad Git/LFS scans over Unreal binary asset folders.

## Explicitly Excluded Actions

- No runtime/gameplay/content edits.
- No Unreal editor, Blender, Niagara, commandlet, staged build, commit, push, tag, reset, clean, or destructive operation.
- No quota denominator work.
- No usage widget work.

## Verification Required After Operator Run

- Codex must inspect the changed wording.
- Codex must run narrow text search for remaining native-goal permission wording.
- Codex must run narrow `git diff --check` for touched files.

## Approval Rationale

This is a bounded docs/process update requested by the user. Claude is the global Operator and should make the authoring change; Codex will validate the resulting wording and report back.
