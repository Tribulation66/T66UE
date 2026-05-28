Verdict: APPROVE

## Blockers

None. Verification, scope justification, LFS handling, deletion guard, and Mini-scope guard are all explicitly covered, and Pablo's authorization quotes cover both the implementation and the AGENTS.md/process scope.

## Major Issues

- Internal consistency between new rule and parent `.report-run.json`. The staged root AGENTS hunk states "New raw run folders should include `.report-run.json` with `expiresAfterDays: 15`," yet `AutoFrameSelection_Hero1AxeAOE_EdgeFinal`, `BindingValidatorSelfTest`, and `EvidenceBundleSelfTest` are sibling raw output folders without their own marker. The packet rationalizes this as a single proof run with a parent marker, but the rule the same commit is codifying reads as per-folder. This will read as inconsistent on the next cleanup pass. Either add `.report-run.json` (or a short pointer file) inside each child folder, or tighten the AGENTS.md wording to explicitly allow a parent marker covering child evidence folders.

## Minor Issues

- Weapons.csv 386-line churn is consistent with 193 lines × 2 (one column added per row) and the structured diff summary, but the deletion accounting line "386 +++…--- " looks alarming at a glance. Worth a one-line note in the commit body or `WeaponsCsvAoeInnerRadiusDiff_summary.txt` explicitly stating "every row's tail edited to append the new column; expected ~193 inserts and ~193 deletes."
- The MASTER_COMBAT.md hunk both bumps "Last updated" and appends a new Combat-VFX paragraph plus a companion-doc reference. The packet calls this an intentional VFX router note, but the added paragraph blends policy ("Production-path automation proof must use real weapon selection…") into a status doc. Acceptable, but consider keeping the policy sentence in `VFX_PROCESS_INDEX.md` or `CombatVFXDefinitionOfDone.md` and leaving the router with a pure pointer; reduces drift risk on future MASTER_COMBAT.md edits.
- `Gameplay/Combat/Hero1AxeVFXPlan.md` is staged but is not listed in the Verification Already Run section. A one-line note that this doc was reconciled against the AOE proof references and DOT/Pierce/Bounce infrastructure-only stance would close the loop.
- `pending_issues_Combat.md` is newly added but the packet only proves the Scripts pending diff; a quick note that the Combat pending file contains VFX-only entries (no Mini, no unrelated combat threads) would mirror the Scripts proof.
- Draft commit message body does not explicitly call out the new `Reports/Proof/CombatVFX/VFXDurableBaseline_20260528` durable proof tree. Optional, but useful for future archaeology.

## Clarifying Questions

- For the parent `.report-run.json` decision: is the intent that child evidence folders inherit the parent's `expiresAfterDays`, or is the parent run folder itself the unit that expires (and the children with it)? The cleanup rule's reading depends on this.
- The packet states `Gameplay/Combat/CombatVFXAuthoringProcedure.md` is retained in MASTER_COMBAT.md's companion list; confirm this file is pre-existing in HEAD and is not being silently replaced by `VFX_PROCESS_INDEX.md` / `CombatVFXDefinitionOfDone.md`.
- Two `pass9` review artifacts are staged into the same commit they are reviewing. Confirm this is the intended pattern (durable review record committed with the change) and not a leftover from an earlier pass.

## Required Verification

Before Codex runs `git commit`:

- `git diff --cached --name-only` matches the Staged Name Status block exactly (no extra files picked up between packet write and commit).
- `git diff --cached --stat` matches the Staged Short Stat (57 files, 6252/200).
- `git status` still shows the QuickRevive/Cliffs/Vending/ToonStyle deletions as unstaged (intentional exclusion intact).
- `git diff --cached -- Content/Data/Weapons.csv | rg "^[-+]" | rg -v "^(---|\+\+\+)" | rg -v ",0\.54," | rg -v "AoeInnerRadiusRatio"` to confirm no non-AoeInnerRadiusRatio value churn slipped in (or rerun the existing structured diff and reattach summary).
- `git diff --cached -- Gameplay/Combat/MASTER_COMBAT.md` shows only the two hunks reproduced in the packet (router area only; no body churn).
- `git lfs ls-files --name-only` after stage but before commit shows `Content/Data/DT_CombatVFXBindings.uasset`, `Content/Data/DT_Weapons.uasset`, and `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset` as LFS pointers.
- Use plain `git commit -m "$(message)"` (or `git commit -F file`); explicitly do not use `-a`, `-A`, `add -A`, or `add .`.
- After commit: `git log -1 --stat` to confirm the recorded file set and shortstat match the packet, and `git push --dry-run` is **not** run (local-only intent per scope).

## Rationale

The packet is thorough: every cross-cutting scope expansion (root AGENTS.md, MASTER_COMBAT.md router note, Weapons.csv schema, generated runtime assets) is paired with an explicit Pablo authorization quote and a structured proof artifact. The validator and evidence-bundle self-tests pass, the production-path Unreal commandlet log is captured, the LFS pointer state is verified, and the working-tree-deletion exclusion is enforced by committing with plain `git commit` rather than broad staging. The "VFX-only" label is loose, but the packet calls that out and the draft commit body reflects it. The one durable consistency issue is the parent vs per-folder `.report-run.json` framing — worth resolving but not severe enough to block a local-only commit, since nothing pushes off the machine.
