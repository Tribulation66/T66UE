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
- Packet path: C:\UE\T66\Reports\AgentReviews\VFXInstructionStatus_20260528\answer_packet.md
- Output scope: review of the packet below only.

<review_packet>
# VFX Instruction Status Answer Review Packet

## Working Goal

Assess the current T66 VFX/instruction process structure, summarize what it covers and what is missing, and provide Codex plus Claude process-improvement recommendations.

## User Request

The user asked for the status of the instruction files: whether the current processes and structures are organized, what the instructions actually cover, what process information exists, what is missing, and whether Codex plus Claude can identify improvements.

## Applicable Repo Instructions

- `AGENTS.md` is the root process router. It requires live repo inspection, working-goal discipline, process fidelity, research-first replication for solved visual/VFX work, PPF/artifact parity/mechanism gates, Claude review, and reporting verification.
- Default scope excludes Mini/minigames. This status answer is scoped to combat VFX and related process docs only.
- `Gameplay/GAMEPLAY_AGENTS.md` routes combat/VFX work to `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md` routes this review packet under `Reports/AgentReviews`.

## Live Sources Inspected

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Reports/AGENTS.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md`
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/HANDOFF_NEXT_AGENT.md`
- Narrow git status over VFX/doc/script paths.

## Scope Assumption

This answer is scoped to the combat VFX instruction/process structure because the live conversation and current working goal are about Hero 1 axe AOE VFX, VFX infrastructure, capture, hitbox proof, and future VFX production. This is not a repo-wide audit of every instruction file under Movement, Stats, Traps, World Generation, UI, packaging, or other systems.

## Codex Assessment

### Organization Status

The instruction structure is conceptually organized, but not yet durable enough.

The live working tree has a clear layered structure:

1. `AGENTS.md`: global process contract and hard gates.
2. `Gameplay/GAMEPLAY_AGENTS.md`: gameplay folder router.
3. `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: generic combat VFX procedure.
4. `Gameplay/Combat/Hero1AxeVFXPlan.md`: Hero 1 axe family plan.
5. `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: per-effect packet for the current AOE slash.
6. `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: current infrastructure inventory.
7. `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/`: cleanup and next-agent handoff.
8. `pending_issues_*.md`: known gaps and deferred work.

The weakness is that many of the important process docs, scripts, data rows, and Niagara assets are still untracked in the working tree. They are organized locally, but a clean checkout would not yet reproduce the process. This should be treated as the biggest current status caveat.

Concrete narrow git status for this VFX/status scope currently includes:

- Modified: `AGENTS.md`
- Modified: `Gameplay/Combat/MASTER_COMBAT.md`
- Modified: `Scripts/pending_issues_Scripts.md`
- Untracked: `Content/Data/CombatVFXBindings.csv`
- Untracked: `Content/Data/DT_CombatVFXBindings.uasset`
- Untracked: `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`
- Untracked: `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- Untracked: `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- Untracked: `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- Untracked: `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md`
- Untracked: `Gameplay/Combat/Hero1AxeVFXPlan.md`
- Untracked: `Gameplay/Combat/pending_issues_Combat.md`
- Untracked: `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md`
- Untracked: `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/HANDOFF_NEXT_AGENT.md`
- Untracked: `Scripts/BuildT66VideoEvidenceBundle.py`
- Untracked: `Scripts/CaptureT66GameplayVideo.ps1`
- Untracked: `Scripts/CaptureT66NiagaraMRQIsolation.ps1`
- Untracked: `Scripts/SetupCombatVFXBindingsDataTable.py`
- Untracked: `Scripts/ValidateCombatVFXProductionBindings.py`

Staging, committing, editing, or deleting any of these files is not part of this answer. That requires Pablo's explicit go-ahead.

### What The Instructions Cover

The root process now covers:

