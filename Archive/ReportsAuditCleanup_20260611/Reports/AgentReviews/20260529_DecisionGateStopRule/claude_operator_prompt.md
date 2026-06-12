You are Claude acting as full T66 Operator under Codex approval.

Working goal: tighten the AGENTS/process rule so a `NEEDS_HUMAN_DECISION` verdict stops immediately at the first valid decision gate instead of continuing adjacent work, packet polishing, reruns, or repeated audit turns.

User context:
- The user gave a B.13 example where Claude returned `NEEDS_HUMAN_DECISION`, but subsequent turns kept auditing and editing packets before finally reporting blocked.
- The expected behavior is: after the first valid `NEEDS_HUMAN_DECISION`, save/reference `decision_block.md`, ask the user once, and stop. On continuations before the user answers, do not rerun review, revise packets, edit docs, inspect more, or perform adjacent work. Just reference the saved decision block.
- The native goal tool may have a separate blocked-threshold policy. The process docs should distinguish immediate "decision gate stop" from whether the host permits setting the formal `blocked` status on that first turn.

Read:
- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`

Approved edit scope:
- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AgentReviews/20260529_DecisionGateStopRule/completion_packet.md`

Make the minimal docs/process changes needed. Preserve unrelated wording.

Implementation requirements:
1. Add/strengthen a rule that the first valid `NEEDS_HUMAN_DECISION` or equivalent user-only decision gate is an immediate stop point.
2. State that agents must not do packet polish, rerun reviews, continue audits, revise docs, perform adjacent implementation, or ask repeated versions of the same decision while waiting for the user.
3. State that continuations before the user answers must only reference the saved decision block and current decision choices.
4. State that if the native goal tool cannot mark `blocked` immediately because of host policy, the agent must still stop work and state the decision-gated status in conversation.
5. Add an equivalent rule in the Operator/Validator protocol verdict handling.
6. Write a concise completion packet summarizing files changed and validation suggestions.

Do not run broad Git/LFS scans. Do not touch runtime source or assets.
