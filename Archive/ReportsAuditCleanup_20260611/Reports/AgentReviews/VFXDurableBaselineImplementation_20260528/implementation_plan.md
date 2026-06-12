# VFX Durable Baseline Implementation Plan

Date: 2026-05-28

## Working Goal

Implement and locally commit a durable VFX-only baseline, including the repeated-question prevention change in `AGENTS.md`, VFX process infrastructure, combat-VFX-local generated asset policy, infrastructure-only Hero 1 DOT/Pierce/Bounce support, production-path automation proof, and commit-gate evidence.

## User Decisions

See `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md`.

- DOT/Pierce/Bounce scope: infrastructure only now.
- Normal proof: production-path automation proof first.
- Generated asset policy: combat-VFX-local policy now; repo-wide policy out of scope.
- Idol overlay scope: architecture-only seam document; no idol assets, active idol rows, or implemented idol behavior.
- Existing generated/runtime VFX data/assets: include approved Hero 1 AOE production binding/runtime assets in the local VFX-only commit; do not author new visual VFX assets in this pass.
- Automated best-frame selection: in scope as an opt-in evidence-tooling improvement because Pablo included it in the explicit "What Is Missing" list; default/manual behavior must remain unchanged.
- Root `AGENTS.md` update: in scope because Pablo explicitly said the repeated-question goal change needs to be reflected in `AGENTS.md`.
- Commit locally only; no push.
- Decision artifact status: `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md` already exists.

## Applicable Instructions

- `AGENTS.md`: full-goal handling, PPF/process fidelity, Claude review, report routing, no Mini scope, local commit safety, no destructive cleanup.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat VFX routes through `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md`: review/proof artifacts under `Reports/AgentReviews` and `Reports/Proof`.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: generic combat VFX authoring process and evidence gates.
- `Gameplay/Combat/Hero1AxeVFXPlan.md`: Hero 1 axe family plan; needs stale-status cleanup only.
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: concrete existing packet/naming pattern.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: current gaps and implemented Hero 1 AOE seams.

## PPF Check

```text
PPF CHECK
Objective: Build durable VFX process/backend infrastructure without authoring new visual VFX.
Proven process: AGENTS.md Section 2/3/4 plus Gameplay/Combat/CombatVFXAuthoringProcedure.md and existing Hero1AxeAOESlashMechanismPacket.md.
My planned implementation: Extend the existing process hierarchy, packet pattern, validator/proof helpers, and capture evidence helper. Do not create a parallel VFX process or new visual carrier method.
Same method class: YES
If NO, why: N/A
User approval required before proceeding: NO, user approved the answered scope and implementation.
Verification evidence: Claude plan review, script self-tests, Unreal validator commandlet log, staged-file manifest, Claude staged-diff review, local commit.
```

Approval interpretation: Pablo has already given implementation and local-commit permission for the approved VFX-only scope. After implementation, local commit proceeds only if validation passes, the staged manifest is in-scope, and Claude staged-diff review approves. If the staged diff contains out-of-scope hunks or Claude staged-diff review returns REVISE/BLOCK, stop for Pablo instead of committing.

## Artifact Parity Gate

```text
ARTIFACT PARITY GATE
Reference artifact/category: Existing Hero1AxeAOESlashMechanismPacket.md and CombatVFXAuthoringProcedure.md packet/procedure split.
Role: Primary
Required: YES
Planned artifact/path: Gameplay/Combat/EffectPacketTemplate.md plus Hero1AxeDOT/Pierce/Bounce mechanism packet stubs.
Status: SAME
Evidence: New files will reference the generic procedure and avoid duplicating it.
```

```text
ARTIFACT PARITY GATE
Reference artifact/category: Existing Hero 1 AOE production binding validator/proof path.
Role: Primary
Required: YES
Planned artifact/path: Scripts/ValidateCombatVFXProductionBindings.py, Scripts/BuildT66VideoEvidenceBundle.py, Scripts/RunHero1AxeAOEVFXBindingProof.ps1.
Status: EQUIVALENT
Evidence: Generalize active-row validation and best-frame/proof modes while preserving Hero1Axe_AOE_Base hard requirements.
```

## Mechanism Manifest

