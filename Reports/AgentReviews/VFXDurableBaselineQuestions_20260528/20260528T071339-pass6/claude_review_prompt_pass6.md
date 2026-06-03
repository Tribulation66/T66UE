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

Answer Pablo's process question and ask the remaining gameplay/process decisions before implementation. Do not claim implementation approval. Do not edit production files from this packet.

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

| Requested item | Proposed pass | Decision status |
|---|---:|---|
| VFX quick-start/index | A | ask whether Pass A runs as one doc chunk |
| Reusable effect packet template | A | ask whether Pass A runs as one doc chunk |
| Definition of Done matrix | A | ask file ownership: index section or sibling DoD doc |
| Generated CSV/DataTable/uasset policy | A | ask repo-wide now vs combat-VFX-local now |
| Stale `Hero1AxeVFXPlan.md` cleanup | A | ask surgical cleanup default |
| Idol overlay architecture | A | ask architecture-only and general scope |
| Future DOT/Pierce/Bounce packets | A | ask scaffolding-only vs implementation |
| Generalized VFX binding validator | B | ask active-row definition now; implement later |
| Normal player-facing proof | B | ask literal UI click vs production-path automation |
| Automated best-frame selection | B | ask heuristic now; implement later |
| DOT/Pierce/Bounce bindings and proof | C unless user overrides | ask whether to pull into this baseline |

## Proposed Answer

The repeated-question loop happened because I let a temporary decision gate become the function-created goal. When that goal was marked blocked, the app resumed the same blocked context several times, and I repeated the full question block.

Future prevention:

1. Make the working goal the full end state, not the temporary question gate.
2. Ask a decision block once, with recommended defaults.
3. On continuations with no new user input, reference the existing decision block instead of reprinting it.
4. Use `blocked` only for true impasses where proceeding would change gameplay behavior, repo policy, commit scope, or irreversible assets with no safe default.
5. If a block state is unavoidable, keep it short.

There are still decisions before implementation. If you answer "use recommended defaults", I can proceed without re-asking this block.

1. **Pass shape:** Should Pass A run as one large doc-only/process chunk, or split into two checkpoints?
   - Recommended: split Pass A into A1 and A2.
   - A1: `VFX_PROCESS_INDEX.md`, effect packet template, DoD ownership, generated-asset policy.
   - A2: stale `Hero1AxeVFXPlan.md` cleanup, idol overlay architecture, DOT/Pierce/Bounce scaffold packets.
   - Pass B later: generalized validator, automated best-frame selector, normal-flow proof mode.
   - Pass C later: actual DOT/Pierce/Bounce VFX implementations and proof.

2. **DOT/Pierce/Bounce scope:** Should this baseline create scaffolding only, or also implement the three real effects?
   - Recommended: scaffolding only. Use `Hero1AxeDOTMechanismPacket.md`, `Hero1AxePierceMechanismPacket.md`, and `Hero1AxeBounceMechanismPacket.md`, matching `Hero1AxeAOESlashMechanismPacket.md`. No active binding rows until each effect has a validated asset.

3. **Normal player-facing proof:** For Pass B, do you require literal Weapon Altar UI-click proof, or is production-path automation acceptable?
   - Recommended: production-path automation first, but this is a real deviation if you intended literal UI clicking now.
   - Production-path automation must use production weapon selection, RunState inventory/item data, stat recompute, combat fire, VFX binding lookup, and damage paths. It may automate setup/camera/capture, but may not inject fake stats, bypass inventory, or spawn VFX manually.

4. **Idol overlay scope:** Should this baseline write only the architecture, and make it general across heroes/weapons?
   - Recommended: yes. Create `CombatVFXIdolOverlayArchitecture.md` with binding/layer/parameter/damage-authority rules. No idol VFX assets or active idol rows yet.

5. **Best-frame selection:** Is this first heuristic acceptable for Pass B?
   - Recommended: yes. Use logged fire time plus saturated-pixel/non-background activity and effect bounding-box area, with manual override. Pick earliest active frame for `start`, midpoint for `mid`, peak activity for `impact`, and first low-activity frame after impact for `dissipate`. Decision captured now to avoid re-asking; implementation stays in Pass B.

6. **Generated asset policy scope:** Your list said repo-wide generated CSV/DataTable/uasset policy. Do you want that repo-wide policy now, or a combat-VFX-local policy now with repo-wide policy explicitly deferred?
   - Recommended: combat-VFX-local now unless you want this pass to touch broader repo policy. If repo-wide now, I will inspect other systems first before writing it.

7. **DoD file ownership:** Should the DoD matrix live inside `VFX_PROCESS_INDEX.md` or as a sibling file linked from the index?
   - Recommended: sibling `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, linked from `VFX_PROCESS_INDEX.md`, so the index remains a router.

8. **Stale `Hero1AxeVFXPlan.md` cleanup:** Surgical cleanup or fuller rewrite?
   - Recommended: surgical. Add current-status links and one `## Superseded` section at the bottom, preserving older static-crescent wording verbatim under it.

9. **Generalized validator active-row rule:** Is this rule acceptable for Pass B?
   - Recommended: yes. Active production rows are rows present in `Content/Data/CombatVFXBindings.csv` or `DT_CombatVFXBindings.uasset` with a concrete Niagara system and not marked deferred/scaffold-only by the effect packet. Scaffold docs alone do not create active binding requirements.

10. **Commit/review gate:** Keep your earlier instruction: local commit only, no push?
   - Recommended: yes. Each implementation pass gets Claude review of the plan before edits and Claude review of the staged diff before local commit. I will also write a staged-file manifest under `Reports/` before committing. I will stop instead of committing if the staged set includes out-of-scope files or hunks.

Already answered and not to re-ask unless scope changes:

- Use `VFX_PROCESS_INDEX.md`.
- Include root `AGENTS.md`.
- Keep durable proof/handoff docs and exclude/delete transient review attempt files.
- Commit generated/runtime VFX data/assets.
- Commit locally only.
- Do not push.

## Reviewer Request

Review this revised answer before it is given to Pablo. Check that it does not silently narrow Pablo's stated scope, does not claim implementation approval, includes the Claude staged-diff checkpoint before commit, and asks only decisions that affect scope/behavior/policy. First non-empty line must be exactly `Verdict: APPROVE` or `Verdict: REVISE`.

</review_packet>
