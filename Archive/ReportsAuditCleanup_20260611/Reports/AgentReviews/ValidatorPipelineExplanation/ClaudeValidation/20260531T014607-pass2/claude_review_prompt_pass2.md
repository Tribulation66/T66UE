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
- Packet path: C:\UE\T66\Reports\AgentReviews\ValidatorPipelineExplanation\answer_packet_v2.md
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
- `OPERATOR_VALIDATOR_PROTOCOL.md` has a special Claude-Operator/Codex-Validator flow where Codex approval happens before Claude performs mutating work, then Codex validates the actual changed files/proof after work.
- `OPERATOR_VALIDATOR_PROTOCOL.md` says decision gates stop all further work while waiting for the user.
- `Scripts\Invoke-ClaudePlanReview.ps1` supports `-ReviewDepth targeted|deepened` and `-RiskReview` for Claude validation.

Answer draft:

The setup has two layers:

1. Who does the work: Operator.
2. Who reviews/gates the work: Validator.

Current state:
- Operator: Codex.
- Validator: Claude.

That means Codex does the main investigation/planning/implementation work, then Claude reviews the packet/result. Claude is not acting as a second worker in validator mode; it is a read-only reviewer.

The pipeline starts with a common intake:

1. Define the task contract and read role state.
   The task is reduced to: working task, operator, validator, scope, stop condition. The repo role file is read so we know who is Operator and who is Validator.

2. Classify the request.
   The request is categorized as one or more of:
   - simple answer;
   - read-only investigation;
   - implementation/change;
   - workflow/process change;
   - proof-bearing work;
   - broad multi-phase work;
   - process-governed work such as UI fidelity, VFX, import, release, or staged build;
   - decision-gated work where only the user can choose.

3. Decide the validation process class.
   Quick validation is for low-risk answers.
   Full validation is for changes, durable recommendations, workflow rules, Unreal/editor work, releases, or anything where a wrong answer causes meaningful rework.

4. Decide the validator review depth.
   Targeted review means: check the packet, anchors, scope, instructions, and verification.
   Deepened review means: targeted review plus risk/oversight thinking: hidden coupling, stale evidence, cleanup/deletion risk, migration risk, asset/data reload risk, weak verification, rollback gaps, and assumptions.

5. Decide whether the work must be phased.
   If the task is broad, it is split into bounded phases. Each phase gets its own packet, review, approval/proof path, and completion evidence.

6. Decide whether proof-bearing routing applies.
   Proof-bearing means the task needs produced evidence, not just reasoning: build logs, commandlet output, screenshots, editor captures, gameplay proof, or visual proof.
   A read-only plan can scope proof-bearing work, but it cannot count as the proof. Current proof must be produced through the approved proof path.

7. Check for user-only decisions.
   If the Validator returns `NEEDS_HUMAN_DECISION`, or if Codex finds a real product/risk/scope choice only the user can make, work stops. No more polishing, no more review reruns, no adjacent work until the user answers.

Current pipeline: Codex Operator, Claude Validator.

1. Codex performs the Operator work.
   Codex reads the relevant live files, folder instructions, pending issues, and process docs. Codex creates the plan/report/change packet and, when implementation is in scope, performs the approved work.

2. Codex prepares the packet.
   The packet must include the task, roles, constraints, out-of-scope items, instructions read, live findings/evidence, process gates, proposed change approach, verification plan, token routing, caveats, and decisions the Validator should challenge.

3. Claude validator preflight happens.
   Before Claude runs, the Anthropic API key guard is checked. Claude validation must use the local Claude Code subscription CLI, not Anthropic API billing.

4. Claude runs Packet Completeness Gate.
   Claude first asks: is this packet complete enough to review?
   If not, verdict is `REVISE`. Claude does not fill in the missing research.

5. Claude reviews the packet.
   In targeted mode, Claude checks anchors, contradictions, scope, instructions, and verification adequacy.
   In deepened mode, Claude also looks for oversights: missed coupling, stale evidence, unsafe cleanup, weak proof, rollback holes, and assumptions that could break the plan.

6. Claude returns a verdict.
   `APPROVE`: Codex can proceed/report under that scope.
   `REVISE`: Codex must fix the packet/work and rerun validation.
   `NEEDS_HUMAN_DECISION`: stop and ask the user once.
   `BLOCK`: hard blocker or unsafe/unavailable prerequisite.

7. Codex finalizes.
   Codex remains responsible for the final answer. Codex reports what changed or what was found, what verification ran, what was skipped, and token spend.

Flipped pipeline: Claude Operator, Codex Validator.

This is different because Claude is now the worker.

1. Codex forwards the task to Claude with the task contract and repo rules.
   Codex should avoid doing a duplicate deep plan because Claude is supposed to own the expensive operator work.

2. Claude produces an Operator Change Request or Operator Packet.
   This is not a greenlight. It is Claude's proposed work scope.

3. Codex runs the completeness and approval check before mutating work.
   Codex checks whether Claude's requested file scope, tool surface, verification plan, process rules, and rollback are acceptable.

4. Codex writes a `Codex Approval: APPROVE` artifact if the scope is safe.
   Without that approval artifact, Claude FullOperator mutating work should not run.

5. Claude performs the approved work.
   For real implementation/proof, Claude uses the FullOperator helper/tool profile. This is where Claude may edit files, run commands, or produce proof, but only inside the approved scope.

6. Codex validates the actual workspace.
   Codex checks the real changed files, logs, captures, command output, and scope. Claude-produced proof is evidence, not final acceptance.

7. Codex returns the final verdict/report.
   If the work is wrong or incomplete, Codex routes it back to Claude as `REVISE` unless a small bounded Codex fix is safer and clearly recorded.

The key difference:

- Claude as Validator reviews Codex's packet. It is mostly text/packet review.
- Codex as Validator checks Claude's actual work in the live workspace. It is file/proof/evidence validation.

So the pipeline is not just "one model checks the other." It answers several routing questions:

- Who is Operator?
- Who is Validator?
- Is the task quick or full validation?
- Is the review targeted or deepened?
- Is this broad enough to split into phases?
- Is it proof-bearing?
- Is a human decision required?
- Is Codex approval required before Claude can mutate files?
- Did the Validator approve, request revision, ask for a human decision, or block?

For the large game-change prompts you are about to send, the likely default category is:
- Codex Operator, Claude Validator.
- Full validation.
- Deepened risk-focused review.
- Phase-bounded implementation.
- Current verification required for any compile/run/capture/proof claim.
- User decision gate whenever design direction, deletion aggressiveness, or risk acceptance is genuinely yours to choose.

ExpectedValidatorDepth: deepened
ValidatorBudgetHint: Check whether this separates the current Codex-Operator/Claude-Validator pipeline from the flipped Claude-Operator/Codex-Validator pipeline, and whether it covers decisions/categories accurately.

</review_packet>