- Goal creation and active-task discipline.
- Live repo first; do not answer from stale docs or memory.
- Claude review as the default cross-review path.
- Research-first replication for solved production categories.
- PPF check, artifact parity gate, mechanism manifest, anti-lookalike rule, PPF close, and mechanism close.
- Image generation through the account-backed `imagegen` path, not API-key scripts.
- Transcript handling: if a YouTube video is needed and a transcript is not already present, ask Pablo for the transcript. Do not revive local Whisper/caption extraction.
- Unreal-owned capture, screenshot, and video evidence rules.
- Niagara combat VFX constraints: primary silhouette must live in Niagara/material/renderer/emitter logic, not actor-side debug geometry or shortcuts.

Path support for this statement:

- `AGENTS.md:170` covers account-backed imagegen and prohibits `OPENAI_API_KEY` API scripts for this workflow.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md:53-61` covers transcript policy: no local transcript/caption/video-source extraction, ask Pablo if needed.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md:76` repeats the VFX mockup rule for separate local Codex CLI worker plus account-backed imagegen.

The generic VFX procedure covers:

- Source evidence intake and source-review workflow.
- Codex first-pass opinion before Claude review.
- Visual target/mockup gate with prompt, artifact, approval, decomposition, and feasibility notes.
- Same-view editor isolation gate using black background and locked view/crop, currently through `Scripts/CaptureT66NiagaraMRQIsolation.ps1`.
- Core construction workflow: carrier archetype assignment, staged checkpoints, mask/material manifests, shared aura material language, Niagara construction order, layer stack, parameter sweep evidence, known automation/editor pitfalls, and close templates.
- Required artifact/mechanism close discipline before claiming an effect is production-ready.

The Hero 1 axe plan covers:

- The four intended Hero 1 axe weapon attacks:
  - AOE: frontal half-moon slash.
  - DOT: aura axe that hits and spins on enemy.
  - Pierce: straight horizontal slash/fissure.
  - Bounce: spirit/aura axe that chains between enemies.
- Current reference process choice for AOE: UE5 sword slash tutorial method class, supported by weapon trail/ribbon concepts.
- Recommended build order: AOE first, then DOT, Pierce, Bounce.
- Acceptance rubric, open decisions, and performance/instrumentation notes.

The AOE mechanism packet covers:

- Current AOE production binding and proof.
- The `Hero_1_black_aoe` binding to `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`.
- Data path: `Content/Data/CombatVFXBindings.csv` to `DT_CombatVFXBindings.uasset`.
- Runtime seam: `UT66CombatComponent` resolves the binding and logs `CombatVFXProductionSpawned`.
- Logical hitbox contract: crescent band, not Niagara collision or material opacity.
- Current hitbox values: `AoeInnerRadiusRatio=0.54`, `BaseVisualRadius=411.4`, effective proof radius/inner radius via item-stat proof.
- Proof state: visual/logical hitbox alignment for Hero 1 AOE is present; full VFX-family pipeline remains partial because future weapons, idol overlays, final polish, and normal acquisition proof are not complete.

Path support for this statement:

- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md:564-567` describes the production binding row and runtime resolution/logging.
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md:605-608` describes `AoeInnerRadiusRatio`, setup generation, inner-radius query use, and `BaseVisualRadius=411.4`.
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md:625` records runtime proof values including `EffectiveSlashRadius`, `EffectiveSlashInnerRadius`, `AoeInnerRadiusRatio`, `BaseVisualRadius`, and `VisualScale`.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md:255-277` inventories the implemented Hero 1 AOE production binding, crescent-band geometry, dispatcher, and proof capture mode.
- `Gameplay/Combat/MASTER_COMBAT.md:189-190` records the current Hero 1 AOE sector/crescent behavior and current production VFX binding.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md:56-61` records the hitbox/data/binding updates.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md:81-82` records proof-log values and rounded-runtime note.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/HANDOFF_NEXT_AGENT.md:58-63` records the next-agent critical constants and proof baseline.

The infrastructure inventory covers:

- Lab and production asset paths.
- Capture tooling:
  - `Scripts/CaptureT66GameplayVideo.ps1`
  - `Scripts/CaptureT66NiagaraMRQIsolation.ps1`
  - runtime screenshot sequence modes including `hero1axeaoe`, `hero1axeaoehitbox`, and `hero1axeaoevfxbinding`.
- Validation/setup tooling:
  - `Scripts/ValidateHero1AxeAOELabVFX.py`
  - `Scripts/SetupCombatVFXBindingsDataTable.py`
  - `Scripts/ValidateCombatVFXProductionBindings.py`
- Runtime contract:
  - VFX presentation is separate from logical damage authority.
  - Temp projectile system remains placeholder/fallback and should not be revived as the real idol overlay system.
  - Hero 1 AOE is the only first production binding. DOT/Pierce/Bounce/idols still require packets and proof.

Cleanup and handoff docs cover:

- Current tree status and proof artifacts for the AOE hitbox cleanup.
- Next-agent boundary: verify normal altar/item/stat acquisition before idol overlay design.
- Known loose ends and caveats.

### Current End-To-End Process For Future VFX

The current documented process is:

1. Derive the working goal and read the relevant routers: `AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, pending issues, then the per-family/per-effect docs.
2. Classify the effect and identify the source process. If source evidence is a video and no transcript is already present, ask Pablo for the transcript.
3. Codex writes its own source/process interpretation first.
4. Send that interpretation and plan to Claude as a review, then reconcile objections.
5. Write PPF check, artifact parity gate, and mechanism manifest before implementation.
6. Generate or select a visual target/mockup when needed, using account-backed imagegen. Get Pablo's approval when the process calls for it.
7. Create or update the per-effect packet with source evidence, artifacts, mechanisms, masks/materials, Niagara layers, timing, capture gates, and proof requirements.
8. Build the effect in lab/production paths while preserving the required Niagara/material/renderer carrier.
9. Run structural validation and keep lab-only assets out of production dependencies.
10. Use the same-view editor isolation gate for isolated visual comparison when visual fidelity is being judged.
11. Use gameplay capture with logs/debug evidence for temporal behavior, hitbox alignment, and damage authority.
12. Close with PPF close and mechanism close. Do not call the result production-ready until the required mechanisms and evidence are present and the user signs off.
13. If promoting to production, add/update the binding row, generate or refresh the DataTable, run the production binding validator, capture runtime proof, and document the proof.

### Missing Or Weak Areas

1. Durability gap: the process docs, scripts, data rows, DataTable, and Niagara asset are still untracked. The structure exists locally but is not yet guaranteed in a clean checkout.
2. No single one-page VFX index. The information is organized, but a new agent still has to know which docs to open in what order.
3. No generic per-effect packet template yet. Current AOE packet is useful, but future DOT/Pierce/Bounce/idol packets need a reusable skeleton.
4. No generalized production binding validator for all future VFX. Current validation is Hero 1 AOE-focused.
5. No normal player-facing altar/item/stat acquisition proof yet. Automation proved scale/speed/damage behavior, but normal user flow still needs proof.
6. Idol overlay architecture is not designed yet. The docs correctly warn not to revive the temp projectile placeholder path, but do not define the final overlay binding/composition layer.
7. Future Hero 1 DOT/Pierce/Bounce effects do not yet have full source breakdowns, mechanism packets, bindings, or proof.
8. Capture evidence still has manual selected-frame windows. `Scripts/pending_issues_Scripts.md` notes the need for best-frame selection from logged fire time or image activity.
9. AOE visual polish is accepted for now but not final production visual signoff.
10. `Hero1AxeVFXPlan.md` contains older status language that still describes the AOE as an approved Step 1 lab prototype/static crescent `PARTIAL` state. Concrete examples are `Hero1AxeVFXPlan.md:5`, `Hero1AxeVFXPlan.md:61`, and `Hero1AxeVFXPlan.md:111-115`. Some of that historical context may be useful, but it needs an audit/update so future agents can distinguish historical prototype status from the newer production-binding and hitbox proof docs.
11. Source-of-truth policy for generated CSV/uasset artifacts needs to be explicit: whether source CSV is authoritative, whether generated uassets are checked in, and how regeneration is verified.
12. The infrastructure inventory is not yet linked from a central combat/VFX index.

