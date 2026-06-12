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

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\VFXDurableBaselineQuestions_20260528\answer_packet.md
- Output scope: review of the packet below only.

<review_packet>
# VFX Durable Baseline Question Answer Packet

## Working Goal

The previous function-created goal is blocked and this environment has no unblock control. Replacement working goal stated in conversation: implement and locally commit a durable VFX-only baseline that includes the missing process infrastructure and approved recommended improvements, after resolving any gameplay-direction questions up front.

## User Question

Before implementation, the user asks:

1. How can we avoid repeated questions due to blocking in the future?
2. Are there any gameplay-decision questions before the next pass?
3. The next pass should implement both "What Is Missing" and "Recommended Improvements", including:
   - VFX quick-start/index.
   - reusable per-effect packet template.
   - generalized production VFX binding validator.
   - normal player-facing proof that altar selection and item stats affect VFX through the real flow.
   - idol overlay architecture.
   - future Hero 1 DOT/Pierce/Bounce packets, bindings, and proof.
   - automated best-frame selection.
   - repo-wide generated CSV/DataTable/uasset source-of-truth policy.
   - stale language cleanup in `Hero1AxeVFXPlan.md`.
   - Definition of Done matrix.
   - local VFX-only durable commit.

## Proposed Answer

The repeated-question issue came from two mistakes:

1. The function-created goal was too narrow. It said the goal was to prepare the commit plan by asking upfront questions, while the actual user objective was to implement the durable VFX baseline and commit it. That made the decision gate look like the whole goal.
2. After the unanswered decision gate repeated through automatic goal continuations, the blocked-audit rule forced a blocked response. The result was technically consistent with the goal mechanism, but bad UX.

Going forward:

- I should set the function/conversation goal to the full desired end state, not to the temporary decision gate.
- If I need a decision, I should ask once, keep the goal active, and avoid reprinting the full question list on automatic continuations. If the thread resumes with no new user input, I should only say I am still waiting on the previously listed decisions, not restate everything.
- I should reserve `blocked` for a true impasse after the blocker repeats, and make the repeated-blocker message short.
- For multi-decision gates, I should ask in numbered form once and include recommended defaults, so the user can answer compactly.
- When the user answers, I should immediately carry those answers forward and not re-ask.

For repeated-question prevention, the structural fix is:

1. Set goals to the full requested end state, not to a temporary decision gate.
2. Keep decision gates in the plan/status, not as the whole goal.
3. If a decision is required, ask once with numbered choices and recommended defaults.
4. On automatic continuations with no new user input, do not reprint the full list; say only that the task is waiting on the previously listed decision block.
5. Do not call `blocked` just because a decision would be useful. A true impasse means implementation would change gameplay behavior, repo policy, commit scope, or irreversible assets and cannot be safely inferred from existing instructions.
6. If `blocked` is required by the tool policy, keep the blocked response short and point to the existing decision block rather than repeating it.

There are gameplay/process direction questions before the full requested pass. Ask these compactly:

1. **DOT/Pierce/Bounce scope:** Should this pass create only packets/binding/proof scaffolding for Hero 1 DOT/Pierce/Bounce, or should it also implement actual gameplay/VFX behavior for those three attacks?
   - Recommended: scaffolding only in this durability pass. Actual DOT/Pierce/Bounce implementations should be separate effect passes with their own source/mockup/packet/proof.

2. **Normal-flow proof definition:** For altar/item/stat proof, should the first pass drive the actual Weapon Altar UI click path, or is a non-cheat automation path that uses the same production subsystems acceptable?
   - Recommended: non-cheat automation first, UI-click proof later if needed. This should use real weapon selection APIs, real run-state item acquisition/stat recalculation, and real combat dispatch, but avoid proof-only deterministic item grants except in separate deterministic proof modes.

3. **Idol overlay scope:** Should the idol overlay architecture be Hero 1-only for now, or general from day one across heroes/weapons?
   - Recommended: architecture general from day one, implementation deferred. Write the binding/layer/parameter/damage-authority design now; do not build idol VFX assets in this pass.

4. **Best-frame selection heuristic:** Is a simple first version acceptable: pick frames by non-background/color activity around the logged fire-time window, keep manual override, and require contact-sheet review before handoff?
   - Recommended: yes. Do not attempt computer-vision scoring beyond activity/window heuristics yet.

5. **Generated asset policy scope:** Should the generated CSV/DataTable/uasset source-of-truth policy be written as combat-VFX-local first, or repo-wide?
   - Recommended: combat-VFX-local policy now, with a note that repo-wide DataTable policy needs a separate broader pass.

6. **Stale `Hero1AxeVFXPlan.md` cleanup:** Should cleanup be surgical status correction only, or a fuller rewrite?
   - Recommended: surgical correction only. Preserve historical prototype notes, but add current status and links so agents do not mistake old partial language for current truth.

7. **Commit gate:** Is the local VFX-only commit already authorized after implementation/validation, or should Codex stop at "ready to commit" and ask again?
   - Recommended: already authorized for this specific pass because Pablo answered "commit locally only for now"; still report exact staged file list before committing if a surprising file enters scope.

Default choices already answered by Pablo:

- Use `VFX_PROCESS_INDEX.md`.
- Include root `AGENTS.md`.
- Keep durable proof/handoff docs and exclude transient review attempts.
- Commit generated/runtime VFX data/assets.
- Commit locally only.
- Do not push.

The Definition of Done matrix must be implemented as an index of existing gates, not a new parallel process. It should include: source/reference accepted, effect packet present, PPF/artifact/mechanism gates present, mockup approved if active, lab assets validate, editor-isolation capture passes, gameplay capture passes, logical hitbox/damage proof passes, production binding validates, normal-flow proof passes, and commit durability.

## Reviewer Request

Review this revised answer before it is given to Pablo. Identify any missing gameplay decision question or unsafe claim. First non-empty line must be exactly `Verdict: APPROVE` or `Verdict: REVISE`.

</review_packet>
