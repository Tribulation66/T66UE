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
- Packet path: C:\UE\T66\Reports\AgentReviews\VFXDurableBaselineImplementation_20260528\staged_diff_review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# VFX Durable Baseline Staged Diff Review Packet

## Working Goal

Implement and locally commit the durable VFX infrastructure baseline, including the AGENTS.md repeated-question fix, VFX process docs, combat-VFX-local generated asset policy, infrastructure-only Hero 1 DOT/Pierce/Bounce support, production-path automation proof, opt-in auto frame evidence, and VFX-only commit hygiene.

## Review Scope

Review the staged diff currently in C:\UE\T66. This is a pre-commit review. Do not propose implementation work outside the staged VFX baseline unless it is a blocker for committing safely.

The staged manifest is Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/staged_manifest.md.

## Key Scope Points

- Local commit only; no push.
- No new DOT/Pierce/Bounce visual assets; their packets are infrastructure-only.
- Idol overlay work is architecture-only; no active idol rows or assets.
- Existing Hero 1 AOE generated/runtime assets are included because Pablo approved committing them.
- Content/Data/Weapons.csv and Content/Data/DT_Weapons.uasset are included because the validator and crescent-band hitbox contract depend on `AoeInnerRadiusRatio`; this is VFX hitbox wiring, not unrelated weapon-balance work.
- Structured CSV comparison proves the weapon-data source change is limited to adding `AoeInnerRadiusRatio`, with no row add/remove and no pre-existing column value changes. The only non-zero value is `Hero_1_black_aoe = 0.54`; the matching DT_Weapons.uasset is the generated runtime DataTable.
- Hero1AxeVFXPlan.md is staged as the durable Hero 1 axe VFX status/plan doc. It records AOE backend/hitbox proof and marks DOT/Pierce/Bounce as infrastructure-only.
- pending_issues_Combat.md is Combat VFX-only. Scripts/pending_issues_Scripts.md keeps existing non-VFX pending issues unchanged and only stages two VFX resolved entries.
- AGENTS.md Report Artifact Routing is intentionally repo-wide process scope because this baseline adds durable proof/report artifacts and fixes the repeated decision-gate loop. MASTER_COMBAT.md's production-path automation sentence is an intentional Combat VFX policy note, not a behavior rewrite.
- This is not a pure content-only VFX commit: it intentionally changes root `AGENTS.md` working-goal/decision-block behavior and adds root report-artifact routing. The commit message body must say that.
- Pablo approval quote is in Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md: `Existing generated/runtime VFX data/assets: include the approved Hero 1 AOE production binding/runtime assets in the local VFX-only commit after staged-diff review; do not author new visual VFX assets in this pass.`
- Additional decision_block.md authorization excerpts: root AGENTS scope quote says, `Ok this goal change needs to be reflected in the agents.md file, because this occurred in a different agent as well, with the repeated questions.` Implementation quote says, `1. Infrastructure only. 2. B. 3. B. Ok you have permission to go with the implementation now.`
- Current root process-router context was supplied by the user in the thread and already included Report Artifact Routing; the staged root AGENTS.md hunk brings HEAD up to that current contract. MASTER_COMBAT.md is updated only as the Combat folder router for the newly approved VFX process index/proof contract.
- Scripts/CaptureT66GameplayVideo.ps1 is staged as an implementation-plan addendum because it is the required opt-in wrapper seam for the new evidence auto-frame mode.
- Gameplay/Combat/MASTER_COMBAT.md must only stage the top VFX router hunk; unrelated existing combat doc hunks must remain unstaged.
- Generated/runtime .uasset files must be staged as Git LFS pointers.
- Synthetic self-test raw frames/contact sheets/video are intentionally not staged; the durable self-test output and self-test report stay staged.
- Staged diff must not include Mini/minigame scope.

## Verification Already Run

