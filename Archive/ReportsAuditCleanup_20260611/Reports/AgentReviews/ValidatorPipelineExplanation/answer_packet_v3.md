Working task:
Operator: Codex
Validator: Claude
Scope: Explain the actual T66 validation process and pipeline in detail, including decisions and categorization. No code/config/content changes.
Stop condition: Deliver a detailed plain-language breakdown after validation.

Validation depth: deepened, because the user explicitly rejected the simplified answer and asked for the actual process pipeline.

Applicable live findings:
- `.t66\operator-state.json` currently sets `operator=Codex` and `validator=Claude`.
- `OPERATOR_VALIDATOR_PROTOCOL.md` says the pipeline exists to weight authoring work toward the Operator, roughly 70-80% Operator and 20-30% Validator/Finisher.
- `OPERATOR_VALIDATOR_PROTOCOL.md` separates validation process class (`quick/full`) from review depth (`targeted/deepened`).
- `OPERATOR_VALIDATOR_PROTOCOL.md` says broad work is phase-bounded.
- `OPERATOR_VALIDATOR_PROTOCOL.md` says proof-bearing work routes to FullOperator.
- `OPERATOR_VALIDATOR_PROTOCOL.md` requires Packet Completeness Gate before validator discovery.
- `OPERATOR_VALIDATOR_PROTOCOL.md` says decision gates stop all further work while waiting for the user.
- `OPERATOR_VALIDATOR_PROTOCOL.md` defines verdicts: APPROVE, REVISE, NEEDS_HUMAN_DECISION, BLOCK.
- `OPERATOR_VALIDATOR_PROTOCOL.md` has a Claude-Operator/Codex-Validator flow where Codex approval happens before Claude mutates files, then Codex validates actual changed files/proof after work.
- `Scripts\Invoke-ClaudePlanReview.ps1` supports `-ReviewDepth targeted|deepened` and `-RiskReview` for Claude validation.

Answer draft:

The validation system is not just "one model checks another." It is a routing pipeline designed to keep the main work with the Operator and keep the Validator focused on critique, gates, and proof review.

The target split is roughly:
- Operator: 70-80% of the real authoring work.
- Validator/Finisher: 20-30% for critique, approval gates, verification review, and final reporting.

That matters because the Validator is not supposed to quietly redo the Operator's job. If the packet is thin, the Validator should send it back as `REVISE`, not fill in the missing work.

Current state:
- Operator: Codex.
- Validator: Claude.

The actual categorization axes:

1. Role direction.
   Is Codex Operator and Claude Validator, or Claude Operator and Codex Validator?

2. Validation process class.
   `Quick` is for low-risk answers.
   `Full` is for edits, workflow changes, Unreal/editor work, releases, durable recommendations, or anything where a wrong answer causes meaningful rework.

3. Validator review depth.
   `Targeted` checks packet completeness, anchors, scope, instructions, and verification.
   `Deepened` is risk-focused: hidden coupling, stale evidence, cleanup/deletion risk, migration risk, data/asset reload risk, weak verification, rollback gaps, and assumptions.

4. Work shape.
   Single bounded task, or broad task that must be split into phases.

5. Proof requirement.
   Reasoning-only, or proof-bearing. Proof-bearing means the task needs produced evidence such as build logs, commandlet output, screenshots, gameplay proof, editor captures, or visual proof.

6. Process-governed work.
   Some work has mandatory procedures: UI fidelity, VFX, import, release, staged build, data reload, performance, etc.

7. Human decision gate.
   If only the user can decide scope, risk, deletion aggressiveness, or product direction, the process stops and asks once. No additional work continues until the user answers.

8. Tool/approval route.
   Read-only review, normal Codex work, or Claude FullOperator work. Claude FullOperator work requires Codex approval before it can mutate files.

Full pipeline for the current setup: Codex Operator, Claude Validator.

1. Intake and state.
   Codex frames the task contract and reads the role state. For this chat, that state says Codex operates and Claude validates.

