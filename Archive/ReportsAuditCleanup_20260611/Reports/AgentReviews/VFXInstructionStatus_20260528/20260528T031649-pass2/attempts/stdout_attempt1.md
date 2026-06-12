Verdict: REVISE

## Blockers

None. The packet is read-only and explicitly gates all implementation on Pablo's go-ahead.

## Major Issues

- **Scope mismatch with the user's actual question.** The user asked about "the instruction files" broadly — current processes/structures, what they cover, gaps, improvements. The packet unilaterally narrows to combat VFX based on the current working goal. That narrowing is a judgment call Pablo should confirm, not a settled assumption. The final answer must either (a) confirm the narrowed scope with Pablo first, or (b) include at least a brief survey of the other instruction surfaces (Movement, Stats, Traps, World, UI, packaging, `Scripts/AGENTS.md`, `Reports/AGENTS.md` family, `Gameplay/GAMEPLAY_AGENTS.md` peers) so Pablo can decide whether the deeper combat-VFX dive answers what he meant.
- **Recommendation #3 (Definition of Done matrix) overlaps with existing close templates.** `CombatVFXAuthoringProcedure.md` already documents PPF close, mechanism close, artifact parity gate, and capture gates. The packet should explicitly state whether the DoD matrix replaces, extends, or indexes those existing gates — otherwise it risks creating a parallel checklist that drifts.
- **Recommendation #1 (one-page VFX process index) needs a non-duplication clause.** The packet says "additive" but does not name the boundary against `Gameplay/GAMEPLAY_AGENTS.md` and `CombatVFXAuthoringProcedure.md`. Without a clear "router vs. quick-start vs. procedure" boundary, a new index file becomes a fourth thing to keep in sync.

## Minor Issues

- The "What The Instructions Cover" section interleaves AGENTS.md global rules with combat-VFX-specific procedure. Splitting global vs. combat-scoped coverage would let Pablo see which gaps are local to combat and which would propagate to any other system.
- Missing-area #11 (source-of-truth policy for generated CSV/uasset) is real but the packet does not note whether this is a combat-VFX-only concern or a repo-wide DataTable policy question (e.g., `Content/Data/DT_Weapons.uasset` is also modified in tree). Worth flagging as cross-cutting.
- Missing-area #10 names stale lines in `Hero1AxeVFXPlan.md` but does not state whether the stale language actively contradicts current production status or merely lags it. The remediation effort and risk differ between those two cases.
- The "Current End-To-End Process For Future VFX" 13-step list reads as derived/synthesized. The packet should make clear whether this is (a) literally documented somewhere or (b) Codex's reconstruction. If (b), that itself is a missing-area: no canonical end-to-end checklist exists.
- The narrow git status enumerates untracked files but does not separate "process docs" from "generated assets" from "tooling scripts." Pablo's decision on durability (#10) will differ per bucket — committing process docs is lower risk than committing generated uassets.
- Proposed final answer shape says "mention the Claude review artifact path once review is complete" but does not specify the path. Per `Reports/AGENTS.md` routing, the expected path is `Reports/AgentReviews/...` — the answer should name the actual artifact location.

## Clarifying Questions

- Did Pablo intend "the instruction files" to mean combat VFX only, or the full instruction surface? Confirm before delivering the narrowed answer.
- Should the proposed `VFX_PROCESS_INDEX.md` live under `Gameplay/Combat/` (combat-scoped) or `Gameplay/` (cross-family, anticipating Hero 2+)?
- Is the durability decision (#10) something Pablo wants Codex to surface as a single explicit choice (stage all / stage some / leave local), or does he want a per-file recommendation?

## Required Verification

- The packet's line-number citations (e.g., `Hero1AxeAOESlashMechanismPacket.md:564-567`, `:605-608`, `:625`; `CombatVFXInfrastructureInventory.md:255-277`, `:299`, `:313-315`) are not independently verified in this review. Codex should re-open each cited file and confirm the line ranges still match before publishing — line numbers drift on edits.
- Narrow git status snapshot must be re-run immediately before finalizing the answer; the listed untracked/modified set may have changed since the packet was assembled.
- Before recommending `ValidateCombatVFXProductionBindings.py` generalization (#4), confirm by reading the current script that it is in fact Hero 1 AOE-hardcoded and not already row-iterating.

## Rationale

The packet is well-structured, properly read-only, cites paths with line numbers, and correctly defers all change actions to Pablo's go-ahead. It satisfies AGENTS.md process discipline (live inspection, working-goal framing, Claude review path, no scope creep into Mini/minigames). The reason for REVISE rather than APPROVE is the scope mismatch: the user asked about instruction files broadly and the packet answers about combat VFX specifically without first confirming that narrowing. That is a clarifying-question gate that should close before the final answer is presented. The DoD/index recommendations also need tighter boundaries against existing docs to avoid creating parallel sources of truth — a known failure mode for instruction surfaces. None of the issues are unsafe; they are presentation and scope-alignment concerns that warrant one revision pass before this is safe to present as Codex's greenlit answer.

