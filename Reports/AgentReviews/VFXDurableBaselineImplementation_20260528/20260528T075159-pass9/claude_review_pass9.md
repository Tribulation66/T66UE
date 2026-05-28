Verdict: APPROVE

## Blockers

None.

## Major Issues

- **Provenance of staged Content assets is asserted but not evidenced.** The three untracked Content assets (`CombatVFXBindings.csv`, `DT_CombatVFXBindings.uasset`, `NS_Hero1AxeAOE_MeshSlash.uasset`) are described as "approved Hero 1 AOE production binding/runtime assets from the earlier VFX work" but the plan only fixes their identity via SHA256/size/mtime without pointing to the earlier approval artifact or capture log. The staged manifest should cite the prior packet/conversation where each was approved, not just hash-pin them.
- **Pre-change print-only capture was taken before this plan was reviewed.** `RunHero1AxeAOEVFXBindingProof_printonly_prechange.txt` is described as already captured under the approved proof folder. That is a pre-implementation action inside a Reports path that is part of the staged manifest. It is benign (read-only print mode), but the plan should explicitly note that this artifact was captured pre-approval as part of investigation and is not itself an implementation step gated on this review.

## Minor Issues

- **Documentation surface area is large for one pass.** Five+ new `Gameplay/Combat/*.md` files (VFX_PROCESS_INDEX, DefinitionOfDone, EffectPacketTemplate, GeneratedAssetPolicy, IdolOverlayArchitecture) plus three Hero 1 mechanism packet stubs. Decision_block.md justifies it, but Codex should be told not to duplicate procedural content across these files; route via reference, not copy.
- **Verification Step 3 lacks an explicit pass/fail contract.** It says to run auto-frame mode and save outputs but does not state what makes that run pass, unlike Steps 4/5/6 which have explicit pass tokens or diff rules. Add a contract such as "auto-frame manifest must record `selection_method=auto` and produce a non-empty contact sheet; failure exits non-zero."
- **Step 11 ("Re-read the staged manifest") is procedural, not verifying.** It cannot fail in any falsifiable way. Either drop it or convert it into a checklist with explicit assertions (each allowlisted path is `present-and-classified` or `unmodified-by-this-pass`).
- **`Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` risks scope creep.** Pablo approved an "architecture-only seam document." Codex should be reminded to keep this short and avoid sketching damage formulas, parameter names, or binding rows that would imply implementation commitment.
- **AGENTS.md style-check clause is vague.** "Same imperative voice, bullet length, and conditional wording" is judged subjectively; the deterministic rule should be: stage the authoritative bullet verbatim, and only deviate if staged-diff review explicitly approves named edits.
- **`Hero1AxeVFXPlan.md` "Superseded Historical Notes" section is conditional ("if it must be preserved").** Make it deterministic: either preserve specific older language under that section or do not add the section. Conditional sections produce review ambiguity.
- **`pending_issues_*.md` "no change is acceptable" wording is fine but the criteria "materially closes a documented gap" is judged by Codex.** Specify which existing gap IDs/headings, if any, are candidates for narrowing in this pass.

## Clarifying Questions

- Does the existing `Scripts/ValidateCombatVFXProductionBindings.py` already print the completion token `=== Combat VFX production binding validation DONE ===`, or is that token being introduced in this pass? If introduced, Step 5's gate would not match pre-existing log output and that should be called out.
- For Step 6, does the existing wrapper already support `-PrintOnly`, or is that flag being added in this pass? If added, the "diff against pre-change" is comparing a new code path against a pre-change file that may have been captured under a different code shape — explain how parity is preserved.
- The decision artifact says "include the approved Hero 1 AOE production binding/runtime assets in the local VFX-only commit after staged-diff review." Confirm: does that include the 1.77 MB Niagara `.uasset`, or only the binding CSV and DataTable? The plan stages all three; the decision artifact wording is ambiguous on whether the Niagara system itself is in this commit.

## Required Verification

The plan's Verification Plan (Steps 1–13) is adequate when combined with the following clarifications/additions:

- Step 5 must explicitly fail if the completion token is missing OR exit code is nonzero OR error patterns appear (already specified — good).
- Add an explicit assertion that `git diff --cached --name-only` is a strict subset of the allowed staged paths AND that no allowed path is staged with an unintended hunk type (already implied in Step 10 — keep it explicit).
- Add an explicit assertion that LFS pointer files (not blob content) are what got staged for the two `.uasset` files, via `git show :Content/...` showing the LFS pointer header.
- The pre-change print-only output diff in Step 6 should normalize only the documented timestamped tokens and absolute paths; any other normalization must be flagged in the manifest.

## Rationale

The plan is tightly scoped to the user-approved VFX-only baseline, treats DOT/Pierce/Bounce as infrastructure-only, keeps the idol seam document architecture-only, and constrains generated asset policy to combat-VFX-local. Safety controls are strong: an allowed-staged-paths allowlist, hunk-level classification for pre-existing modifications in allowlisted files, mandatory LFS pointer-vs-blob verification, a hard Unreal validator gate with explicit completion token and error pattern checks, a print-only proof diff to validate that existing MP4 proof remains representative, byte-for-byte preservation of default frame selection behavior with auto-frame opt-in only, mandatory Claude staged-diff review before commit, and local-commit-only with no push. The plan correctly respects AGENTS.md (no destructive cleanup of unrelated modified hunks, no Mini scope, full-end-state goal routing, decision artifact referenced). The remaining concerns are documentation hygiene, falsifiability of two verification steps, and provenance citation for the three pre-existing Content assets — none rise to a blocker. Codex should be reminded to keep new docs lean, to make Step 3 and Step 11 falsifiable, and to cite the prior approval for the three Content assets in the staged manifest.