Path support for selected missing-area claims:

- `Gameplay/Combat/CombatVFXInfrastructureInventory.md:299` says the binding CSV/uasset are generated assets and may not be tracked; future agents must inspect files directly and run setup/validator scripts.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md:313-315` says Hero 1 AOE is resolved, while future weapons/idols still need reviewed binding rows, production-promotion proof, and effect-specific logical geometry/proof targets.
- `Scripts/pending_issues_Scripts.md:55-57` records the manual selected-frame limitation and desired best-frame post-processor.
- `Gameplay/Combat/pending_issues_Combat.md:41-43` records the same selected-frame limitation in the combat VFX context.

## Codex Proposed Improvements

These are recommendations only. None of them should be treated as approved work until Pablo explicitly gives a new implementation go-ahead.

1. Create a one-page VFX process index under `Gameplay/Combat`, probably `VFX_PROCESS_INDEX.md` or `CombatVFX_README.md`, that tells a new agent exactly what to read first and in what order. This should be additive: it should route to the existing procedure and packets, not replace `Gameplay/GAMEPLAY_AGENTS.md` or `CombatVFXAuthoringProcedure.md`.
2. Create `Gameplay/Combat/EffectPacketTemplate.md` with mandatory sections for source evidence, PPF, artifact parity, mechanism manifest, mockup decomposition, material/mask manifest, Niagara layer stack, hitbox contract, capture plan, close evidence, and known caveats. This should be a reusable instance skeleton, not a duplicate of the generic authoring procedure.
3. Add a `Definition of Done` matrix with statuses such as:
   - Process docs present.
   - Source/reference accepted.
   - Mockup approved.
   - Lab asset validates.
   - Same-view editor capture passes.
   - Gameplay capture passes.
   - Hitbox/logical authority proven.
   - Production binding validates.
   - Normal acquisition proven.
   - Docs and generated assets are durable in version control.
4. Generalize `ValidateCombatVFXProductionBindings.py` so it can validate all rows/effects, not only Hero 1 AOE.
5. Add a normal-flow proof mode for weapon altar selection plus item/stat changes, separate from deterministic automation-only proof.
6. Improve evidence capture with automatic best-frame/contact-sheet selection based on fire time, pixel activity, color/intensity, and debug log events.
7. Write an idol overlay architecture doc before any idol implementation. It should define binding shape, layer composition, parameter propagation, damage-authority boundaries, and proof gates.
8. Audit and remove stale VFX status language from `Hero1AxeVFXPlan.md`.
9. Decide and document version-control policy for generated DataTables and VFX binding CSV/uassets.
10. Decide whether to make the current VFX process baseline durable by staging/committing the current process docs/scripts/assets, or explicitly mark some files as local/provisional. This is a Pablo decision because it changes repo state.

## Proposed Final Answer Shape

The final answer should say:

- Scope note: this answer covers the combat VFX instruction/process structure, not a full repo-wide instruction audit.
- Yes, the combat VFX process is organized in the working tree and it is much better than before.
- The main caveat is durability: many key files are untracked or modified, so the organization is not yet safe from a clean-checkout perspective.
- Then summarize the layers of coverage, the current future-VFX process, the missing pieces, and the recommended improvements.
- Make clear that improvements are proposals and need Pablo's go-ahead before implementation.
- It should mention the Claude review artifact path once review is complete.

## Reviewer Request

Please review this status assessment as a read-only reviewer. Identify:

- Any flawed assumption about the current instruction organization.
- Any missing instruction file or process category that should be mentioned.
- Any gap that is overstated or understated.
- Any final-answer caveat that must be clearer for Pablo.
- Any recommendation that is unsafe, too broad, or not grounded in the inspected live files.

First non-empty line must be exactly one of:

`Verdict: APPROVE`

or

`Verdict: REVISE`

Then provide concise rationale and any required changes.

</review_packet>
