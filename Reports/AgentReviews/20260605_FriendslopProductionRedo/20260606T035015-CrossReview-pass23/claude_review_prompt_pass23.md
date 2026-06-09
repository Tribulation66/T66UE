You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\pass14_direct_reference_plate_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\pass14_codex_result_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T030208-IndependentAnswer-pass22\claude_review_pass22.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Pass14 Direct Reference Plate Continuation Prompt

Original handoff: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\fresh_agent_main_menu_pass14_prompt_final.md`

Decision block: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\decision_block.md`

User decision:

```text
1. Allow direct reference derived runtime plates with strict gates, no API. Go on and continue
```

Task for Validator:

Provide an independent repo-grounded answer for continuing pass14 under this approved policy change. Codex should derive runtime plates from exact reference crops, remove only live-content zones locally, run strict component gates, and avoid API/CLI/`OPENAI_API_KEY`. Identify any critical risks or stop conditions before runtime wiring. Confirm whether the pass14 contract/pass log should record this user-approved exception to the prior "no crop/masked runtime plates" boundary.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The user picked decision-block option 1: derive runtime plates from exact Round06 reference crops, remove only the masked live-content zones locally, run the strict reference component gate, and ship a plate only on a true PASS. No API/CLI/`OPENAI_API_KEY`. This is approved policy — proceed.

**Yes, record the exception explicitly.** The prior "do not ship crop/masked runtime plates" boundary lives in three places that currently still forbid it, so they must be reconciled before any plate is wired, or the next agent will correctly self-block:

1. `decision_block.md` lines 20-24 and the pass14 contract addendum lines 16-18, 24 ("not permission to ship direct cropped reference pixels as runtime plates").
2. `fresh_agent_main_menu_pass14_prompt_final.md` line 115 ("Do not repeat... using crop/inpaint/masked plates as runtime assets").
3. The pass log's Pass12 entries and the Pass09 posthoc invalidation, which is the institutional reason the no-crop rule exists.

Record the exception as a dated, scoped amendment (user-approved 2026-06-06) in:
- `pass_log.md` — a new Pass14 entry noting the user-approved boundary change and citing the decision-block + user-decision text.
- `main_menu_pass14_component_contract_addendum.md` — amend the "Corrected Pass14 Extraction Rule" so the crop-derived plate path is permitted *under the gate*, superseding the current "not permission to ship" wording.

Keep the amendment narrow: crop-*derived* plates with live-content zones removed, gated, are now allowed; raw unmodified crops and API/CLI remain forbidden.

**Critical risks / stop conditions before runtime wiring:**

- **Gate currently fails everything.** `pass14_candidate_component_gate_report.md` shows all six families auto-FAIL (IoU 0.14–0.66, every Manual Status = REVIEW, `Result: FAIL`). Policy approval does not lower the bar — no plate is wired until its row flips to PASS with no remnant/smudge. The approval changes the *source method*, not the gate threshold.
- **Content-removal completeness.** The whole failure history (Pass09/Pass12 false accepts) came from partially-erased reference children. Every text/glyph/skull/score/avatar/checkbox zone in the mask must be fully blanked, or it is the same "looks masked/painted over" failure the user rejected.
- **Acceptance precedence unchanged.** `VerifyUIFidelity` `FAIL=0` is still not acceptance; `pass14_visual_scorecard.md Result: PASS` is the holistic gate (addendum lines 87-94). Do not call pass14 done on structural counts.
- **Title is out of scope for this exception.** Per addendum/parity gate, the title becomes live layered Slate text, not a crop plate — keep it that way.
- **No silent fallback.** If crop-derived + gate still can't pass a family, stop for the user rather than shipping a lookalike or reaching for API/CLI.

## Evidence Checked

- `pass14_direct_reference_plate_prompt.md` (the prompt under review)
- `decision_block.md` — option 1 text and the three blocking reasons
- `fresh_agent_main_menu_pass14_prompt_final.md` — pass14 contract, no-crop mistake list, acceptance bar
- `pass14_reference_component_gate/pass14_candidate_component_gate_report.md` — all 6 families FAIL/REVIEW, `Result: FAIL`
- `main_menu_pass14_component_contract_addendum.md` — current extraction rule still says crops are not shippable
- `pass_log.md` — Pass09 posthoc invalidation and Pass12 crop-plate history (origin of the no-crop rule)

## Questions Or Blockers

None requiring the user — the decision is already made. The work is Codex's to execute. Flagging only that the documentation reconciliation above is a prerequisite, not optional.

## Caveats

