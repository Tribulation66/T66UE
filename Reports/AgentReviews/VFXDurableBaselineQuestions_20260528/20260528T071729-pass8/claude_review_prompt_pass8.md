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

## Output Scope

Answer Pablo's process question and ask only the remaining gameplay/process decisions before implementation. Do not claim implementation approval. Do not edit production files from this packet.

## User Question

Before implementation, Pablo asks:

1. How can repeated questions due to blocking be avoided in the future?
2. Are there gameplay decision questions before the next pass?
3. The next pass should implement both "What Is Missing" and "Recommended Improvements".

## Repo Evidence Checked

- `AGENTS.md` requires Claude review for substantive answers and before implementation/commit, report artifacts under `Reports/`, and a greenlight bound to the current output scope.
- `Gameplay/GAMEPLAY_AGENTS.md` routes combat VFX work to `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md` routes review packets to `Reports/AgentReviews` and proof runs to `Reports/Proof/<Domain>/<TaskSlug>`.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md` does not exist and would be created in the implementation pass before sibling docs link to it.
- Existing mechanism packet naming pattern is `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`; proposed DOT/Pierce/Bounce names follow that pattern.
- Current review artifact home is `Reports/AgentReviews/VFXDurableBaselineQuestions_20260528/`.

## Coverage Table

The default is to produce one Claude-reviewed implementation plan that attempts to cover both lists. If Pablo answers in a way that makes the bundled scope materially larger than the current VFX-only baseline, the plan review is the explicit pause point where Codex must report the size/risk and ask whether to split before edits.

| Requested item | Default handling after user answers |
|---|---|
| VFX quick-start/index | implement |
| Reusable effect packet template | implement |
| Definition of Done matrix | implement |
| Generated CSV/DataTable/uasset source-of-truth policy | implement either repo-wide or combat-VFX-local based on answer |
| Stale `Hero1AxeVFXPlan.md` cleanup | implement surgically |
| Idol overlay architecture | implement as architecture doc, no idol VFX assets unless Pablo asks |
| Future DOT/Pierce/Bounce packets | implement infrastructure only unless Pablo asks for real effects now |
| Generalized VFX binding validator | implement |
| Normal player-facing proof | implement literal UI or production-path automation based on answer |
| Automated best-frame selection | implement; exact heuristic specified in the implementation plan |
| Local VFX-only commit | implement after Claude plan review, validation, staged manifest, Claude staged-diff review |

## Proposed Answer

The repeated-question loop happened because I let a temporary decision gate become the function-created goal. When that goal was marked blocked, the app resumed the same blocked context several times, and I repeated the full question block.

Future prevention:

1. Make the working goal the full end state, not the temporary question gate.
2. Ask a decision block once and save it at `Reports/AgentReviews/<TaskSlug>/decision_block.md`.
3. On continuations with no new user input, check that decision artifact first and reference it instead of reprinting the questions.
4. Use `blocked` only for true impasses where proceeding would change gameplay behavior, repo policy, commit scope, or irreversible assets with no safe default.
5. If a block state is unavoidable, keep it short, point to the decision artifact, and do not restate the whole list.

For the next pass, I will not silently split your requested "What Is Missing + Recommended Improvements" scope. After you answer the questions below, the next step is a Claude-reviewed implementation plan. That plan is the checkpoint where I must tell you if your answers made the single pass too large or risky before any edits begin.

There are three direction questions:

1. **DOT/Pierce/Bounce scope:** Choose one.
   - A. Infrastructure only in this pass: packets, binding schema/rules, validator coverage, and proof route/templates; real DOT/Pierce/Bounce VFX later through their own reference/mockup/effect-packet cycles. Recommended.
   - B. Infrastructure plus the three real DOT/Pierce/Bounce VFX implementations in this pass. This is the literal larger scope if you meant "future" to mean build them now.

2. **Normal player-facing proof:** Choose one.
   - A. Literal Weapon Altar UI-click proof in this pass.
   - B. Production-path automation proof first. Recommended. It must use production weapon selection, RunState inventory/item data, stat recompute, combat fire, VFX binding lookup, and damage paths. It may automate setup/camera/capture, but may not inject fake stats, bypass inventory, or spawn VFX manually.

3. **Generated asset policy scope:** Choose one.
   - A. Truly repo-wide generated CSV/DataTable/uasset source-of-truth policy now. This best matches your wording, but it requires inspecting representative non-combat generated-data flows and makes the pass larger.
   - B. Combat-VFX-local policy now, with repo-wide policy explicitly marked out of scope. Recommended for keeping this pass focused, but it is a deliberate narrowing from your repo-wide wording.

For smaller structure choices, I will use defaults unless you object:

- DoD matrix as sibling `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, linked from the new `Gameplay/Combat/VFX_PROCESS_INDEX.md`.
- Stale `Hero1AxeVFXPlan.md` cleanup is surgical: no deletion of historical decisions, no rewording preserved sections, current-status links added, and older static-crescent wording preserved verbatim under one `## Superseded` section.
- Idol overlay work is architecture only and general across heroes/weapons; no idol VFX assets or active idol rows in this pass.
- Best-frame selector uses logged fire time plus saturated-pixel/non-background activity and effect bounding-box area, with manual override; the exact thresholds belong in the implementation plan for Claude review.
- Generalized validator treats rows as active only when present in `Content/Data/CombatVFXBindings.csv` or `DT_CombatVFXBindings.uasset` with a concrete Niagara system and not marked deferred/scaffold-only by the effect packet.
- Local commit only, no push. Before commit: Claude-reviewed implementation plan, validation, staged-file manifest under `Reports/`, and Claude staged-diff review. I stop and report the out-of-scope file/hunk if anything outside the approved VFX scope enters the staged set.

Already answered and not to re-ask unless scope changes:

- Use `VFX_PROCESS_INDEX.md`.
- Include root `AGENTS.md`.
- Keep durable proof/handoff docs and exclude/delete transient review attempt files.
- Commit generated/runtime VFX data/assets.
- Commit locally only.
- Do not push.

## Reviewer Request

Review this revised answer before it is given to Pablo. Check that it honors the requested one-pass baseline by default, flags any recommended narrowing explicitly, includes the Claude staged-diff checkpoint before commit, and asks only decisions that affect scope/behavior/policy. First non-empty line must be exactly one of `Verdict: APPROVE`, `Verdict: REVISE`, or `Verdict: BLOCK`.

</review_packet>