- python Scripts/BuildT66VideoEvidenceBundle.py --self-test-root Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/EvidenceBundleSelfTest: PASS.
- python Scripts/ValidateCombatVFXProductionBindings.py --self-test-root Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/BindingValidatorSelfTest: PASS.
- Unreal commandlet production binding validator: PASS with === Combat VFX production binding validation DONE === in Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ValidateCombatVFXProductionBindings_Unreal.log.
- Default Scripts/RunHero1AxeAOEVFXBindingProof.ps1 -PrintOnly normalized output matches pre-change proof.
- Existing normal-flow proof references are cited in Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/PROOF_SUMMARY.md, including Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Hero1AxeAOEVFXBindingProofSummary.md and Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/.
- LFS checks: DT_CombatVFXBindings.uasset, DT_Weapons.uasset, and NS_Hero1AxeAOE_MeshSlash.uasset all have filter/diff/merge=lfs and staged `git cat-file -p :path` outputs are LFS pointer text; exact oids are recorded in the staged manifest.
- CombatVFXBindings.csv row check: exactly one AOE row is active for Hero1Axe_AOE_Base; there are no DOT/Pierce/Bounce/idol overlay rows or non-empty future-effect asset paths.
- Weapons.csv structured diff: old columns 21, new columns 22, only added column `AoeInnerRadiusRatio`, old rows 192, new rows 192, no added/removed rows, no non-added-column value changes, one non-zero value `Hero_1_black_aoe = 0.54`; saved at Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/WeaponsCsvAoeInnerRadiusDiff_summary.txt.
- DT_Weapons.uasset stat: `Content/Data/DT_Weapons.uasset | 4 ++--`, matching the generated DataTable update for the CSV.
- Future-effect asset guard: rg over DOT/Pierce/Bounce packets and the idol overlay architecture doc for `Content/VFX`, `.uasset`, `NiagaraSystem`, and `/Game/VFX` returned no output.
- Working-tree deletion guard: current status has unstaged deletions under QuickRevive, Cliffs, Vending, and ToonStyle reports; they are intentionally excluded and commit must use plain `git commit`, not `git commit -a`, `git add -A`, or broad staging.
- Scripts pending diff proof: Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ScriptsPendingIssuesDiff_snippet.txt reproduces `git diff --cached -- Scripts/pending_issues_Scripts.md`; it appends exactly two VFX-resolved entries.
- Changed-file hunk audit: AGENTS.md and MASTER_COMBAT.md hunks are reproduced below; Weapons.csv is covered by the structured CSV proof; DT_Weapons.uasset is covered by LFS pointer/stat proof; Scripts/pending_issues_Scripts.md is covered by the diff snippet proof.
- Capture wrapper default check: the normalized `RunHero1AxeAOEVFXBindingProof.ps1 -PrintOnly` output includes capture wrapper command/argument lines and matches pre-change proof when `-EvidenceAutoSelectFrames` is not passed.
- Report marker check: Reports/AGENTS.md already routes proof runs under `Reports/Proof/<Domain>/<TaskSlug>` and says new raw run folders should include `.report-run.json`; the parent `.report-run.json` at Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ applies to the child self-test/evidence folders because they are part of one proof run.
- PROOF_SUMMARY.md now states the parent `.report-run.json` applies to `EvidenceBundleSelfTest`, `BindingValidatorSelfTest`, and `AutoFrameSelection_Hero1AxeAOE_EdgeFinal` because they are child evidence folders inside one proof run.
- AGENTS anchor check: Reports/AGENTS.md has no `### Report Artifact Routing` heading, so the root AGENTS.md section does not collide with an existing section anchor.
- Excluded deletion follow-up: the QuickRevive/Cliffs/Vending/ToonStyle deletions remain outside this VFX commit for a separate cleanup/hygiene pass; no broad staging will be used.
- Mini scope guard: git diff --cached --name-only | Select-String -Pattern 'Mini|Minigame' returned no output.
- git diff --cached --check: PASS: no whitespace errors.

## Staged Short Stat

~~~text
 57 files changed, 6252 insertions(+), 200 deletions(-)
~~~

## Deletion Accounting

~~~text
AGENTS.md                         |  14 +-
Content/Data/DT_Weapons.uasset    |   4 +-
Content/Data/Weapons.csv          | 386 +++++++++++++++++++-------------------
Gameplay/Combat/MASTER_COMBAT.md  |   6 +-
Scripts/pending_issues_Scripts.md |  13 ++
5 files changed, 223 insertions(+), 200 deletions(-)
~~~

## Draft Commit Message

~~~text
Codify durable combat VFX pipeline

- add combat VFX process index, DoD, packet template, generated asset policy, and idol overlay architecture
- document infrastructure-only Hero 1 DOT/Pierce/Bounce packet path and production-path proof expectations
- generalize Combat VFX binding/evidence helpers while preserving Hero 1 AOE production checks
- update root AGENTS.md goal/decision-block protocol and report artifact routing to prevent repeated clarification loops
- update MASTER_COMBAT.md to route Combat VFX work through the new process index and production-path proof contract
- preserve approved Hero 1 AOE generated/runtime VFX assets and hitbox data: Content/Data/CombatVFXBindings.csv, Content/Data/DT_CombatVFXBindings.uasset, Content/Data/Weapons.csv, Content/Data/DT_Weapons.uasset, and Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset
- decision scope recorded in Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md

Local commit only; no push.
~~~

## Staged Name Status

