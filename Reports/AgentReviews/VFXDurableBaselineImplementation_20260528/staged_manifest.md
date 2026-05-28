# VFX Durable Baseline Staged Manifest

Date: 2026-05-28

## Scope Decision Source

Decision artifact: `Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md`.

Approved scope captured there:

```text
DOT/Pierce/Bounce scope: infrastructure only now.
Normal proof: production-path automation proof first.
Generated asset policy: combat-VFX-local policy now, repo-wide generated asset policy out of scope for this pass.
Idol overlay scope: architecture-only seam document now; no idol VFX assets, active idol rows, or implemented idol behavior.
Existing generated/runtime VFX data/assets: include the approved Hero 1 AOE production binding/runtime assets in the local VFX-only commit after staged-diff review; do not author new visual VFX assets in this pass.
Automated best-frame selection: in scope because the user listed it under "What Is Missing"; implement as opt-in evidence tooling only, preserving current manual/default behavior.
Root AGENTS.md update: in scope because Pablo explicitly said the repeated-question goal change needs to be reflected in AGENTS.md.
```

This is the quoted user-approval source for committing `Content/Data/CombatVFXBindings.csv`, `Content/Data/DT_CombatVFXBindings.uasset`, and `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`.

Additional authorization excerpts now saved in `decision_block.md`:

```text
Root AGENTS.md scope: user said, "Ok this goal change needs to be reflected in the agents.md file, because this occurred in a different agent as well, with the repeated questions."
Implementation permission and narrowed decisions: user said, "1. Infrastructure only. 2. B. 3. B. Ok you have permission to go with the implementation now."
```

## AGENTS.md Anchor

Staged insertion anchor:

```markdown
- User constraints, planning-only boundaries, and repository instructions override convenience. If the user changes scope, update, close, or replace the active function-created goal with the available goal controls before proceeding; if the environment cannot update the function goal, state the replacement working goal in the conversation and discard stale assumptions.
```

Staged new bullet:

```markdown
- Working goals must describe the full requested end state, not a temporary clarification gate. If a decision gate is needed, ask it once and, for durable work, save it as `Reports/AgentReviews/<TaskSlug>/decision_block.md`; on continuations, reference the saved gate instead of repeating questions. Mark `blocked` only when no safe default, instruction, or user-approved decision can move the full goal forward.
```

Classification: approved process-router baseline. The staged `AGENTS.md` also includes current report-routing policy already present in the working router.

## Staged Files

Root/process:

```text
AGENTS.md
Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md
Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/implementation_plan.md
Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/20260528T075159-pass9/claude_review_prompt_pass9.md
Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/20260528T075159-pass9/claude_review_pass9.md
Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/staged_manifest.md
```

Combat VFX docs:

```text
Gameplay/Combat/VFX_PROCESS_INDEX.md
Gameplay/Combat/CombatVFXDefinitionOfDone.md
Gameplay/Combat/EffectPacketTemplate.md
Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md
Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md
Gameplay/Combat/Hero1AxeDOTMechanismPacket.md
Gameplay/Combat/Hero1AxePierceMechanismPacket.md
Gameplay/Combat/Hero1AxeBounceMechanismPacket.md
Gameplay/Combat/Hero1AxeVFXPlan.md
Gameplay/Combat/CombatVFXInfrastructureInventory.md
Gameplay/Combat/pending_issues_Combat.md
Gameplay/Combat/MASTER_COMBAT.md
```

Runtime/source assets and scripts:

```text
Content/Data/CombatVFXBindings.csv
Content/Data/DT_CombatVFXBindings.uasset
Content/Data/Weapons.csv
Content/Data/DT_Weapons.uasset
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset
Scripts/BuildT66VideoEvidenceBundle.py
Scripts/CaptureT66GameplayVideo.ps1
Scripts/RunHero1AxeAOEVFXBindingProof.ps1
Scripts/ValidateCombatVFXProductionBindings.py
Scripts/pending_issues_Scripts.md
```

Proof packet:

```text
Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/**
```

Rationale: this proof packet contains `.report-run.json`, `PROOF_SUMMARY.md`, script self-test outputs, auto-frame selector evidence from a retained real capture, Unreal validator logs, print-only wrapper outputs, and generated/runtime asset identity snapshots. Synthetic self-test raw frames/contact sheets/video are intentionally not staged because `EvidenceBundleSelfTest_output.txt` and `EvidenceBundleSelfTest/self_test_report.json` are the durable evidence and the raw synthetic files are regenerable.