- I did not visually inspect the crop/mask/contact images themselves; the gate report's numeric FAILs are my basis for the "everything currently fails" claim.
- "Strict gate" is only meaningful if the gate thresholds aren't quietly relaxed to make the approved method pass; verify the gate script wasn't loosened alongside the policy change.
- The exception is scoped to the Main Menu pass14 families. It should not be read as a global FriendslopStyle license to ship crop-derived plates on future screens without the same gate.

</validator_independent_answer>

<codex_draft>
# Codex Pass14 Result Draft

Working task:
Operator: Codex
Validator: Claude
Scope: Continue FriendslopStyle Main Menu pass14 from pass13 partial state, using the user-approved direct reference-derived runtime plate exception with strict gates and no API/OPENAI_API_KEY fallback.
Stop condition: produce current pass14 artifacts, verification evidence, and honest acceptance status. Do not call strict DONE if visual scorecard, responsive, or manual gates remain open.

## User Approval

The user approved option 1: direct reference-derived runtime plates are allowed for Main Menu pass14 only, with strict gates and no API fallback.

## Main Changes

- Added `UI/FriendslopStyle/Elements/main_menu_pass14_component_contract_addendum.md` with the direct-reference exception, ownership rules, component gate, title containment, and required scorecard rows.
- Updated `UI/FriendslopStyle/Checklists/main_menu_checklist.md` with explicit title/subtitle/CTA/friend-row/local-row containment rows and live-title content rows.
- Added direct reference-derived blank chrome plates after v4 component gate PASS:
  - `SourceAssets/UI/FriendslopStyle/MainMenu/cta_primary_round06.png`
  - `SourceAssets/UI/FriendslopStyle/MainMenu/cta_secondary_round06.png`
  - `SourceAssets/UI/FriendslopStyle/MainMenu/search_field_round06.png`
  - `SourceAssets/UI/FriendslopStyle/MainMenu/topbar_icon_dark_round06.png`
  - `SourceAssets/UI/FriendslopStyle/MainMenu/left_panel_round06.png`
  - `SourceAssets/UI/FriendslopStyle/MainMenu/leaderboard_panel_round06.png`
  - matching files under `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/`
- Updated runtime descriptor wiring in `Source/T66/UI/Style/T66FriendslopStyle.cpp` for `topbar_icon_dark_round06.png`.
- Updated `Source/T66/UI/Screens/T66MainMenuScreen.cpp`:
  - live layered title replaces cropped title bitmap path;
  - measured title/subtitle/CTA text fit;
  - CTA labels/icons are contained;
  - live Slate search magnifier replaces `?` placeholder.
- Added small compile unblock in `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` for missing TestRoom skeletal override symbols exposed by the rebuild; added `Source/T66/Gameplay/GameMode/pending_issues_GameMode.md` noting functional TestRoom proof remains out of UI scope.
- Updated process/registry/pass log docs:
  - `Saved/Codex/UI/FriendslopStyle/MainMenu/pass_log.md`
  - `UI/FriendslopStyle/friendslop_asset_registry.md`

## Plate Gate Evidence

- Gate root: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/`
- Final report: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_reference_component_gate/pass14_direct_reference_v4_component_gate_report.md`
- Result: PASS for six direct-derived plate families.

## Desktop Verification Evidence

- Build: `Scripts/StageStandaloneBuild.ps1 -SkipCook` succeeded after the compile unblock; staged exe ready at `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`; shortcuts refreshed.
- Capture: `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass14_fixture_capture.png`
- Dump: `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass14_fixture_dump_utf8.json`
- Component crop sheet: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_component_crop_sheet.png`
- Visual scorecard: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_visual_scorecard.md` with `Result: PASS` for the desktop fixture.
- Verifier report: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_verify_report.md`
- Verifier result: `PASS=264 FAIL=0 UNSURE=0`

## Responsive And Manual Status

- Responsive report: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass14_responsive_report.md`
- Responsive contact sheet: `Saved/Codex/UI/FriendslopStyle/MainMenu/Responsive/pass14_responsive_contact_sheet.png`
- Responsive result: NEEDS_WORK. 1600x900, 1366x768, 1280x720, and 2560x1440 look clean in the contact sheet; 3440x1440 exposes the existing fixed 1920x1080 canvas aspect-fit root, causing side bars and title/topbar pressure.
- Manual checklist: `Saved/Codex/UI/FriendslopStyle/MainMenu/manual_interaction_checklist.md`
- Manual result: PENDING_MANUAL_REVIEW. Automated metadata passes, but hands-on interaction results are not filled.

## Acceptance Statement

Automated desktop visual/data gate is clean for pass14. Strict DONE is not claimed because responsive Step K has an ultrawide NEEDS_WORK finding and manual Step L remains pending.

</codex_draft>
