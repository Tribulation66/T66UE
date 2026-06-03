Done. Operator work complete and within approved scope.

**What changed**
- `C:\UE\T66\AGENTS.md` (Section 1, line 6): replaced the `unless the user explicitly asks for one` goal exception with a hard no-native-goal-tool rule — do not create, update, complete, block, or query goals (`/goal`, `set_goal`, `create_goal`, `update_goal`, `get_goal`, or equivalent native goal APIs) as part of repo process. Preserved and strengthened the host-state clarification: host-exposed goal context is non-authoritative telemetry only and does not control scope, stop conditions, continuations, blocking, completion, or whether work may continue.

**Not changed (intentionally)**
- `OPERATOR_VALIDATOR_PROTOCOL.md` — its goal references (lines 495, 572) already align with the hard rule and don't conflict, so no edit was needed.
- `Reports\AGENTS.md` — contains no goal-tool wording.

**Verification**
- Narrow grep of both target files: AGENTS.md line 6 no longer carries the exception; remaining "unless the user explicitly asks" hits are unrelated screenshot/version rules; protocol references reinforce the rule.
- `git diff --check -- AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md` → EXIT=0 (only a benign LF→CRLF informational notice).

**Completion packet:** `C:\UE\T66\Reports\AgentReviews\20260529_NoNativeGoalToolRule\claude_completion_packet.md`

No commit/push performed (excluded by scope). This artifact is for Codex validation, not a greenlight.