~~~text
M	AGENTS.md
A	Content/Data/CombatVFXBindings.csv
A	Content/Data/DT_CombatVFXBindings.uasset
M	Content/Data/DT_Weapons.uasset
M	Content/Data/Weapons.csv
A	Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset
A	Gameplay/Combat/CombatVFXDefinitionOfDone.md
A	Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md
A	Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md
A	Gameplay/Combat/CombatVFXInfrastructureInventory.md
A	Gameplay/Combat/EffectPacketTemplate.md
A	Gameplay/Combat/Hero1AxeBounceMechanismPacket.md
A	Gameplay/Combat/Hero1AxeDOTMechanismPacket.md
A	Gameplay/Combat/Hero1AxePierceMechanismPacket.md
A	Gameplay/Combat/Hero1AxeVFXPlan.md
M	Gameplay/Combat/MASTER_COMBAT.md
A	Gameplay/Combat/VFX_PROCESS_INDEX.md
A	Gameplay/Combat/pending_issues_Combat.md
A	Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/20260528T075159-pass9/claude_review_pass9.md
A	Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/20260528T075159-pass9/claude_review_prompt_pass9.md
A	Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md
A	Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/implementation_plan.md
A	Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/staged_diff_review_packet.md
A	Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/staged_manifest.md
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/.report-run.json
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/contact_sheet.png
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/manifest.json
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/selected_frames.md
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/selected_frames/00_start_frame_0051.png
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/selected_frames/01_mid_frame_0056.png
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/selected_frames/02_impact_frame_0062.png
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/selected_frames/03_dissipate_frame_0071.png
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal/visibility_checklist.md
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/AutoFrameSelection_Hero1AxeAOE_EdgeFinal_output.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/BindingValidatorSelfTest/negative_bindings.csv
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/BindingValidatorSelfTest/positive_bindings.csv
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/BindingValidatorSelfTest/self_test_report.json
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/BindingValidatorSelfTest_output.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/EvidenceBundleSelfTest/self_test_report.json
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/EvidenceBundleSelfTest_output.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/PROOF_SUMMARY.md
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/RunHero1AxeAOEVFXBindingProof_printonly_autoframe.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/RunHero1AxeAOEVFXBindingProof_printonly_normalized_diff.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/RunHero1AxeAOEVFXBindingProof_printonly_postchange.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/RunHero1AxeAOEVFXBindingProof_printonly_prechange.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ScriptsPendingIssuesDiff_snippet.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ValidateCombatVFXProductionBindings_Unreal.log
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ValidateCombatVFXProductionBindings_Unreal_stdout.log
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/WeaponsCsvAoeInnerRadiusDiff_summary.txt
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/generated_runtime_assets_identity_postvalidation.json
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/generated_runtime_assets_identity_prevalidation.json
A	Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/generated_runtime_assets_identity_prevalidation.txt
A	Scripts/BuildT66VideoEvidenceBundle.py
A	Scripts/CaptureT66GameplayVideo.ps1
A	Scripts/RunHero1AxeAOEVFXBindingProof.ps1
A	Scripts/ValidateCombatVFXProductionBindings.py
M	Scripts/pending_issues_Scripts.md
~~~

## Critical Cached Hunk: MASTER_COMBAT.md

~~~diff
diff --git a/Gameplay/Combat/MASTER_COMBAT.md b/Gameplay/Combat/MASTER_COMBAT.md
index 3606d4a2..0c3dc671 100644
--- a/Gameplay/Combat/MASTER_COMBAT.md
+++ b/Gameplay/Combat/MASTER_COMBAT.md
@@ -1,10 +1,12 @@
 # T66 Master Combat

-**Last updated:** 2026-05-26
+**Last updated:** 2026-05-28
 **Scope:** Single-source handoff for combat runtime flow, targeting, damage routing, damage provenance logging, combat collision roles, debug visibility, hit feedback, spatial headshots, accuracy-driven aiming, and boss body-part combat.
