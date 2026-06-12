You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\ValidatorDepthTightening\validator_depth_answer_packet.md
- Output scope: review of the packet below only.

<review_packet>
Working task:
Operator: Codex
Validator: Claude
Scope: Answer how to make Claude's validator role more thoughtful for this and future T66 chats, especially around risks and oversights. No production code, gameplay data, assets, config, content, or save changes.
Stop condition: Provide concrete options, separating immediate per-run practice from durable protocol/helper changes.

Validation depth requested: targeted

User constraints:
- Codex is currently Operator and Claude is currently Validator.
- The user is asking how to adjust the validation process before sending broad implementation prompts.
- Do not implement process changes in this answer unless explicitly requested.
- Mini/minigame scope is excluded.

Applicable instructions read:
- C:\UE\T66\AGENTS.md as supplied by the user.
- C:\UE\T66\.t66\operator-state.json: Codex Operator, Claude Validator.
- C:\UE\T66\Reports\AGENTS.md for this packet location.
- C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md validator sections.
- C:\UE\T66\Scripts\Invoke-ClaudePlanReview.ps1 parameter and prompt sections.

Live findings:
- `OPERATOR_VALIDATOR_PROTOCOL.md:396-397` already supports `ExpectedValidatorDepth: targeted | deepened` and `ValidatorBudgetHint`.
- `OPERATOR_VALIDATOR_PROTOCOL.md:485-514` defines escalation triggers and requires deepened validation reporting when the validator deepens.
- `OPERATOR_VALIDATOR_PROTOCOL.md:558-566` requires validator output to include completeness, anchor checks, scope checks, findings, missing verification, depth, and token spend.
- `Scripts\Invoke-ClaudePlanReview.ps1:32-44` exposes `-MaxTurns` from 1 to 10 with default 10, and Claude `-Effort` values `low`, `medium`, `high`, `xhigh`, and `max`.
- `Scripts\Invoke-ClaudePlanReview.ps1:378-408` currently instructs Claude to be strict about contradictions, missing verification, unsafe scope, and unclear goals, with fixed review headings.

Answer draft:
The best way to make Claude's validator role more thoughtful is to change the validation packet and invocation, not just ask Claude to be "stricter."

For this chat, I can do it immediately without repo changes:
- Mark broad change packets as `ExpectedValidatorDepth: deepened`.
- Fill `ValidatorBudgetHint` with the exact risks and anchors Claude should inspect, such as spawn rules, data-table implications, UI screen impact, save/runtime compatibility, asset reload/cook paths, cleanup/deprecation scope, and verification gaps.
- Invoke Claude validation with `-Effort high` or `-Effort xhigh` for broad architecture/change plans. The helper currently allows `-Effort low`, `medium`, `high`, `xhigh`, or `max`, and `-MaxTurns` defaults to 10 with an allowed range of 1 to 10.
- Require the Operator packet to include "decisions the Validator should challenge" and a risk/oversight checklist before Claude sees it.
- Treat Claude's job as risk-focused gatekeeping: check assumptions, missed coupling, stale evidence, verification adequacy, and scope boundaries. It should not become a second implementation pass unless the packet is too incomplete and needs `REVISE`.

For the upcoming large change set, my recommended default is:
- Phase the work before implementation.
- For each phase, produce a Codex Operator packet with `ExpectedValidatorDepth: deepened`.
- Run Claude with `-Effort high` for normal phases and `-Effort xhigh` for migration/deletion/asset/runtime-proof phases.
- Ask Claude to return findings as Blocker, Major, Minor, plus missing verification and assumptions challenged.
- Only proceed when the packet passes completeness and the risk review.

For future chats, the durable solution is a small repo process/helper change:
1. Update `OPERATOR_VALIDATOR_PROTOCOL.md` so broad implementation plans, cleanup/deprecation plans, UI/VFX/runtime-proof work, and multi-system gameplay changes default to `deepened` validation.
2. Add a `-ReviewDepth deepened` or `-RiskReview` switch to `Scripts\Invoke-ClaudePlanReview.ps1` that injects extra review requirements into the Claude prompt.
3. Extend the Claude review prompt with required sections such as:
   - Packet Completeness Gate
   - Anchor Checks
   - Risk/Oversight Review
   - Assumptions Challenged
   - Missing Verification
   - Verdict
4. Add explicit prompts for: "What hidden coupling could this miss?", "What would break if the main assumption is wrong?", "What stale-doc/live-code mismatch should be checked?", and "What weak implementation could appear to pass but fail the user's real goal?"
5. Optionally make the helper default to `-Effort medium` or `-Effort high` when the packet says `ExpectedValidatorDepth: deepened`.

The operator state file only controls roles. It does not currently persist validation depth, effort, or risk-review behavior. So for this chat I can apply the more thoughtful validator convention manually; for future chats, it should be encoded in the protocol/helper.

Recommended next action:
If you want this to stick across future chats, ask me to make a process-only change to the protocol and Claude review helper. I would keep that scoped to `OPERATOR_VALIDATOR_PROTOCOL.md` and `Scripts\Invoke-ClaudePlanReview.ps1`, then have Claude validate the change.

ExpectedValidatorDepth: targeted
ValidatorBudgetHint: Check whether the answer correctly separates immediate operating practice from durable repo changes; check whether it overstates current helper capabilities; check whether it respects the user's request not to implement changes yet.

</review_packet>
