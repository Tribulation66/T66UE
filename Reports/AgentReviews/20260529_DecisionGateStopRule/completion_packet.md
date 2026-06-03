# Completion Packet: Decision Gate Stop Rule

## Outcome

Tightened the T66 process docs so the first valid `NEEDS_HUMAN_DECISION` verdict
or equivalent user-only decision gate is an immediate stop point. Agents must
stop all other work, save/reference the decision block, ask once, and on
continuations before the user answers only reference the saved decision block.
Removed reliance on native goal-tool blocked thresholds: decision-gate stop is
now process-controlled and immediate.

## Files Changed

- `AGENTS.md` — Section 1 Project Contract. Added three bullets after the
  existing decision-gate wording: (1) a Decision Gate Stop rule naming the first
  valid `NEEDS_HUMAN_DECISION`/equivalent as an immediate stop with an explicit
  no-do list (no packet polish, rerun reviews, continued audits, doc revision,
  further inspection, adjacent implementation, or reworded repeats); (2) a
  continuation rule restricting pre-answer turns to referencing the saved
  decision block and current choices; (3) a host-tool carve-out stating that
  host goal/blocked status does not control whether work continues.
- `OPERATOR_VALIDATOR_PROTOCOL.md` — Validator Verdict Contract. Added a
  `### Decision Gate Stop Rule` subsection directly after the verdict meanings,
  mirroring the AGENTS.md rule for both Operator and Validator, including the
  same no-do list, continuation restriction, and host-policy carve-out.
- `Reports/AgentReviews/20260529_DecisionGateStopRule/completion_packet.md` —
  this packet.

## B.13 Failure Mode Addressed

In the B.13 example, Claude returned `NEEDS_HUMAN_DECISION` but subsequent turns
kept auditing and editing packets before finally reporting blocked. The correct
behavior, now codified in both docs, is: stop immediately at the first valid
`NEEDS_HUMAN_DECISION`, save/reference `decision_block.md`, ask the user once,
and on every continuation before the user answers only reference the saved
decision block and current choices — no reruns, no packet revision, no further
discovery, no adjacent work.

## Verification

- Edits were exact-string replacements that preserved surrounding wording; no
  unrelated lines were modified.
- Suggested validation for Codex:
  - `git diff --check` on the two touched docs (`AGENTS.md`,
    `OPERATOR_VALIDATOR_PROTOCOL.md`) — expect clean except known CRLF/line-
    ending warnings.
  - Read-back of the inserted bullets in `AGENTS.md` Section 1 and the
    `Decision Gate Stop Rule` subsection in the protocol to confirm the four
    required rules are present and the host-policy distinction is explicit.
  - Confirm no runtime source, assets, or Git state were touched (only the three
    approved files changed).

## Review / Validation

Pending Codex validation of the actual diff. This Operator artifact is not a
greenlight.

## Token Ledger
AuthoringTokens: claude-opus-4-8:Unavailable
ReviewTokens: Unavailable
FinishTokens: Unavailable
OperatorIsAuthoring: YES
PerModel: Claude=Unavailable, Codex=Unavailable
TargetMet: Unavailable
Notes: Helper token manifest not exposed in this run; report Unavailable rather
than estimate.

## Caveats

- Docs-only change. No behavioral enforcement exists in tooling; the rule relies
  on agent adherence to AGENTS.md and the Operator/Validator protocol.
- Native goal tools are no longer part of the process contract unless the user
  explicitly asks to use them.