```text
MECHANISM MANIFEST
Reference/source: CombatVFXAuthoringProcedure.md, CombatVFXInfrastructureInventory.md, current proof scripts.
Required mechanisms:
  1. Mechanism: full-end-state goal routing
     Required: YES
     Planned implementation: add AGENTS.md rule that decision gates live as artifacts and are not the function goal.
     Evidence needed: AGENTS.md diff plus decision_block.md artifact.
  2. Mechanism: process routing
     Required: YES
     Planned implementation: add VFX_PROCESS_INDEX.md and DoD file linking to existing procedure and packets.
     Evidence needed: docs exist and route to owner docs.
  3. Mechanism: infrastructure-only future effect packets
     Required: YES
     Planned implementation: add DOT/Pierce/Bounce packet stubs marked deferred/no active rows.
     Evidence needed: packet files and validator active-row rule.
  4. Mechanism: production binding validation
     Required: YES
     Planned implementation: keep Hero1Axe_AOE_Base hard checks, add active-row iteration/reporting and deferred scaffold behavior.
     Evidence needed: validator self/report output and Unreal commandlet log.
  5. Mechanism: production-path automation proof
     Required: YES
     Planned implementation: document and harden current hero1axeaoevfxbinding proof as production-path automation, with wrapper/report updates.
     Evidence needed: proof wrapper dry run or print-only plus docs.
  6. Mechanism: automated best-frame selection
     Required: YES
     Planned implementation: add optional auto-select mode to BuildT66VideoEvidenceBundle.py using frame activity/bounding box heuristics with manual override.
     Evidence needed: helper self-test output and manifest showing selection method.
```

## Files To Edit Or Add

Root/process:

- `AGENTS.md`: add durable goal/decision-block prevention rule in Section 1 immediately after the existing bullet about replacing/discarding stale function-created goals. Authoritative staged text:

  ```markdown
  - Working goals must describe the full requested end state, not a temporary clarification gate. If a decision gate is needed, ask it once and, for durable work, save it as `Reports/AgentReviews/<TaskSlug>/decision_block.md`; on continuations, reference the saved gate instead of repeating questions. Mark `blocked` only when no safe default, instruction, or user-approved decision can move the full goal forward.
  ```

Gameplay/Combat docs:

- Add `Gameplay/Combat/VFX_PROCESS_INDEX.md`.
- Add `Gameplay/Combat/CombatVFXDefinitionOfDone.md`.
- Add `Gameplay/Combat/EffectPacketTemplate.md`.
- Add `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`.
- Add `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`: architecture-only seam document justified by the approved "what is missing" list. It will define future overlay ownership, binding/parameter layering, and damage-authority boundaries only. It must not reference concrete idol VFX assets, create active idol binding rows, or implement idol behavior.
- Add `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`.
- Add `Gameplay/Combat/Hero1AxePierceMechanismPacket.md`.
- Add `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`.
  - Each infrastructure-only packet must contain: status/deferred banner, source/process references, primary carrier archetype target, planned artifact slots, active-row rule, proof-template placeholders, and explicit "no active production binding row yet" language. They are not just empty headers.
- Update `Gameplay/Combat/Hero1AxeVFXPlan.md` surgically:
  - add `## Current Status Snapshot - 2026-05-28` near the top after the metadata block,
  - state that Hero 1 AOE now has production binding/hitbox backend proof but still lacks final visual-polish approval,
  - state DOT/Pierce/Bounce remain infrastructure-only in this pass,
  - add `## Superseded Historical Notes` at the bottom for older static-crescent prototype language if it must be preserved out of the current-status path,
  - do not delete historical decisions or rewrite preserved sections.
