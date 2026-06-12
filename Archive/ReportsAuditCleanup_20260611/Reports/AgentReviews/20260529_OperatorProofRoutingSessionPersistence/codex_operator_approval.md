Codex Approval: APPROVE

## Approved Task

Implement the user's approved Operator-process fixes:

1. Make Claude session persistence effectively opt-in for Operator runs where `MaxTurns = 0`, because persistence only supports max-turn resume and can trigger `thinking` / `redacted_thinking` resume failures.
2. Route proof-bearing work to FullOperator earlier: if acceptance requires builds, commandlets, captures, gameplay proof, or visual judgment, read-only packets are planning only and the implementation/proof phase must be FullOperator after Codex approval.
3. Keep Codex as the final proof owner: Claude may run/build/capture in FullOperator mode, but Codex validates actual evidence and remains the user-facing integrator/final reporter.

## Approved Scope

- Edit `Scripts\Invoke-ClaudeDirectRead.ps1` for the session-persistence default/policy and preflight/manifest truthfulness.
- Edit `OPERATOR_VALIDATOR_PROTOCOL.md` for proof-bearing FullOperator routing and Codex final proof ownership.
- Edit `AGENTS.md` only as short router wording if needed.
- Edit `Scripts\Invoke-ClaudeReadOnlyOperator.ps1` only if wrapper parameter forwarding/help text must stay consistent.
- Do not edit runtime/gameplay/content/assets/build outputs/usage widget.

## Approved Tool Surface

Claude FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1`, using `-NoSessionPersistence` for this run because this task is explicitly fixing session persistence behavior.

## Required Process Rules

- Follow root `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, and `Reports\AGENTS.md`.
- Do not use native goal functions.
- Do not use Claude plan mode.
- Preserve unrelated user changes.
- Keep `AGENTS.md` as router and `OPERATOR_VALIDATOR_PROTOCOL.md` as detailed authority.
- No broad Git/LFS scans over Unreal binary asset folders.

## Explicitly Excluded Actions

- No Unreal editor, Blender, Niagara, commandlet, staged build, commit, push, tag, reset, clean, or destructive operation.
- No gameplay/runtime/content changes.
- No quota denominator work.
- No usage widget work.

## Verification Required After Operator Run

- PowerShell parse-check edited `.ps1` files.
- Preflight checks that show:
  - FullOperator default with `MaxTurns = 0` has `SessionPersistence: False`.
  - FullOperator with explicit `-MaxTurns N` keeps session persistence available unless disabled.
  - `-NoSessionPersistence` still forces no persistence.
  - ReadOnly defaults remain sane.
- Narrow text search for proof-routing/final-proof-owner wording.
- Narrow `git diff --check` for touched files.

## Approval Rationale

These changes directly address the latest user-reported Claude Operator failure modes while preserving the current Claude-as-Operator / Codex-as-validator architecture.