## Hunk Classification

- `AGENTS.md`: staged root goal/decision-block fix plus report routing from the current working router. Classification: approved process baseline.
- `Gameplay/Combat/MASTER_COMBAT.md`: staged only the top VFX router hunk adding `VFX_PROCESS_INDEX.md` and the production-path automation proof note. Other existing combat doc hunks are intentionally unstaged.
- `Gameplay/Combat/Hero1AxeVFXPlan.md`: staged as the durable Hero 1 axe VFX status/plan doc; it records AOE as production-bound for backend/hitbox proof and DOT/Pierce/Bounce as infrastructure-only.
- `Gameplay/Combat/pending_issues_Combat.md`: staged as a Combat VFX-only pending/resolution file. It covers AOE slash reference supersession, editor-isolation capture, old evidence capture issues, AOE visual-polish deferral, and VFX auto-frame resolution.
- `Scripts/pending_issues_Scripts.md`: staged VFX capture timeout resolution and VFX auto-frame evidence resolution. Classification: VFX baseline handoff cleanup.
- `Content/Data/Weapons.csv` and `Content/Data/DT_Weapons.uasset`: staged because the Hero 1 AOE production validator and crescent-band hitbox contract depend on the new `AoeInnerRadiusRatio` source/runtime weapon data. This is VFX hitbox wiring, not unrelated weapon balance work.
- New Combat docs, scripts, report artifacts, and generated/runtime VFX assets: staged as new durable VFX baseline files.

Policy scope notes:

- The staged `AGENTS.md` Report Artifact Routing section is intentionally repo-wide process scope because this commit adds durable proof/report artifacts and fixes the repeated decision-gate loop that caused this VFX baseline to stall.
- The staged `MASTER_COMBAT.md` production-path automation proof sentence is an intentional Combat VFX policy note: production-path proof may be automation-driven when it exercises real weapon selection, RunState inventory/item stats, combat fire, VFX binding lookup, and damage paths; it is not a claim that literal UI-click proof has already been added for every future packet.
- Commit title/body must make this explicit: the local VFX baseline commit also changes repo-wide `AGENTS.md` goal/decision-block behavior and root report-artifact routing.

## LFS And CSV Checks

Attributes:

```text
Content/Data/CombatVFXBindings.csv: filter: unspecified
Content/Data/CombatVFXBindings.csv: diff: unspecified
Content/Data/CombatVFXBindings.csv: merge: unspecified
Content/Data/CombatVFXBindings.csv: text: unspecified
Content/Data/DT_CombatVFXBindings.uasset: filter: lfs
Content/Data/DT_CombatVFXBindings.uasset: diff: lfs
Content/Data/DT_CombatVFXBindings.uasset: merge: lfs
Content/Data/DT_CombatVFXBindings.uasset: text: unset
Content/Data/DT_Weapons.uasset: filter: lfs
Content/Data/DT_Weapons.uasset: diff: lfs
Content/Data/DT_Weapons.uasset: merge: lfs
Content/Data/DT_Weapons.uasset: text: unset
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: filter: lfs
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: diff: lfs
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: merge: lfs
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset: text: unset
```

Staged `.uasset` pointer check:

```text
Content/Data/DT_CombatVFXBindings.uasset -> LFS pointer oid sha256:71d011b98c5db0fde43a57dbcf4a759b53e73d30e415a13eb6bf03a3221fa8b2, size 3849.
Content/Data/DT_Weapons.uasset -> LFS pointer oid sha256:820b6439833d7e71b21a5588b92ce2d79270ece265680280d97a1b572fbabb4d, size 272408.
Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset -> LFS pointer oid sha256:3025881ef90532feaa3453b27c097ad2861c797a04e7e8cc6eb43a3911bbaaf4, size 1772344.
```

Pointer command evidence:

```text
git cat-file -p :Content/Data/DT_CombatVFXBindings.uasset
version https://git-lfs.github.com/spec/v1
oid sha256:71d011b98c5db0fde43a57dbcf4a759b53e73d30e415a13eb6bf03a3221fa8b2
size 3849

git cat-file -p :Content/Data/DT_Weapons.uasset
version https://git-lfs.github.com/spec/v1
oid sha256:820b6439833d7e71b21a5588b92ce2d79270ece265680280d97a1b572fbabb4d
size 272408

git cat-file -p :Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset
version https://git-lfs.github.com/spec/v1
oid sha256:3025881ef90532feaa3453b27c097ad2861c797a04e7e8cc6eb43a3911bbaaf4
size 1772344
```

