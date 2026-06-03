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

The function-created goal is still marked blocked and this environment has no unblock control. Replacement working goal stated in conversation: implement and locally commit a durable VFX-only baseline that includes the missing process infrastructure and approved recommended improvements, after resolving any gameplay/process direction questions up front.

## User Question

Before implementation, Pablo asks:

1. How can repeated questions due to blocking be avoided in the future?
2. Are there gameplay decision questions before the next pass?
3. The next pass should implement both "What Is Missing" and "Recommended Improvements":
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

## Repo Evidence Checked

- `AGENTS.md` requires full-goal process fidelity, Claude review, no silent process swaps, reports under `Reports/`, and careful VFX-only commits.
- `Gameplay/GAMEPLAY_AGENTS.md` routes combat VFX to `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md` permits agent review/proof packets under `Reports/AgentReviews` and `Reports/Proof`.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md` does not exist yet.
- Existing VFX docs include `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, `Gameplay/Combat/Hero1AxeVFXPlan.md`, `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`, `Gameplay/Combat/CombatVFXInfrastructureInventory.md`, and `Gameplay/Combat/pending_issues_Combat.md`.
- Existing validators/scripts include `Scripts/ValidateCombatVFXProductionBindings.py`, `Scripts/SetupCombatVFXBindingsDataTable.py`, `Scripts/BuildT66VideoEvidenceBundle.py`, `Scripts/CaptureT66GameplayVideo.ps1`, and `Scripts/RunHero1AxeAOEVFXBindingProof.ps1`.

## Proposed Answer

The repeated-question issue came from the goal-continuation/blocking mechanism, but the root cause was how I used it: I narrowed the active goal around the decision gate instead of the full implementation end state. After the goal was marked blocked, the app resumed the same blocked context several times, and I repeated the same question block instead of keeping a short "still waiting on the prior gate" status.

I cannot disable that app/tool behavior from inside the thread. The fix I can enforce is behavioral:

1. Set the working goal to the full end state, not to a temporary question gate.
2. Ask decision gates once, with recommended defaults.
3. On automatic continuations with no new user input, do not reprint the full gate; refer back to the existing decision block.
4. Reserve `blocked` for true impasses where implementation would change gameplay behavior, repo policy, commit scope, or irreversible assets and no safe default exists.
5. If the tool policy still requires a blocked state, keep the blocked response short and do not restart the same question list.

There are still gameplay/process decisions before the full requested pass. Defaults below assume Pablo wants the recommended split; if he says "one bundle", the same choices apply with higher review/commit risk.

1. **Pass split:** Do you want the full work split into natural chunks?
   - Recommended: yes.
   - Pass A now: VFX process index, effect packet template, DoD/status matrix, combat-VFX generated asset policy, stale `Hero1AxeVFXPlan.md` cleanup, idol overlay architecture doc, and DOT/Pierce/Bounce scaffold packets.
   - Pass B next: generalized validator, automated best-frame selector, and normal-flow proof mode.
   - Pass C later: actual DOT/Pierce/Bounce VFX implementations and proof.

2. **DOT/Pierce/Bounce scope for this pass:** Should Pass A create scaffolding only?
   - Recommended: yes. Create packet templates/packet stubs and future-proof doc routes only; do not create active production binding rows or gameplay implementations for DOT/Pierce/Bounce yet.
   - Proposed packet paths: `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`, `Gameplay/Combat/Hero1AxePierceMechanismPacket.md`, and `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`.

3. **Normal player-facing proof definition:** Is a non-cheat automation proof acceptable before literal UI-click proof?
   - Recommended: yes, but this is a sequencing decision. If "player-facing proof" must literally click through the Weapon Altar UI this pass, say so.
   - Non-cheat automation means it may automate setup/camera/capture, but must use production weapon selection, RunState inventory/item data, stat recompute, combat fire, VFX binding lookup, and damage paths. It may not inject fake stat values, bypass inventory, or spawn the VFX manually.

4. **Idol overlay scope:** Should the idol overlay work be architecture only and general from day one?
   - Recommended: yes. Write `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` now with binding/layer/parameter/damage-authority rules. Do not build idol VFX assets or active idol binding rows in this pass.

5. **Best-frame selection heuristic:** Is a simple first version acceptable?
   - Recommended: yes. Use logged fire time plus saturated-pixel/non-background activity and effect bounding-box area, with manual override. Tie-break by earliest active frame for `start`, midpoint for `mid`, peak activity for `impact`, and first low-activity frame after impact for `dissipate`.

6. **Generated asset policy scope:** Should source-of-truth policy be combat-VFX-local now?
   - Recommended: yes. Write `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md` for Combat VFX CSV/DataTable/uasset ownership now, and explicitly defer repo-wide generated asset policy to a broader pass.

7. **Stale `Hero1AxeVFXPlan.md` cleanup:** Should cleanup be surgical?
   - Recommended: yes. Add a current-status banner, link to current packet/proof/inventory docs, and move older static-crescent wording under a historical/superseded context. Do not rewrite the whole plan.

8. **Definition of Done ownership:** Should the DoD matrix be a checklist/status router, not a new validator?
   - Recommended: yes. Put it in `Gameplay/Combat/VFX_PROCESS_INDEX.md` as an index of existing gates: source/reference accepted, effect packet present, PPF/artifact/mechanism gates present, mockup approved when active, lab assets validate, editor-isolation capture passes, gameplay capture passes, hitbox/damage proof passes, production binding validates, normal-flow proof passes, and commit durability.

9. **Generalized validator active-row rule:** Is this definition acceptable for Pass B?
   - Recommended: yes. Active production rows are rows present in `Content/Data/CombatVFXBindings.csv` or `DT_CombatVFXBindings.uasset` with a concrete Niagara system and not marked deferred/scaffold-only by the effect packet. Scaffold packet docs alone do not create active binding requirements.

10. **Commit gate:** Should I keep your earlier answer: local commit only, no push?
   - Recommended: yes. I will commit locally after validation and a self-inspected VFX-only staged list. I will stop instead of committing only if an out-of-scope file or hunk enters the staged set, because that would violate the VFX-only scope.

Already answered by Pablo and not to be re-asked unless scope changes:

- Use `VFX_PROCESS_INDEX.md`.
- Include root `AGENTS.md`.
- Keep durable proof/handoff docs and exclude/delete transient review attempt files.
- Commit generated/runtime VFX data/assets.
- Commit locally only.
- Do not push.

## Reviewer Request

Review this answer before it is given to Pablo. Identify any missing gameplay decision question, unsafe claim, contradiction with the already-answered commit policy, or risk that would cause another repeated-question loop. First non-empty line must be exactly `Verdict: APPROVE` or `Verdict: REVISE`.

</review_packet>
