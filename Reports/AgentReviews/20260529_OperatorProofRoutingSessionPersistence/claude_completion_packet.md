# Claude Operator Completion Packet: Session Persistence And Proof Routing

This is an Operator work artifact, not a greenlight. Codex validates the actual
changes and writes the final user-facing report.

## Outcome

Implemented all three approved fixes:

1. Session persistence is now opt-in by max-turn cap in the Claude helper.
2. Protocol/docs now route proof-bearing work to FullOperator.
3. Protocol/docs now codify Codex as the final proof owner.

## Files Changed

- `Scripts\Invoke-ClaudeDirectRead.ps1`
- `Scripts\Invoke-ClaudeReadOnlyOperator.ps1`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `AGENTS.md`

## Exact Changes Made

### `Scripts\Invoke-ClaudeDirectRead.ps1`

- Comment-based help (`.DESCRIPTION`): rewrote the session-persistence paragraph
  to state the new opt-in rule — off by default when `-MaxTurns <= 0`, on when
  `-MaxTurns > 0`, with `-NoSessionPersistence` forcing off and the new
  `-SessionPersistence` forcing on; notes the rationale (no max-turn resume to
  support when unbounded; avoids thinking/redacted_thinking resume failures) and
  that effective persistence is reported truthfully and disables max-turn
  continuation when off.
- Param block: added `[switch] $SessionPersistence` (with a comment) next to the
  existing `[switch] $NoSessionPersistence`.
- Added an effective-persistence resolution block before the effective-capability
  fields:
  - Throws `FailureKind=ClaudeProcessFailed; -NoSessionPersistence and
    -SessionPersistence are mutually exclusive.` when both switches are passed.
  - `$EffectiveSessionPersistence` = `$false` if `-NoSessionPersistence`, `$true`
    if `-SessionPersistence`, else `$MaxTurns -gt 0`.
  - `$SessionPersistenceSource` string describing the chosen source for
    preflight/manifest transparency.
- Preflight report: `SessionPersistence:` now prints `$EffectiveSessionPersistence`
  (was `-not [bool]$NoSessionPersistence`); added a `SessionPersistenceSource:`
  line.
- Main attempt invocation: now passes
  `-NoSessionPersistence:(-not $EffectiveSessionPersistence)` instead of the raw
  bound switch, so the run and its max-turn continuation honor the effective
  value. (Continuation was already gated on `-not $NoSessionPersistence` inside
  `Invoke-ClaudeAttempt`, so continuation stays disabled when effective
  persistence is off.)
- Failure manifest and success manifest: added `SessionPersistence` and
  `SessionPersistenceSource` fields; `NoSessionPersistence` now records
  `(-not $EffectiveSessionPersistence)` so it reflects the truthful effective
  value rather than only the raw switch.

### `Scripts\Invoke-ClaudeReadOnlyOperator.ps1`

- Added `[switch] $SessionPersistence` to the param block (parity with
  `-NoSessionPersistence`). The wrapper already forwards all bound params
  generically, so the new switch reaches `Invoke-ClaudeDirectRead.ps1`.
- `.DESCRIPTION`: added a short paragraph describing the same opt-in rule and the
  two override switches.

### `OPERATOR_VALIDATOR_PROTOCOL.md`

- Core Invariants: added two bullets — proof-bearing work routes to FullOperator
  (with an in-doc link), and Codex is the final proof owner.
- New section `## Proof-Bearing Work Routing` after Phase-Bounded Operator Tasks:
  defines a proof-bearing task (build/compile logs, commandlet markers,
  runtime/editor captures, gameplay proof, visual judgment, multiple proof
  classes); states a read-only packet may plan but cannot be the proof phase; the
  implementation/proof phase must be a Codex-approved FullOperator run; composes
  with phase-bounding.
- New section `## Codex As Final Proof Owner`: Claude-produced proof is evidence
  not acceptance; Codex validates existence/freshness/scope/adequacy and may
  deepen under escalation triggers; Codex remains user-facing integrator/reporter
  unless the user changes that.

### `AGENTS.md`

- Operator/Validator Protocol section: added one router bullet summarizing
  proof-bearing FullOperator routing and the Codex final-proof-owner rule, with
  `OPERATOR_VALIDATOR_PROTOCOL.md` named as the owning authority.

## Commands Run And Results

All run via `pwsh -NoProfile`.

- Parse-check (`[Parser]::ParseFile`) of both edited scripts — **PASS**
  (`Invoke-ClaudeDirectRead.ps1: PARSE OK`, `Invoke-ClaudeReadOnlyOperator.ps1:
  PARSE OK`).
- FullOperator preflight, default (`MaxTurns=0`) — **PASS**:
  `SessionPersistence: False`, source `default off (MaxTurns <= 0 ...)`.
- FullOperator preflight, `-MaxTurns 10` — **PASS**: `SessionPersistence: True`,
  source `default on (MaxTurns > 0 can --resume on max-turn)`.
- FullOperator preflight, `-NoSessionPersistence` — **PASS**:
  `SessionPersistence: False`, source `forced off by -NoSessionPersistence`.
- FullOperator preflight, `-MaxTurns 10 -NoSessionPersistence` — **PASS**:
  `SessionPersistence: False` (force-off overrides the MaxTurns>0 default).
- FullOperator preflight, `-SessionPersistence` (MaxTurns=0) — **PASS**:
  `SessionPersistence: True`, source `forced on by -SessionPersistence`.
- Conflict, both switches — **PASS**: throws the mutual-exclusion error.
- ReadOnly wrapper preflight, default — **PASS**: `ToolProfile: ReadOnly`,
  `SessionPersistence: False`, `TimeoutPolicy: 180 s per attempt`,
  `MutatingCapability: False`.
- ReadOnly wrapper preflight, `-MaxTurns 5` — **PASS**: `SessionPersistence:
  True`.
- ReadOnly wrapper preflight, `-SessionPersistence` — **PASS**:
  `SessionPersistence: True` (switch forwarded correctly).
- Narrow text search in `OPERATOR_VALIDATOR_PROTOCOL.md` for proof-routing /
  final-proof-owner wording — **PASS** (both new sections + invariant bullets
  present).
- `git diff --check` on the four touched files — **CLEAN** (only a benign
  `LF will be replaced by CRLF` warning on `AGENTS.md`; no whitespace errors).

## Unverified Areas

- No live Claude run was performed; all helper checks were `-Preflight` only
  (per approval, no Claude invocation needed). Runtime continuation behavior with
  a real session was not exercised end-to-end, but the continuation gate is
  unchanged in logic and now keyed off the effective value.
- Did not edit `Scripts\Invoke-ClaudePlanReview.ps1` (not in scope; the approval
  named only the four files above).

## Remaining Blockers / Follow-ups

- None blocking. Optional future polish: a unit-style harness for the
  effective-persistence matrix, but that exceeds approved scope.

## Constraints Honored

- No Unreal/Blender/Niagara/commandlet/staged-build/runtime/content/asset work.
- No git commit/push/tag/reset/clean; no broad Git/LFS scan (diff --check was
  narrowed to the four touched non-binary files).
- No native goal functions; no Claude plan mode.
- Preserved unrelated user changes (only targeted edits to the four files; the
  pre-existing `AGENTS.md` modifications were left intact).
