# Completion Packet: Phase-Bounded FullOperator Timeout Policy

## Outcome

Implemented the two approved process improvements inside the Codex-approved scope:

1. Added a phase-bounded Operator task rule for broad implementation work to
   `OPERATOR_VALIDATOR_PROTOCOL.md`, with a router pointer in `AGENTS.md`.
2. Changed the FullOperator helper timeout default from `1200s` to `0`
   (unbounded) so implementation phases are not killed mid-run, and made the
   protocol doc + helper synopsis/comments/preflight truthful. Read-only profiles
   and explicit timeboxed probes keep the shorter guard.

This is an Operator work artifact, not a greenlight. Codex validates the actual
changes and writes the final user-facing report.

## Files Changed

- `Scripts/Invoke-ClaudeDirectRead.ps1`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `AGENTS.md`

## Exact Changes Made

### `Scripts/Invoke-ClaudeDirectRead.ps1`

- Timeout default resolution: FullOperator default changed from `1200` to `0`
  (unbounded); read-only profiles still default to `180`. Added an explanatory
  comment that phases default unbounded and `-TimeoutSeconds N` intentionally
  timeboxes a probe.
- `-TimeoutSeconds` param comment block updated from "FullOperator defaults to
  1200s" to "FullOperator defaults to 0 (unbounded) ... read-only profiles
  default to 180s. Pass an explicit -TimeoutSeconds N to timebox a probe."
- `.DESCRIPTION` synopsis updated to state the unbounded FullOperator default,
  the 180s read-only guard, and the explicit-timebox opt-in.
- No change to the preflight code itself; the existing `$TimeoutPolicy`
  derivation (`-le 0` => "Unbounded (no wall-clock guard)") now reports the new
  default correctly.

### `OPERATOR_VALIDATOR_PROTOCOL.md`

- Helper Timeout bullet rewritten: FullOperator default is now `0` (unbounded);
  read-only stays `180`; explicit `-TimeoutSeconds N` only for an intentional
  timebox; read-only runs and timeboxed probes keep the shorter guard.
- New `## Phase-Bounded Operator Tasks` section added (between Operator/Validator
  Flow and Operator Packet): defines broad tasks (multi-system; Unreal/editor
  asset work; runtime+assets; docs+runtime+proof; build/capture harness plus
  implementation; verification spanning several proof classes), requires a phase
  plan in the packet before approval, approves phases one at a time (plan
  approval does not pre-approve later phases, each phase needs its own approval
  artifact and completion packet), gives a non-rigid recommended 5-phase shape,
  and preserves Claude-authors / Codex-validates across phases.
- Codex Approval Gate: added a paragraph requiring a phase plan for broad tasks
  and per-phase approval instead of one open-ended approval.

### `AGENTS.md`

- Added one router bullet under the Operator/Validator Protocol section: broad
  implementation tasks split into bounded phases with an approved phase plan;
  FullOperator implementation phases default to unbounded (`-TimeoutSeconds 0`)
  unless intentionally timeboxed; points to `OPERATOR_VALIDATOR_PROTOCOL.md` as
  the owning authority. `AGENTS.md` kept as a short router; detail lives in the
  protocol file.

## Commands Run And Results

- PowerShell parse-check of `Scripts/Invoke-ClaudeDirectRead.ps1` via
  `[System.Management.Automation.Language.Parser]::ParseFile` — PASS
  (`PARSE OK`, no errors). `Invoke-ClaudeReadOnlyOperator.ps1` was not edited.
- FullOperator preflight:
  `Invoke-ClaudeDirectRead.ps1 -Mode Operator -ToolProfile FullOperator
  -CodexApprovalPath <approval> -Preflight` — PASS. Reports
  `TimeoutPolicy: Unbounded (no wall-clock guard)`, `ApprovalRequired: True`,
  `CodexApprovalStatus: Valid`, `MutatingCapability: True`.
- Read-only preflight: `Invoke-ClaudeReadOnlyOperator.ps1 -Preflight` — PASS.
  Reports `TimeoutPolicy: 180 s per attempt` (short guard preserved).
- Explicit timebox preflight: FullOperator with `-TimeoutSeconds 600 -Preflight`
  — PASS. Reports `TimeoutPolicy: 600 s per attempt` (intentional timebox still
  honored).
- `git diff --check -- AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md
  Scripts/Invoke-ClaudeDirectRead.ps1` — CLEAN (no whitespace errors; only a
  benign LF→CRLF informational warning on `AGENTS.md`).

## Unverified Areas

- No Claude FullOperator run was actually launched (only `-Preflight`), so live
  unbounded-run behavior was not exercised end to end — the timeout-resolution
  and preflight logic are verified by parse + preflight only.
- `Scripts/Invoke-ClaudeReadOnlyOperator.ps1` was not modified; its `-1`
  sentinel forwards to the same resolution path, confirmed via its preflight.

## Remaining Blockers / Follow-ups

- None. Changes are confined to the approved docs/script scope.
- Pre-existing unrelated working-tree changes (e.g. prior `M AGENTS.md` and
  other tracked edits from the session snapshot) were preserved; only the single
  additive router bullet was added to `AGENTS.md`.

## Helper Run Directory / Manifest

- Not applicable: this task did not invoke the Claude helper for its own work
  (the Operator ran directly via the approved FullOperator session). No
  `Reports/AgentReviews/ClaudeDirectRead/<Run>/manifest.json` was produced by
  this task. Preflight runs do not write manifests.

## Token Ledger

AuthoringTokens: Claude=Unavailable
ReviewTokens: Codex=Unavailable
FinishTokens: Codex=Unavailable
OperatorIsAuthoring: YES
PerModel: Claude=Unavailable, Codex=Unavailable
TargetMet: Unavailable
Notes: This Operator run did not expose a parseable token count to itself; Codex
should populate the ledger from its own usage and the helper/host counts at
final-report time.

## Caveats

- Operator artifact only; Codex must validate the actual diff, re-confirm the
  preflight policy, and write the final user-facing report.
