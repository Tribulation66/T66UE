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

Review depth: deepened.
Deepened validation is the risk-focused review mode. It is not a second implementation pass.
Keep the exact output headings below. Put risk/oversight content inside those headings; do not add headings.

For deepened review, actively look for:
- Hidden coupling, stale-doc/live-code mismatch, and assumptions that could make the plan incomplete.
- Unsafe cleanup, deprecation, deletion, migration, data reload, asset cook, or runtime/source mismatch consequences.
- Scope bleed, especially into explicitly excluded systems.
- Verification gaps, weak pass markers, rollback gaps, and evidence that would fail to prove the user's stated goal.
- The weakest implementation that could appear to pass while missing the real intent.

In Clarifying Questions, ask only user-owned decisions that block safe progress.
In Required Verification, name exact verification gaps and expected pass markers.
In Rationale, summarize the main assumption or oversight risk you challenged.

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
- Packet path: C:\UE\T66\Reports\AgentReviews\ValidatorPipelineExplanation\answer_packet.md
- Output scope: review of the packet below only.

<review_packet>
Working task:
Operator: Codex
Validator: Claude
Scope: Explain the actual T66 validation process and pipeline in detail, including decisions and categorization. No code/config/content changes.
Stop condition: Deliver a detailed plain-language breakdown after validation.

Validation depth: deepened, because the user explicitly rejected the simplified answer and asked for the actual process pipeline.

Applicable live findings:
- `.t66\operator-state.json` currently sets `operator=Codex` and `validator=Claude`.
- `AGENTS.md` requires task contracts, folder instruction discovery, planning for changes, always-on validation, token reporting, and no native goal tools.
- `OPERATOR_VALIDATOR_PROTOCOL.md` separates process class (`quick/full`) from review depth (`targeted/deepened`).
- `OPERATOR_VALIDATOR_PROTOCOL.md` says the Operator owns most authoring work and the Validator owns critique/gatekeeping.
- `OPERATOR_VALIDATOR_PROTOCOL.md` requires the Packet Completeness Gate before validator discovery.
- `OPERATOR_VALIDATOR_PROTOCOL.md` says proof-bearing work routes to FullOperator and broad work is phase-bounded.
- `OPERATOR_VALIDATOR_PROTOCOL.md` defines verdicts: APPROVE, REVISE, NEEDS_HUMAN_DECISION, BLOCK.
- `Scripts\Invoke-ClaudePlanReview.ps1` supports `-ReviewDepth targeted|deepened` and `-RiskReview` for Claude validation.

Answer draft:

Current setup:
Codex is the Operator. Claude is the Validator. That means Codex does the main work and Claude checks the sealed packet before the answer/result is treated as valid.

Full pipeline:

1. Request arrives.
   Codex writes the task contract: working task, operator, validator, scope, stop condition.
   Decision: is this a simple answer, a read-only investigation, a planning task, an implementation task, proof-bearing work, or a release/build task?

2. Role state is read.
   The repo state file decides who is Operator and Validator.
   Current decision: Operator = Codex, Validator = Claude.

3. Scope is classified.
   Codex checks whether Mini/minigame scope is explicitly included, whether folder AGENTS files apply, whether pending issues apply, whether PPF applies, and whether a human decision is needed.
   Decision: if the task has an unresolved product/risk/scope decision, stop and ask once instead of continuing.

4. Validation class is chosen.
   Quick validation is for low-risk answers.
   Full validation is for code, docs, config, workflow, Unreal assets, releases, durable recommendations, or anything where being wrong would cause real rework.
   Decision: quick or full.

5. Review depth is chosen.
   Targeted review checks packet completeness, cited anchors, scope, instructions, and verification.
   Deepened review is risk-focused: hidden coupling, stale evidence, cleanup risk, data/asset reload risk, runtime/source mismatch, weak verification, rollback gaps, and assumptions.
   Decision: targeted or deepened. For broad/high-risk packets, use deepened and usually `-Effort high`.

6. Broad/proof-bearing routing is decided.
   If the work is broad, split it into bounded phases.
   If the work needs proof like build logs, commandlet markers, editor captures, gameplay proof, or visual proof, route proof production through FullOperator rather than accepting a read-only plan as proof.
   Decision: one phase or multiple phases; ordinary operator work or FullOperator proof work.

7. Operator creates the packet.
   The packet includes the task, roles, constraints, instructions read, live findings, PPF/process gates, proposed patch approach, verification plan, token routing, open decisions, and anti-lookalike discriminator when needed.
   Decision: if the packet is too thin, it should not go to implementation.

8. If Claude is Operator, Codex approval happens before mutating work.
   Codex checks whether Claude's requested scope, tool surface, file targets, and verification are acceptable.
   Decision: Codex Approval APPROVE or REJECT.
   In the current setup, Codex is Operator, so this approval gate is mostly relevant when Claude is doing the main work.

9. Validator runs the Packet Completeness Gate.
   First question: is the packet complete enough to review?
   If required sections are missing, the Validator returns REVISE and does not spend time filling gaps.

10. Validator checks the packet.
   It checks cited anchors, contradictions, scope, process instructions, verification adequacy, and risks.
   In deepened mode, it also challenges assumptions and asks what weak implementation could appear to pass.
   Decision: stay targeted or deepen under escalation triggers.

11. Validator returns a verdict.
   APPROVE: safe to proceed/report under reviewed scope.
   REVISE: Codex/Operator can fix the packet or work and rerun validation.
   NEEDS_HUMAN_DECISION: only the user can decide; save/report the decision and stop.
   BLOCK: hard blocker, unavailable prerequisite, unsafe contradiction, or external issue.

12. Operator performs or finalizes the work.
   With Codex as Operator, Codex edits/runs checks if implementation is in scope.
   With Claude as Operator, Claude performs approved work, then Codex validates actual workspace changes and proof.

13. Verification is reviewed.
   The final answer must say what verification ran, what passed, and what was skipped.
   For runtime/visual/proof work, older evidence cannot replace a requested current proof.

14. Final report goes to the user.
   Codex remains responsible for the final user-facing answer.
   Token spend is reported.
   Any validator artifact path is reported when relevant.

Claude-as-Validator vs Codex-as-Validator:

Claude as Validator:
- Codex does the main work.
- Claude reviews the sealed packet through `Invoke-ClaudePlanReview.ps1`.
- Claude is read-only in that mode: no edits, no commands, no implementation.
- Best use: independent critique of Codex's plan/result before it reaches the user.

Codex as Validator:
- Claude does the main work, usually as Operator.
- Codex approves Claude's scope before mutating work.
- Codex validates the actual local workspace, changed files, logs, captures, and proof.
- Best use: when Claude is doing the expensive operator work, but Codex must remain the final proof owner.

The most important difference:
Claude-as-Validator reviews Codex's packet. Codex-as-Validator validates Claude's actual work in the live workspace.

ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Check whether this explains the full pipeline, decisions, categories, and Claude-vs-Codex validator distinction without being too simplistic.

</review_packet>