CSV state:

```text
git ls-files --eol: i/lf w/crlf attr/ Content/Data/CombatVFXBindings.csv
BOM=False CRLF=True LFOnly=False Bytes=551
```

Generated/runtime asset identity is recorded in:

```text
Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/generated_runtime_assets_identity_prevalidation.json
Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/generated_runtime_assets_identity_postvalidation.json
```

Result: pre/post hashes, sizes, and mtimes match for the original three generated/runtime VFX assets. The staged weapon-data hitbox files are additionally identified as `Content/Data/Weapons.csv` SHA256 `EB5AC1DBBA0FCAF91198A41C1378EF9A2953A73DB1304A6E35AEDABB9DB44408`, 60403 bytes, and `Content/Data/DT_Weapons.uasset` SHA256 `820B6439833D7E71B21A5588B92CE2D79270ECE265680280D97A1B572FBABB4D`, 272408 bytes.

## Weapon Data Scope Proof

Structured CSV comparison output is saved at:

```text
Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/WeaponsCsvAoeInnerRadiusDiff_summary.txt
```

Result:

```text
Old columns: 21
New columns: 22
Added columns: AoeInnerRadiusRatio
Removed columns: none
Old rows: 192
New rows: 192
Added rows: none
Removed rows: none
Non-added-column value changes: 0
Non-zero AoeInnerRadiusRatio values: 1
Non-zero row: Hero_1_black_aoe = 0.54
```

`git diff --cached --stat -- Content/Data/DT_Weapons.uasset` output:

```text
Content/Data/DT_Weapons.uasset | 4 ++--
1 file changed, 2 insertions(+), 2 deletions(-)
```

Interpretation: `Content/Data/Weapons.csv` only adds the `AoeInnerRadiusRatio` column and the Hero 1 black AOE inner-radius value required by the crescent-band VFX hitbox contract; `Content/Data/DT_Weapons.uasset` is staged as the matching generated runtime DataTable.

## Verification

- `python Scripts/BuildT66VideoEvidenceBundle.py --self-test-root Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/EvidenceBundleSelfTest`: passed; see `EvidenceBundleSelfTest_output.txt`.
- `python Scripts/ValidateCombatVFXProductionBindings.py --self-test-root Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/BindingValidatorSelfTest`: passed; see `BindingValidatorSelfTest_output.txt`.
- Unreal production binding validator commandlet: passed with completion token in `ValidateCombatVFXProductionBindings_Unreal.log` and no `LogPython: Error`, `LogOutputDevice: Error`, or `Traceback`.
- Auto-frame selector against retained Hero 1 AOE proof frames: wrote `AutoFrameSelection_Hero1AxeAOE_EdgeFinal/manifest.json` with `selection_method=auto_activity` and selected `start=51`, `mid=56`, `impact=62`, `dissipate=71`.
- `Scripts/RunHero1AxeAOEVFXBindingProof.ps1 -PrintOnly`: normalized post-change output matches pre-change proof; see `RunHero1AxeAOEVFXBindingProof_printonly_normalized_diff.txt`.
- `Scripts/RunHero1AxeAOEVFXBindingProof.ps1 -PrintOnly -EvidenceAutoSelectFrames`: smoke output saved at `RunHero1AxeAOEVFXBindingProof_printonly_autoframe.txt`.
- Existing normal-flow proof references are preserved in `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/PROOF_SUMMARY.md`: `Saved/VideoCaptures/Hero1AxeAOE_VFXBindingProof_20260528_001540/Hero1AxeAOEVFXBindingProofSummary.md` for real weapon selection, item stats, VFX binding lookup, combat fire, and damage path evidence; and `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/` for crescent-band visual/hitbox proof.
- `Content/Data/CombatVFXBindings.csv` row check: exactly one row is active, `Hero1Axe_AOE_Base|WeaponBase|Hero_1_black_aoe|AOE|/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash|Hero1AxeAOESlashMechanismPacket`; there are no DOT/Pierce/Bounce/idol overlay rows or non-empty asset paths for deferred future effects.
- Mini scope guard: `git diff --cached --name-only | Select-String -Pattern 'Mini|Minigame'` returned no output.
- Future-effect asset guard: `rg "Content/VFX|\\.uasset|NiagaraSystem|/Game/VFX" Gameplay/Combat/Hero1AxeDOTMechanismPacket.md Gameplay/Combat/Hero1AxePierceMechanismPacket.md Gameplay/Combat/Hero1AxeBounceMechanismPacket.md Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` returned no output.
- `Scripts/pending_issues_Scripts.md` hunk proof is saved at `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/ScriptsPendingIssuesDiff_snippet.txt`; it appends exactly two VFX-resolved entries.
- `Scripts/RunHero1AxeAOEVFXBindingProof.ps1 -PrintOnly` normalized output includes the capture wrapper command/argument lines and matches the pre-change proof by default; therefore the `CaptureT66GameplayVideo.ps1` auto-frame route is opt-in and default wrapper behavior remains unchanged for the production proof path.
- `Reports/AGENTS.md` already routes proof runs to `Reports/Proof/<Domain>/<TaskSlug>` and says new raw run folders should include `.report-run.json`; the staged root `AGENTS.md` Report Artifact Routing section mirrors that policy and does not introduce a conflicting section anchor.
- `.report-run.json` exists at the parent `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528/` run folder and in each child evidence folder: `AutoFrameSelection_Hero1AxeAOE_EdgeFinal`, `BindingValidatorSelfTest`, and `EvidenceBundleSelfTest`.