- Update `Gameplay/Combat/MASTER_COMBAT.md` only with a single process-router note of 10 lines or fewer naming `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, and the production-path automation proof contract. No combat behavior rewrite.
- Update `Gameplay/Combat/CombatVFXInfrastructureInventory.md` only with a single "Durable baseline index" note of 10 lines or fewer that points future agents to `VFX_PROCESS_INDEX.md` as the first read after folder routers. No inventory rewrite.

Scripts:

- `Scripts/ValidateCombatVFXProductionBindings.py`: generalize active-row validation/reporting while preserving Hero 1 AOE row and geometry assertions.
- `Scripts/ValidateCombatVFXProductionBindings.py`: add a no-Unreal `--self-test-root` mode that exercises CSV active-row parsing/deferred-row behavior and writes self-test output without requiring `unreal` import.
- Validator self-test implementation must defer the `import unreal` statement into the Unreal-only validation path. `--self-test-root` must run from system Python without attempting to import `unreal`.
- `Scripts/BuildT66VideoEvidenceBundle.py`: add optional automatic frame selection using saturated/non-background activity and bounding box area, keep manual selected-frame override, and keep existing behavior unchanged unless an explicit auto flag is passed.
  - Default behavior must remain byte-for-byte equivalent in selected-frame choices and contact-sheet path when no auto flag is passed, apart from unavoidable timestamped output paths in wrappers. The new code path is behind an explicit flag such as `--auto-select-frames`.
- `Scripts/CaptureT66GameplayVideo.ps1`: expose the evidence helper's new auto-selection flag as opt-in only. This is the necessary wrapper seam between production capture scripts and `BuildT66VideoEvidenceBundle.py`; defaults must remain unchanged.
- `Scripts/RunHero1AxeAOEVFXBindingProof.ps1`: add/confirm print-only and production-path proof notes; expose the new evidence auto-frame mode as opt-in only so existing captures keep their manual selected-frame behavior by default.
- `Scripts/pending_issues_Scripts.md`: optional edit only if the manual-frame pending issue is resolved or narrowed by opt-in automation; no change is acceptable.
- `Gameplay/Combat/pending_issues_Combat.md`: optional edit only if one of the previously documented Combat VFX gaps is resolved or narrowed by this pass; no change is acceptable.
  - Criteria: if this pass materially closes a documented gap, update the pending issue; otherwise leave the file untouched.

Generated/runtime assets to stage but not author in this pass:

- `Content/Data/CombatVFXBindings.csv`: approved Hero 1 AOE production binding source CSV.
- `Content/Data/DT_CombatVFXBindings.uasset`: generated runtime DataTable matching the CSV.
- `Content/Data/Weapons.csv`: source weapon data with the Hero 1 AOE `AoeInnerRadiusRatio` field required by crescent-band hitbox validation.
- `Content/Data/DT_Weapons.uasset`: generated runtime weapon DataTable matching the CSV.
- `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`: approved Hero 1 AOE production Niagara asset from the earlier VFX work.

These are included because Pablo explicitly approved committing generated/runtime VFX data/assets. This pass must not modify or regenerate a new visual Niagara system unless a validation command necessarily touches timestamps through existing setup; any content diff must be described in the staged manifest.

Current pre-plan status for these generated/runtime assets:

- `git status --short --untracked-files=all -- <three paths>` reports all three as untracked (`??`).
- Pre-run identity was captured before implementation commandlets:
  - `Content/Data/CombatVFXBindings.csv`: SHA256 `A77D4DFD173533CDBEDA047C2DB50E8650C9EEE99AC0D71FD309D226A18764DD`, 551 bytes, mtime UTC `2026-05-28T04:47:43.7911975Z`.
  - `Content/Data/DT_CombatVFXBindings.uasset`: SHA256 `71D011B98C5DB0FDE43A57DBCF4A759B53E73D30E415A13EB6BF03A3221FA8B2`, 3849 bytes, mtime UTC `2026-05-28T04:47:43.7980431Z`.
  - `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`: SHA256 `3025881EF90532FEAA3453B27C097AD2861C797A04E7E8CC6EB43A3911BBAAF4`, 1772344 bytes, mtime UTC `2026-05-28T02:22:37.9645648Z`.

Report/proof:

- Add this implementation packet and later staged manifest under `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/`.
- Add validation summary under `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/`.

## Out Of Scope

- New DOT/Pierce/Bounce visual VFX assets.
- Idol VFX assets or active idol binding rows.
- Repo-wide generated asset policy outside Combat VFX.
- Mini/minigame systems.
- Push/tag/release.
- Redesigning the current Hero 1 AOE visual.

## Risks And Controls

- Risk: staging unrelated modified files. Control: narrow path staging, staged-file manifest, Claude staged-diff review before commit.
- Risk: validator overclaims visual fidelity. Control: validator text and docs state structural validation is not visual acceptance.
- Risk: best-frame heuristic misses subtle effects. Control: manual override remains and selected notes record method.
- Risk: generated asset policy becomes repo-wide by accident. Control: policy title/scope is Combat VFX local and says repo-wide is out of scope.
- Risk: normal proof automation could be mistaken for literal UI click. Control: docs label it production-path automation, not literal UI proof.

Allowed staged paths for this commit:

- `AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/EffectPacketTemplate.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`
- `Gameplay/Combat/Hero1AxePierceMechanismPacket.md`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Scripts/ValidateCombatVFXProductionBindings.py`
- `Scripts/BuildT66VideoEvidenceBundle.py`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Scripts/RunHero1AxeAOEVFXBindingProof.ps1`
- `Scripts/pending_issues_Scripts.md`
- `Content/Data/CombatVFXBindings.csv`
- `Content/Data/DT_CombatVFXBindings.uasset`
- `Content/Data/Weapons.csv`
- `Content/Data/DT_Weapons.uasset`
- `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`
- `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/**`
- `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/**`

Any modified/deleted path outside that list must remain unstaged unless a later Claude-reviewed plan explicitly adds it. Any out-of-scope hunk inside an allowed file must be reported in the staged manifest and left unstaged or separately approved.

Pre-existing modified hunks inside allowlisted files are intentionally unstaged by default, even if they look harmless. Deterministic rule: enumerate every pre-existing hunk by file and line range in the staged manifest; classify any hunk not added by this pass as `intentionally unstaged` unless the manifest cites quoted prior approval from the decision artifact, prior approved packet, or exact user text that makes it part of this VFX baseline. If such a hunk cannot be cleanly excluded, stop before commit and report it to Pablo; do not stage it under a generic `blocker` label.

Pre-existing unrelated hunks in `Gameplay/Combat/Hero1AxeVFXPlan.md`, `Gameplay/Combat/MASTER_COMBAT.md`, and `Gameplay/Combat/CombatVFXInfrastructureInventory.md` also follow this default; they are not rewritten, deleted, or staged unless the manifest cites their VFX-baseline approval.

Risk: `.uasset` content assets could be staged as raw blobs instead of LFS pointers. Control: staged manifest must include `git check-attr` output and pointer-vs-blob inspection for `Content/Data/DT_CombatVFXBindings.uasset`, `Content/Data/DT_Weapons.uasset`, and `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`; commit is blocked if expected LFS pointer behavior is not present.

Shared validator disclaimer string to use in docs/report output:

```text
This validator proves Combat VFX binding structure, required assets, source guards, and declared data contracts. It does not prove visual fidelity, temporal mechanism quality, final player-facing readability, or Pablo visual approval.
```

Disclaimer wiring:

- Print the string from `Scripts/ValidateCombatVFXProductionBindings.py` at validator start and include it in self-test/report output.
- Include the same string in `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `Gameplay/Combat/CombatVFXDefinitionOfDone.md`, and the `Hero1AxeVFXPlan.md` current-status snapshot.
- Include a short reference to the same disclaimer in `Gameplay/Combat/EffectPacketTemplate.md` and the three Hero 1 future-effect packet stubs, because those documents also discuss structural/non-visual scope.

AGENTS.md insertion anchor:

```markdown
- User constraints, planning-only boundaries, and repository instructions override convenience. If the user changes scope, update, close, or replace the active function-created goal with the available goal controls before proceeding; if the environment cannot update the function goal, state the replacement working goal in the conversation and discard stale assumptions.
```

The new goal/decision-block bullet lands immediately after that anchor.

AGENTS.md style check: before staging, compare the new bullet against the surrounding Section 1 bullets for the same imperative voice, bullet length, and conditional wording. Stage the authoritative bullet text above unless staged-diff review explicitly approves a style-only adjustment.

Pre-change proof wrapper output:

- Captured at `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/RunHero1AxeAOEVFXBindingProof_printonly_prechange.txt`.
- After implementation, run the same command and diff against this pre-change output after normalizing timestamped output root paths. Defaults must remain identical except for generated timestamp/output folder names.

Content asset Git/LFS status already checked before implementation:

```text
git status --short --untracked-files=all -- <approved Content paths>
?? Content/Data/CombatVFXBindings.csv
?? Content/Data/DT_CombatVFXBindings.uasset
?? Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset

git check-attr filter diff merge text -- <approved Content paths>
Content/Data/CombatVFXBindings.csv: filter: unspecified
Content/Data/CombatVFXBindings.csv: diff: unspecified
Content/Data/CombatVFXBindings.csv: merge: unspecified
Content/Data/CombatVFXBindings.csv: text: unspecified
Content/Data/DT_CombatVFXBindings.uasset: filter: lfs
Content/Data/DT_CombatVFXBindings.uasset: diff: lfs
Content/Data/DT_CombatVFXBindings.uasset: merge: lfs
Content/Data/DT_CombatVFXBindings.uasset: text: unset
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: filter: lfs
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: diff: lfs
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: merge: lfs
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: text: unset
```

Staged manifest must confirm LFS pointer-vs-blob state after staging for the three staged `.uasset` files and record that the CSV files are normal text content.

Decision artifact content to preserve as the scope source:

```text
DOT/Pierce/Bounce scope: infrastructure only now.
Normal proof: production-path automation proof first.
Generated asset policy: combat-VFX-local policy now, repo-wide generated asset policy out of scope for this pass.
Idol overlay scope: architecture-only seam document now; no idol VFX assets, active idol rows, or implemented idol behavior.
Existing generated/runtime VFX data/assets: include the approved Hero 1 AOE production binding/runtime assets in the local VFX-only commit after staged-diff review; do not author new visual VFX assets in this pass.
Automated best-frame selection: in scope because the user listed it under "What Is Missing"; implement as opt-in evidence tooling only, preserving current manual/default behavior.
Root AGENTS.md update: in scope because Pablo explicitly said the repeated-question goal change needs to be reflected in AGENTS.md.
```

## Verification Plan

1. Capture a second pre-run identity check for the generated/runtime VFX assets before any validator or Unreal commandlet invocation in the implementation phase; record additional weapon-data identity if those hitbox data files enter the staged VFX scope.
2. `python Scripts/BuildT66VideoEvidenceBundle.py --self-test-root <temp>` to verify evidence helper and auto-selection. The self-test must assert that default/manual mode without `--auto-select-frames` selects the existing fixed `auto_indices` frame set, while `--auto-select-frames` records an explicit selection method in the manifest. The default path must not use the auto heuristic.
3. Run `Scripts/BuildT66VideoEvidenceBundle.py` auto-frame mode against this known retained-frame capture, which exists now and has 72 PNG frames: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/frames`. Save manifest/contact-sheet output under the proof folder. If that folder is missing at verification time for any reason, stop and report the disappearance to Pablo; do not silently recapture or silently remove the auto-frame surface area.
4. `python Scripts/ValidateCombatVFXProductionBindings.py --self-test-root <temp>` to verify active-row parsing/deferred-row handling without Unreal.
   - The self-test creates synthetic CSV fixtures under `<temp>` rather than reading project content.
   - Positive fixture: one active Hero 1 AOE-like row with a concrete Niagara system and one deferred DOT/Pierce/Bounce-like scaffold row with no concrete Niagara system.
   - Negative fixture: a malformed active row missing required fields must fail inside the self-test harness.
   - Pass contract: process exits 0 and prints `SELF TEST PASSED`; any failed assertion exits non-zero.
5. Run the production binding validator through Unreal as a hard gate:
   ```powershell
   & 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\UE\T66\T66.uproject' -run=pythonscript -script='C:\UE\T66\Scripts\ValidateCombatVFXProductionBindings.py' -unattended -nop4 -nosplash
   ```
   Save the validator log under `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/`. Commit is blocked unless the command exits with code 0 and the log contains the completion token `=== Combat VFX production binding validation DONE ===` with no `LogPython: Error`, `LogOutputDevice: Error`, `Traceback`, or `[CombatVFXProductionBindingsValidate]` error lines. If UnrealEditor-Cmd is unreachable or the validator reports binding/asset errors, stop and report the blocker instead of committing. Scoped corrections may be made only inside allowed staged paths and only when they do not expand the mechanism manifest or approved scope, then the hard gate must be rerun.
6. Run `Scripts/RunHero1AxeAOEVFXBindingProof.ps1 -PrintOnly` to verify wrapper argument generation without launching capture, then diff normalized output against `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/RunHero1AxeAOEVFXBindingProof_printonly_prechange.txt`.
   - Normalize only timestamped output tokens matching `Hero1AxeAOE_VFXBindingProof_\d{8}_\d{6}` and absolute paths beneath those timestamped output roots.
   - Compare normalized Unreal command argv and capture wrapper argument lines.
   - Cosmetic `Write-Host` additions outside command/argument lines do not invalidate proof by themselves.
   - If normalized command/argument lines differ, existing MP4 proof is invalidated and a fresh production-path capture is required before commit.
7. Record existing production-path proof references from `Hero1AxeAOESlashMechanismPacket.md` and `CombatVFXInfrastructureInventory.md` in the proof summary. Because wrapper/evidence-helper changes are opt-in/additive only and defaults remain unchanged, existing MP4 proof remains valid for the current production path only if Step 6 passes.
8. Record post-validation SHA256 hash, byte size, and modified time for:
   - `Content/Data/CombatVFXBindings.csv`
   - `Content/Data/DT_CombatVFXBindings.uasset`
   - `Content/Data/Weapons.csv`
   - `Content/Data/DT_Weapons.uasset`
   - `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`
9. Run targeted text checks for new route docs and stale references.
   - Verify `VFX_PROCESS_INDEX.md` is referenced from `MASTER_COMBAT.md` and `CombatVFXInfrastructureInventory.md`.
   - Verify the shared validator disclaimer appears verbatim in validator startup output, `VFX_PROCESS_INDEX.md`, `CombatVFXDefinitionOfDone.md`, and `Hero1AxeVFXPlan.md`.
   - Verify `Hero1AxeVFXPlan.md` has a current-status snapshot that does not describe the older static crescent as the live current state, while any older wording remains under `Superseded` or unchanged historical context.
   - Verify the three infrastructure-only Hero 1 future-effect packets each state that no active production binding row exists yet.
10. Create `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/staged_manifest.md`, including:
   - AGENTS.md insertion anchor proof,
   - every staged file with rationale,
   - hunk-level classification for allowlisted files that were already modified before this pass: `added by this pass`, `pre-existing VFX baseline hunk intentionally staged`, `intentionally unstaged`, or `blocker for Pablo`,
   - any allowed-file hunks intentionally left unstaged,
   - identity check for the generated/runtime VFX assets plus staged weapon hitbox data,
   - LFS attribute and staged pointer-vs-blob checks for the three staged `.uasset` files,
   - line-ending state for `Content/Data/CombatVFXBindings.csv`, using `git ls-files --eol` where possible and a fallback byte scan if the file is still untracked,
   - BOM state for `Content/Data/CombatVFXBindings.csv` from the fallback byte scan while the file is untracked,
   - quoted final AGENTS.md bullet,
   - quoted AGENTS.md insertion anchor as actually staged,
   - decision artifact excerpt matching the staged scope,
   - `git diff --cached --stat`,
   - confirmation that `git diff --cached --name-only` is a subset of the allowed staged paths,
   - confirmation that modified/deleted paths outside the approved list remain unstaged.
11. Re-read the staged manifest and confirm every allowlisted path is either present and classified or explicitly noted as not modified by this pass.
12. Run Claude staged-diff review before local commit.
13. Commit locally only.

Draft commit message:

```text
Codify durable combat VFX pipeline

- add combat VFX process index, DoD, packet template, generated asset policy, and idol overlay architecture
- document infrastructure-only Hero 1 DOT/Pierce/Bounce packet path and production-path proof expectations
- generalize Combat VFX binding/evidence helpers while preserving Hero 1 AOE production checks
- update root AGENTS.md goal/decision-block protocol and report artifact routing to prevent repeated clarification loops and keep proof artifacts under Reports/
- preserve approved Hero 1 AOE generated/runtime VFX assets and hitbox data: Content/Data/CombatVFXBindings.csv, Content/Data/DT_CombatVFXBindings.uasset, Content/Data/Weapons.csv, Content/Data/DT_Weapons.uasset, and Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset
- decision scope recorded in Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md
```

## Reviewer Request

Review this implementation plan before edits. Identify missing scope, risky file ownership, inadequate verification, conflicts with user answers, or places where the plan silently narrows or expands the approved VFX-only baseline. First non-empty line must be exactly `Verdict: APPROVE`, `Verdict: REVISE`, or `Verdict: BLOCK`.
