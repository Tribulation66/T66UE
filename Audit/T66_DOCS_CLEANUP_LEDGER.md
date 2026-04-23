# T66 Docs Cleanup Ledger

**Date:** 2026-04-12  
**Purpose:** First-pass Markdown cleanup inventory for the repo-wide cleanup project.  
**Rule:** No file should be deleted until it is classified here as either:

- `Delete now`: superseded and not needed as active reference
- `Delete with paired edits`: safe to remove only if references are updated in the same pass
- `Keep`: still active, canonical, or materially useful

## 1. Canonical Keep Set

These are the current source-of-truth docs and should remain:

- `MASTER DOCS/README.md`
- `MASTER DOCS/T66_MASTER_GUIDELINES.md`
- `MASTER DOCS/T66_PROJECT_CATALOGUE.md`
- `MASTER DOCS/T66_DECISION_LOG.md`
- `MASTER DOCS/T66_IMPORT_PIPELINE_GUIDELINES.md`
- `MASTER DOCS/MASTER_BACKEND.md`
- `MASTER DOCS/MASTER_COMBAT.md`
- `MASTER DOCS/MASTER_LIGHTING.md`
- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `MASTER DOCS/MASTER_TRAPS.md`
- `ANTI_CHEAT/MASTER_ANTI_CHEAT.md`
- `ANTI_CHEAT/ANTI_CHEAT_IMPLEMENTATION_CHECKLIST.md`

## 2. Keep As Active Supporting Docs

These are not master docs, but they still look useful as active references:

- `Docs/README.md`
  - Keep, but it should be updated later to reflect the actual archive strategy in the repo.
- `Docs/Plans/T66_UI_IMPLEMENTATION_PLAN.md`
  - Still useful as a detailed implementation reference tied to the UI audit.
- `Docs/Systems/T66_Console_Commands.md`
  - Useful operational reference and not clearly superseded by a master doc.
- `Docs/UI/ChatGPT_Image_Production_Workflow.md`
- `Docs/UI/MasterStyle.md`
- `Docs/UI/Item_Icon_Prompt_Pack.md`
- `Docs/UI/Secondary_Buff_Icon_Prompt_Pack.md`
- `Docs/UI/Accuracy_Family_Item_Icon_Prompt_Pack.md`
- `Docs/UI/Accuracy_Item_And_TempBuff_Audit.md`
- `Docs/UI/Accuracy_Item_Icon_Prompt_Pack.md`
- `Docs/UI/Accuracy_Secondary_Buff_Icon_Prompt_Pack.md`
- `Audit/T66_UI_AUDIT.md`
- `Audit/T66_PACKAGING_CLEANUP_TRACKER.md`
- `Audit/PERFORMANCE_AUDIT.md`

## 3. Delete Now Candidates

These are superseded, legacy, or explicitly deprecated with no clear need to keep them as active repo docs:

- `Guidelines/Archive/guidelines_legacy.md`
  - Superseded by `MASTER DOCS/T66_MASTER_GUIDELINES.md`.
- `Guidelines/Archive/memory_legacy.md`
  - Legacy agent context; not the current project truth.
- `Docs/UI/Secondary_Buff_Icon_Prompt_Pack_remaining.md`
  - Explicitly marked as deprecated overflow scratch material.
- `Docs/Systems/T66_Preview_Stage_Handoff.md`
  - Explicitly called out by `MASTER DOCS/MASTER_LIGHTING.md` as no longer the current source of truth.
- `Docs/Systems/T66_Stages.md`
  - Old design appendix, not part of the current master-doc structure.
- `Docs/Plans/T66_Procedural_Landscape_Plan.md`
  - Historical implementation plan; the implemented runtime truth now lives in `MASTER DOCS/MASTER_MAP_DESIGN.md`.
- `Audit/Audit_03-31-2026.md`
  - Older diagnostic snapshot, superseded by more current targeted audits.
- `Audit/audit 3.16.26.md`
  - Older performance audit snapshot, superseded by `Audit/PERFORMANCE_AUDIT.md` as the cleaner active reference.

## 4. Delete With Paired Edits

These are likely removable, but only if we update the remaining docs that still point to them:

- `Docs/Systems/lighting.md`
  - Still referenced in `MASTER DOCS/MASTER_LIGHTING.md` and `MASTER DOCS/MASTER_MAP_DESIGN.md`.

Recommended paired edit:

- remove or rewrite the historical/input references in the relevant master docs
- keep any important surviving facts in the master docs before deleting the old files

## 5. Hold For Later Review

These are not obvious delete targets yet:

- `Docs/Systems/lighting.md`

Reason:

- they contain project history that was recently consolidated into master docs
- they can likely be removed, but that should happen in the same pass as reference cleanup

## 6. Proposed Docs Cleanup Order

1. Delete the `Delete now` candidates.
2. Update master-doc references that still point at old system docs.
3. Delete the `Delete with paired edits` candidates.
4. Revisit whether older audit reports should be removed or kept as historical records.

## 7. First-Pass Recommendation

The safest first destructive docs pass is:

- delete all files listed in `Delete now candidates`
- leave the lighting system doc until the master-doc references are cleaned in the same change

## 8. Completed Follow-Up

- `Docs/Systems/backend.md` was moved to `Docs/Archive/Systems/backend_architecture_historical.md`.
- `Docs/Systems/backend_0.1_checklist.md` was moved to `Docs/Archive/Systems/backend_0.1_checklist_historical.md`.
- `Docs/Systems/version_0.1_full_checklist.md` was moved to `Docs/Archive/Systems/version_0.1_full_checklist_historical.md`.
- `MASTER DOCS/MASTER_BACKEND.md` was updated so those files are no longer treated as active input sources.