2. Categorize the task.
   Codex chooses quick/full, targeted/deepened, single-phase/multi-phase, proof-bearing or not, process-governed or not, and whether a user decision is needed.

3. Codex does the Operator work.
   Codex reads live files, instructions, folder rules, pending issues, and process docs. Codex creates the packet, report, plan, or implementation result under the task scope.

4. Codex prepares the validation packet.
   The packet must include task, roles, constraints, out-of-scope items, instructions read, live findings, process gates, proposed approach, verification plan, token routing, caveats, and decisions the Validator should challenge.

5. Claude validator preflight runs.
   Before Claude runs, the Anthropic API key guard is checked. Claude validation must use the local Claude Code subscription CLI, not Anthropic API billing.

6. Claude runs the Packet Completeness Gate.
   If required sections are missing, Claude returns `REVISE`. Claude does not do the missing discovery for Codex.

7. Claude reviews.
   In targeted mode, Claude checks anchors, contradictions, scope, instructions, and verification.
   In deepened mode, Claude also checks hidden risk, weak proof, stale assumptions, cleanup/deletion consequences, rollback holes, and what weak implementation could appear to pass.

8. Claude returns a verdict.
   `APPROVE`: Codex may proceed/report under reviewed scope.
   `REVISE`: Codex must fix the packet/work and rerun validation.
   `NEEDS_HUMAN_DECISION`: stop, save/reference the decision, ask once, and do no other work.
   `BLOCK`: hard blocker, unsafe contradiction, unavailable prerequisite, or external state problem.

9. Codex finalizes.
   Codex remains responsible for the final user-facing answer. Codex reports verification performed, skipped verification, caveats, validator artifact when relevant, and token spend.

Full pipeline when flipped: Claude Operator, Codex Validator.

1. Codex receives the request, frames the contract, reads role state, and forwards the task/rules to Claude.
   Codex should not do a duplicate deep plan because Claude is now supposed to own the expensive Operator work.

2. Claude produces an Operator Change Request or Operator Packet.
   This is not a greenlight. It is Claude's proposed scope and plan.

3. Codex runs completeness and approval before Claude mutates files.
   Codex checks scope, files, tools, commands, process rules, verification, rollback, and excluded actions.

4. Codex writes a `Codex Approval: APPROVE` artifact if safe.
   Claude FullOperator work should not run without that approval artifact.

5. Claude performs the approved work.
   In FullOperator mode, Claude may edit files, run commands, or produce proof, but only inside the approved scope.

6. Codex validates the actual workspace.
   Codex checks changed files, logs, command output, captures, proof freshness, and scope. Claude-produced proof is evidence, not final acceptance.

7. Codex reports or returns the work.
   If validation passes, Codex reports to the user. If not, Codex returns `REVISE` to Claude unless a small bounded Codex fix is safer and clearly recorded.

The key difference:

- Claude as Validator reviews Codex's packet/result. It is mostly packet critique.
- Codex as Validator validates Claude's real workspace changes and proof. It is file/proof/evidence validation.

The hard stop conditions:
- Incomplete packet: `REVISE`.
- Unsafe or impossible plan: `BLOCK` or `REVISE`, depending on cause.
- User-only choice: `NEEDS_HUMAN_DECISION`, then all work stops until the user answers.
- Requested current proof missing: cannot substitute old evidence.

For the large game-change prompts coming next, the likely routing is:
- Codex Operator, Claude Validator.
- Full validation.
- Deepened/risk-focused review, normally `-ReviewDepth deepened -Effort high`.
- Phase-bounded work.
- Proof-bearing routing whenever builds, runtime captures, UI screenshots, gameplay proof, or editor evidence are required.
- Decision gate whenever design direction, deletion aggressiveness, or risk acceptance is yours to choose.

ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Check whether this states the purpose, real categorization axes, current pipeline, flipped pipeline, gate order, verdict meanings, and hard stop conditions accurately.