## Staged Diff Stat

```text
62 files changed, 6574 insertions(+), 200 deletions(-)
```

Deletion accounting from `git diff --cached --stat`:

```text
AGENTS.md                         |  14 +-
Content/Data/DT_Weapons.uasset    |   4 +-
Content/Data/Weapons.csv          | 386 +++++++++++++++++++-------------------
Gameplay/Combat/MASTER_COMBAT.md  |   6 +-
Scripts/pending_issues_Scripts.md |  13 ++
5 files changed, 223 insertions(+), 200 deletions(-)
```

## Allowlist Notes

`Scripts/CaptureT66GameplayVideo.ps1` is staged as an implementation-plan addendum because it is the required opt-in wrapper seam for `BuildT66VideoEvidenceBundle.py --auto-select-frames`. The script parameter comment states this is an opt-in evidence packaging helper only and does not change gameplay capture defaults.

## Unstaged Paths

Unrelated modified/deleted paths remain unstaged. The intentionally unstaged set includes pre-existing content/code/doc churn outside the VFX durable baseline, plus non-VFX hunks still present in `Gameplay/Combat/MASTER_COMBAT.md`.

The current working tree has unstaged deletions under `Content/UI/Sprites/Interactables/QuickReviveIcon.uasset`, `Content/World/Cliffs/*`, `Content/World/Interactables/Vending/*`, `Source/T66/Gameplay/T66QuickReviveVendingMachine.*`, and `ToonStyle/Reports/**`. These are intentionally not part of this VFX commit and must remain unstaged; use plain `git commit`, not `git commit -a`, `git add -A`, or broad staging.

Follow-up handling for those deletions is outside this VFX baseline. They remain for a separate cleanup/hygiene pass owned by their respective systems.

## Commit Command Constraint

Use a plain commit command against the current index:

```text
git commit -m "Codify durable combat VFX pipeline" -m "<body below>"
```

Do not use `git commit -a`, `git add -A`, `git add .`, or any broad staging command before commit.

Draft commit message body:

```text
- add combat VFX process index, DoD, packet template, generated asset policy, and idol overlay architecture
- document infrastructure-only Hero 1 DOT/Pierce/Bounce packet path and production-path proof expectations
- generalize Combat VFX binding/evidence helpers while preserving Hero 1 AOE production checks
- update root AGENTS.md goal/decision-block protocol and report artifact routing to prevent repeated clarification loops
- update MASTER_COMBAT.md to route Combat VFX work through the new process index and production-path proof contract
- preserve approved Hero 1 AOE generated/runtime VFX assets and hitbox data: Content/Data/CombatVFXBindings.csv, Content/Data/DT_CombatVFXBindings.uasset, Content/Data/Weapons.csv, Content/Data/DT_Weapons.uasset, and Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset
- decision scope recorded in Reports/AgentReviews/VFXDurableBaselineImplementation_20260528/decision_block.md

Local commit only; no push.
```
