# Final Confirmation Points

Date: 2026-05-27

These are the points the final user-facing report must foreground because they are more nuanced than the original shorthand request.

## Movement Cleanup

- The user asked to delete dead movement hooks.
- The actual Gate A movement work removed live source callsites for wrappers that were functionally neutral:
  - `GetMovementSpeedSecondaryMultiplier()` returned literal `1.f`.
  - The movement formula removed only a neutral `* 1.f` factor.
- Evidence:
  - `Reports/Hygiene/2026-05-27/movement_run_summary_equivalence.md`
  - `Reports/Hygiene/2026-05-27/hero_movement_qa_verification.json`
  - `Reports/Hygiene/2026-05-27/run_summary_smoke_verification.json`
- Final answer wording: this was a live-callsite no-op cleanup, not a pure "already unreferenced symbol" deletion. Ask the user to confirm this interpretation when they are back if they wanted a stricter rule.

## Run Summary UI Row

- The old Run Summary `Move Speed Mult` row displayed the removed neutral `1.0x` value.
- The cleanup intentionally removed that visible row instead of retaining or replacing it.
- Evidence:
  - `Reports/Hygiene/2026-05-27/run_summary_smoke_verification.json`
  - `Reports/Hygiene/2026-05-27/movement_run_summary_equivalence.md`
- Final answer wording: this is a user-visible UI cleanup and should be confirmed separately from the movement behavior no-op.

## Report Retention

- The user asked to delete whole run folders after 15 days.
- The implemented policy keeps the 15-day whole-folder threshold but adds safety guards:
  - a durable summary must exist outside the raw run folder
  - no active references may still point at the raw run folder
- Evidence:
  - `AGENTS.md`
  - `Reports/README.md`
  - `Reports/AGENTS.md`
- Final answer wording: this is a conservative implementation of the 15-day cleanup rule. Ask the user to confirm whether they want this conditional rule or literal unconditional deletion after 15 days.

## Preserved Generated-Looking Folders

- `Saved/StagedBuildsDemo`, `Saved/StagedBuilds/WindowsHotfix`, and `Saved/StagedBuilds/WindowsTemp` were preserved because no regeneration owner was proven.
- `Saved/D2` was preserved because it appears to be material/VFX experiment output but is not fully identified.
- Final answer wording: these should be left as follow-up cleanup candidates, not claimed as cleaned.

## Authoritative Scope Boundary

- `Reports/Hygiene/2026-05-27/hygiene_change_scope.md` is the authoritative per-pass change/delete list.
- `Reports/Hygiene/2026-05-27/final_diff_scope.txt` is only a broad mixed diff inventory and includes unrelated pre-existing dirty files.