-**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Backend/Anti Cheat/ANTI_CHEAT_POLICY_REFERENCE.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
+**Companion docs:** `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md`, `Backend/Anti Cheat/ANTI_CHEAT_POLICY_REFERENCE.md`, `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
 **Maintenance rule:** Update this file after every material combat, targeting, damage-model, hitbox, projectile, boss-health, or combat-UI change.

+Combat VFX process note: start future VFX work from `VFX_PROCESS_INDEX.md` and `CombatVFXDefinitionOfDone.md`. Production-path automation proof must use real weapon selection, RunState inventory/item stats, combat fire, VFX binding lookup, and damage paths; it is not the same as literal UI-click proof unless a packet requires UI-click validation.
+
 ## 1. Executive Summary

 - Combat now supports target handles with optional hit-zone selection for standard enemies and bosses.
~~~

## Critical Cached Hunk: AGENTS.md

~~~diff
diff --git a/AGENTS.md b/AGENTS.md
index 295790e6..7d297373 100644
--- a/AGENTS.md
+++ b/AGENTS.md
@@ -3,11 +3,12 @@
 ## 1. Project Contract

 - Treat this file as the root process router for `C:\UE\T66`. It defines global behavior, accepted process classes, and where to find deeper folder-owned instructions.
-- Before answering or acting on every new user request or question, derive the current working goal in one sentence. Use the native goal/set_goal mechanism when available; otherwise state the working goal in the conversation.
-- Use the working goal to decide what files/systems to inspect, what changes are in scope, and what verification proves the request is done.
+- Before answering or acting on every new user request or question, derive the current working goal in one sentence. When a native goal function is available (`/goal`, `set_goal`, `create_goal`, or an equivalent tool), call it to create or set that goal before planning, review, implementation, or substantive answers. If no goal function is available, state the working goal in the conversation.
+- Treat the function-created or conversation-stated working goal as the active task contract. Use it to decide what files/systems to inspect, what changes are in scope, and what verification proves the request is done.
 - Start from the live repo, current folder instructions, current assets, current scripts, and current machine state. Do not answer from stale docs or memory when the live project can be checked.
 - Default scope excludes Mini/minigame systems. Unless the user explicitly names Mini, minigames, a specific minigame, or a Mini-owned path, do not inspect, change, recommend, capture, validate, or include Mini/minigame code, UI, assets, docs, checklists, or generated outputs. Do not treat unrelated terms such as minimap or mini-stat as Mini scope. If a task appears impossible without Mini/minigame work, ask before including it.
-- User constraints, planning-only boundaries, and repository instructions override convenience. If the user changes scope, replace the working goal and discard stale assumptions.
+- User constraints, planning-only boundaries, and repository instructions override convenience. If the user changes scope, update, close, or replace the active function-created goal with the available goal controls before proceeding; if the environment cannot update the function goal, state the replacement working goal in the conversation and discard stale assumptions.
+- Working goals must describe the full requested end state, not a temporary clarification gate. If a decision gate is needed, ask it once and, for durable work, save it as `Reports/AgentReviews/<TaskSlug>/decision_block.md`; on continuations, reference the saved gate instead of repeating questions. Mark `blocked` only when no safe default, instruction, or user-approved decision can move the full goal forward.
 - For each completed change, report the exact verification performed, or state clearly why verification was skipped.
 - Do not silently swap an accepted process for a faster or simpler method. If the process matters to the quality target, the process is part of the task.
 - For solved-category visual, animation, rigging, VFX, import, UI fidelity, generated-media, and comparable production tasks, use the research-first replication rule in Section 2 before implementation.
@@ -206,6 +207,13 @@ Reported status: FULL/PARTIAL
 - Prefer narrow path checks against the specific files being inspected. Broad scans can spawn many `git-lfs.exe` workers that hash large `.uasset` or generated asset files, saturating disk I/O and freezing the desktop.
 - If a broad Git/LFS scan is necessary, warn the user first and explain that it may temporarily hammer disk. Treat high disk usage with many `git-lfs.exe` processes and little or no network activity as local LFS hashing/comparison, not a push or pull.

+### Report Artifact Routing
+
+- Store agent-authored reports, handoff packets, proof summaries, review packets, cleanup manifests, and temporary report runs under `Reports/`, following `Reports/AGENTS.md`.
+- Use `Audit/` only for user-requested audits or existing audit lifecycle workflows. Do not put ordinary reports there.
+- Report-only ToonStyle artifacts belong under `Reports/ToonStyle`, not under production ToonStyle folders.
+- Raw report/proof run folders expire after 15 days. Delete whole run folders only after confirming a durable summary exists outside the raw folder and no active references still point at that run. New raw run folders should include `.report-run.json` with `expiresAfterDays: 15` for unambiguous future cleanup.
+
 ### Script Lifecycle

 - Keep reusable master scripts tight and documented. Delete task-specific scripts after the task is accomplished, but first move durable process improvements into the relevant master script, README, or process doc.
~~~

## Requested Review

Check for:

- Scope leaks into unrelated code/content/docs.
- Any unstaged/staged mismatch that undermines the VFX-only commit.
- Validator/evidence tooling problems that would make the proof unreliable.
- Report artifacts that are too transient or missing a durable summary.
- Any reason the commit should be blocked before local commit.

First non-empty line must be exactly Verdict: APPROVE, Verdict: REVISE, or Verdict: BLOCK.

</review_packet>
