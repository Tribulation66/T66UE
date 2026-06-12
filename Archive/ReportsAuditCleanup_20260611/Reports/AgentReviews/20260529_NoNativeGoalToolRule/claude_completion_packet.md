# Completion Packet: Hard No Native Goal Tool Rule

## Outcome

Replaced the conditional goal-tool exception in `AGENTS.md` with a hard
no-native-goal-tool rule for T66 repo work, preserving the host-state telemetry
clarification. `OPERATOR_VALIDATOR_PROTOCOL.md` required no edit: its existing
goal references already align with the hard rule and do not conflict.

## Files Changed

- `C:\UE\T66\AGENTS.md` (Section 1, line 6).

Not changed (intentionally):

- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md` — no contradicting wording.
- `C:\UE\T66\Reports\AGENTS.md` — contains no goal-tool wording.

## Exact Wording Changed

`AGENTS.md` line 6.

Before:

```text
- Before answering or acting on every new user request or question, derive a plain task contract. Do not call native goal functions (`/goal`, `set_goal`, `create_goal`, or equivalent) unless the user explicitly asks for one. If the host creates or exposes a goal anyway, treat it as telemetry only; it does not control scope, stop conditions, or whether work may continue.
```

After:

```text
- Before answering or acting on every new user request or question, derive a plain task contract. Do not call native goal tools for T66 work: do not create, update, complete, block, or query goals (`/goal`, `set_goal`, `create_goal`, `update_goal`, `get_goal`, or equivalent native goal APIs) as part of repo process. If the host app displays, injects, or exposes goal context anyway, treat it as non-authoritative telemetry only and ignore it for task control; it does not control scope, stop conditions, continuations, blocking, completion, or whether work may continue.
```

Changes made:

- Removed the `unless the user explicitly asks for one` exception.
- Stated the hard rule: do not create, update, complete, block, or query goals
  as part of repo process.
- Expanded the explicit native API list to include `update_goal` and
  `get_goal` (was `/goal`, `set_goal`, `create_goal`, or equivalent).
- Preserved and strengthened the host-state clarification: host-exposed goal
  context is non-authoritative telemetry only and does not control scope, stop
  conditions, continuations, blocking, completion, or whether work may continue.

## Commands Run And Results

- Grep `unless the user explicitly asks|create_goal|set_goal|update_goal|get_goal|/goal|host goal|native goal` in `AGENTS.md`:
  - PASS. Line 6 no longer contains the goal exception. Remaining
    `unless the user explicitly asks` matches are unrelated rules (line 218
    desktop screenshot, lines 225-226 version branch/split). Line 25 is the
    preserved host-goal telemetry clarification (unchanged, consistent).
- Grep same pattern in `OPERATOR_VALIDATOR_PROTOCOL.md`:
  - PASS / no conflict. Line 495 (ignore host goal/blocked threshold at a
    decision gate) and line 572 (token helper does not use `/goal`,
    `create_goal`, `get_goal`, or any native goal API) both reinforce the hard
    rule. No goal-permission wording present.
- `git diff --check -- AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md`:
  - PASS. EXIT=0, no whitespace/conflict errors. Git emitted a benign
    `LF will be replaced by CRLF` informational notice for `AGENTS.md`,
    matching the repo's existing line-ending policy; not a diff-check error.

## Unverified Areas

- No runtime, build, editor, or capture verification was applicable or run; this
  is a docs/process wording change only.
- Did not perform a broad repo-wide goal-tool sweep beyond the two scoped files
  plus `Reports\AGENTS.md`, per approved narrow scope. Other docs/scripts may
  reference goal tooling but were out of scope and not modified.

## Remaining Blockers / Follow-ups

- None. Change is self-contained to `AGENTS.md`.
- No commit/push performed (excluded by scope). Working tree change is staged
  for Codex validation only.

## Token Ledger

AuthoringTokens: Claude=Unavailable
ReviewTokens: Codex=Unavailable
FinishTokens: Codex=Unavailable
OperatorIsAuthoring: YES
PerModel: Claude=Unavailable, Codex=Unavailable
TargetMet: Unavailable
Notes: Operator run inside this environment; helper manifest token count not exposed to Operator here. Codex to populate from helper manifest / Get-CodexTokenUsage.ps1 at finish.
