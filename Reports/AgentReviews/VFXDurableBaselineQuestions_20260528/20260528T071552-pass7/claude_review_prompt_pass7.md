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
- `Gameplay/Combat/VFX_PROCESS_INDEX.md` does not exist.
- Existing mechanism packet naming pattern is `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`; proposed DOT/Pierce/Bounce names follow that pattern.

## Coverage Table

The default is one next implementation pass that covers both lists. Any split would need Pablo's explicit approval.

| Requested item | Default handling in next pass |
|---|---|
| VFX quick-start/index | implement |
| Reusable effect packet template | implement |
| Definition of Done matrix | implement |
| Generated CSV/DataTable/uasset source-of-truth policy | implement, pending repo-wide scope answer |
| Stale `Hero1AxeVFXPlan.md` cleanup | implement surgically |
| Idol overlay architecture | implement as architecture doc, no idol VFX assets unless Pablo asks |
| Future DOT/Pierce/Bounce packets | implement, pending "future" meaning answer |
| Generalized VFX binding validator | implement |
| Normal player-facing proof | implement, pending proof-mode answer |
| Automated best-frame selection | implement |
| Local VFX-only commit | implement after Claude plan review, validation, staged manifest, Claude staged-diff review |

## Proposed Answer

The repeated-question loop happened because I let a temporary decision gate become the function-created goal. When that goal was marked blocked, the app resumed the same blocked context several times, and I repeated the full question block.

Future prevention:

1. Make the working goal the full end state, not the temporary question gate.
2. Ask a decision block once, save the decision block as a report artifact, and reference that artifact on continuations instead of reprinting the questions.
3. Use `blocked` only for true impasses where proceeding would change gameplay behavior, repo policy, commit scope, or irreversible assets with no safe default.
4. If a block state is unavoidable, keep it short and point to the existing decision artifact.

For the next pass, I will not silently split your requested "What Is Missing + Recommended Improvements" scope. The default is one implementation pass that covers both lists, followed by a local VFX-only commit after the required review gates.

There are only three direction questions I should ask before that pass:

1. **DOT/Pierce/Bounce meaning:** When you say "future Hero 1 DOT/Pierce/Bounce packets, bindings, and proof," do you mean infrastructure for those future effects, or actual DOT/Pierce/Bounce VFX implementations now?
   - Recommended: infrastructure now: packets, binding schema/rules, validator coverage, and proof route/templates. Real DOT/Pierce/Bounce VFX should each get the full reference/mockup/effect-packet cycle later. This is a scope-narrowing recommendation if you meant actual effects now, so confirm it.

2. **Normal player-facing proof:** Do you require literal Weapon Altar UI-click proof in this pass, or is production-path automation acceptable?
   - Recommended: production-path automation first. It must use production weapon selection, RunState inventory/item data, stat recompute, combat fire, VFX binding lookup, and damage paths. It may automate setup/camera/capture, but may not inject fake stats, bypass inventory, or spawn VFX manually. If you meant literal UI clicking, I need that direction.

3. **Generated asset policy scope:** For "repo-wide generated CSV/DataTable/uasset source-of-truth policy," should I truly inspect and write policy across the repo now, or keep the implemented policy combat-VFX-local and explicitly mark repo-wide policy out of scope?
   - Recommended: truly repo-wide policy now, but policy-only: inspect representative existing generated CSV/DataTable/uasset flows and write the source-of-truth rules without changing unrelated systems.

For the smaller structure choices, I will use defaults unless you object:

- DoD matrix as sibling `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, linked from `Gameplay/Combat/VFX_PROCESS_INDEX.md`.
- Stale `Hero1AxeVFXPlan.md` cleanup is surgical: current-status links plus one `## Superseded` section preserving older static-crescent wording verbatim.
- Idol overlay work is architecture only and general across heroes/weapons; no idol VFX assets or active idol rows in this pass.
- Best-frame selector uses logged fire time plus saturated-pixel/non-background activity and effect bounding-box area, with manual override.
- Generalized validator treats rows as active only when present in `Content/Data/CombatVFXBindings.csv` or `DT_CombatVFXBindings.uasset` with a concrete Niagara system and not marked deferred/scaffold-only by the effect packet.
- Local commit only, no push. Before commit: Claude-reviewed implementation plan, validation, staged-file manifest under `Reports/`, and Claude staged-diff review. I stop if out-of-scope files or hunks enter the staged set.

Already answered and not to re-ask unless scope changes:

- Use `VFX_PROCESS_INDEX.md`.
- Include root `AGENTS.md`.
- Keep durable proof/handoff docs and exclude/delete transient review attempt files.
- Commit generated/runtime VFX data/assets.
- Commit locally only.
- Do not push.

## Reviewer Request

Review this revised answer before it is given to Pablo. Check that it honors the requested one-pass baseline by default, flags any recommended narrowing explicitly, includes the Claude staged-diff checkpoint before commit, and asks only decisions that affect scope/behavior/policy. First non-empty line must be exactly `Verdict: APPROVE` or `Verdict: REVISE`.

</review_packet>
