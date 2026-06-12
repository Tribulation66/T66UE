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
- Packet path: C:\UE\T66\Reports\AgentReviews\ValidatorPipelineExplanation\answer_packet_v6.md
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
- `OPERATOR_VALIDATOR_PROTOCOL.md:18-19` gives the target split: Operator about 70-80%, Validator/Finisher about 20-30%.
- `OPERATOR_VALIDATOR_PROTOCOL.md:33-37` separates validation process class (`quick/full`) from Validator review depth (`targeted/deepened`).
- `OPERATOR_VALIDATOR_PROTOCOL.md:191-215` defines phase-bounded broad work.
- `OPERATOR_VALIDATOR_PROTOCOL.md:233-266` defines proof-bearing routing; lines 253-255 name `Scripts\Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator` with a valid Codex approval artifact.
- `OPERATOR_VALIDATOR_PROTOCOL.md:450-474` defines the Packet Completeness Gate.
- `OPERATOR_VALIDATOR_PROTOCOL.md:578-596` defines the Decision Gate Stop Rule.
- `OPERATOR_VALIDATOR_PROTOCOL.md:554-573` defines verdicts: APPROVE, REVISE, NEEDS_HUMAN_DECISION, BLOCK.
- `OPERATOR_VALIDATOR_PROTOCOL.md:610-621` defines the Claude-Operator/Codex-Validator flow.
- `AGENTS.md:169` says direct user commands `Make Claude operator` and `Make Codex operator` are project-global operator-switch commands.
- `AGENTS.md:170-171` requires checking `ANTHROPIC_API_KEY` before Claude and using the local Claude Code CLI.
- `Scripts\Invoke-ClaudePlanReview.ps1:10-12` says it refuses API-key billing by default, and `Scripts\Invoke-ClaudePlanReview.ps1:102-119` implements the key check.
- `Scripts\Invoke-ClaudePlanReview.ps1:46-49` supports `-ReviewDepth targeted|deepened` and `-RiskReview`.

Answer draft:

The validation system is a routing pipeline. Its purpose is to keep the main work with the Operator and keep the Validator focused on critique, gates, and proof review.

The target split is roughly:
- Operator: 70-80% of the authoring work.
- Validator/Finisher: 20-30% for critique, approval gates, verification review, and final reporting.

So the Validator is not supposed to quietly redo the Operator's job. If the packet is thin, the Validator sends it back as `REVISE`.

Current state:
- Operator: Codex.
- Validator: Claude.

The actual categorization axes:

1. Role direction.
   Current default is Codex Operator / Claude Validator. The flipped path is Claude Operator / Codex Validator.

2. Validation process class.
   `Quick` is for low-risk answers.
   `Full` is for edits, workflow changes, Unreal/editor work, releases, durable recommendations, or high-rework-risk answers.

3. Validator review depth.
   `Targeted` checks packet completeness, anchors, scope, instructions, and verification.
   `Deepened` is risk-focused: hidden coupling, stale evidence, cleanup/deletion risk, migration risk, data/asset reload risk, weak verification, rollback gaps, and assumptions.

4. Work shape.
   Either one bounded task, or a broad task split into phases.

5. Proof requirement.
   Reasoning-only, or proof-bearing. Proof-bearing means acceptance needs produced evidence: build logs, commandlet output, screenshots, gameplay proof, editor captures, or visual proof.

6. Process-governed work.
   Some categories have mandatory procedures: UI fidelity, VFX, import, release, staged build, data reload, performance, etc.

7. Human decision gate.
   If only the user can decide scope, risk, deletion aggressiveness, or product direction, the process stops and asks once. No other work continues while waiting.

8. Tool/approval route.
   Normal Codex work, read-only Claude review, read-only Claude operator planning, or Claude FullOperator proof/implementation. In this repo, the proof-producing FullOperator path is `Scripts\Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator`, and it requires Codex approval first.

Current pipeline: Codex Operator, Claude Validator.

1. Intake and state.
   Codex frames the task contract and reads role state. In this chat, that state says Codex operates and Claude validates.

2. Categorization.
   Codex decides quick/full, targeted/deepened, single-phase/multi-phase, proof-bearing or not, process-governed or not, and whether a user decision is needed.

3. Codex does the Operator work.
   Codex reads live files, folder rules, pending issues, and process docs. Codex produces the packet, report, plan, or implementation result under the task scope.

4. Codex prepares the validation packet.
   The packet records task, roles, constraints, out-of-scope items, instructions read, live findings, process gates, approach, verification plan, token routing, caveats, and decisions the Validator should challenge.

5. Claude validator preflight.
   Before Claude runs, Codex checks that `ANTHROPIC_API_KEY` is not set. In the normal workflow, Claude validation uses the local Claude Code subscription CLI, not API billing. The helper has an explicit override switch, but the default T66 process stops and asks rather than silently using API billing.

6. Claude runs Packet Completeness Gate.
   If required sections are missing, Claude returns `REVISE`. Claude does not fill in missing research.

7. Claude reviews.
   Targeted mode checks anchors, contradictions, scope, instructions, and verification.
   Deepened mode also checks risk: hidden coupling, stale assumptions, unsafe cleanup, weak proof, rollback holes, and weak implementations that could appear to pass.

8. Claude returns a verdict.
   `APPROVE`: Codex may proceed/report under reviewed scope.
   `REVISE`: Codex fixes the packet/work and reruns validation.
   `NEEDS_HUMAN_DECISION`: stop, save/reference the decision, ask once, and do no other work.
   `BLOCK`: hard blocker, unsafe contradiction, unavailable prerequisite, or external-state problem.

9. Codex finalizes.
   Codex remains responsible for the final answer: what was done/found, verification performed, skipped verification, caveats, validator artifact when relevant, and token spend.

Important proof-bearing nuance:
The current global role state can remain Codex Operator / Claude Validator for planning and normal work. If a phase needs produced proof, that proof-producing phase has its own approved tool route: `Invoke-ClaudeDirectRead.ps1 -ToolProfile FullOperator` with a Codex approval artifact. The protocol does not say this automatically rewrites `.t66\operator-state.json`. Project-global role switches are separate user commands like "Make Claude operator" or "Make Codex operator." So for now I treat proof routing as a per-phase tool/approval route, not a global operator-state change, unless you explicitly change the operator.

Flipped pipeline: Claude Operator, Codex Validator.

1. Codex receives the request, frames the contract, reads role state, and forwards the task/rules to Claude.
   Codex avoids doing a duplicate deep plan because Claude is now supposed to own the expensive Operator work.

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

Hard stop conditions:
- Incomplete packet: `REVISE`.
- Unsafe or impossible plan: `BLOCK` or `REVISE`, depending on cause.
- User-only choice: `NEEDS_HUMAN_DECISION`, then all work stops until the user answers.
- Requested current proof missing: old evidence cannot be substituted.
- Claude API billing risk: if `ANTHROPIC_API_KEY` is set, the default process stops and asks before running Claude.

For the large game-change prompts coming next, the likely routing is:
- Planning/report phases: Codex Operator, Claude Validator.
- Validation class: full.
- Review depth: deepened/risk-focused, normally `-ReviewDepth deepened -Effort high`.
- Work shape: phase-bounded.
- Proof-producing phases: route to Claude FullOperator after Codex approval when builds, runtime captures, UI screenshots, gameplay proof, or editor evidence are required.
- Human decision gate whenever design direction, deletion aggressiveness, or risk acceptance is yours to choose.

ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Check whether this anchors the proof-route and state-route distinction, reflects the API-key override nuance, and accurately describes decisions/categories.

</review_packet>
