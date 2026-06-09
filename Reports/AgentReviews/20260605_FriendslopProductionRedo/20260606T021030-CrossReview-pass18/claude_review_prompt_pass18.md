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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\pass14_user_request_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\fresh_agent_main_menu_pass14_prompt_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T015901-IndependentAnswer-pass17\claude_review_pass17.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Pass14 User Request Prompt

User request:

> Ok so the too many requests I think was a bug, if you get that you should just close that codex CLI and deploy a new one. I tested the iamgegen request and it was fine. So I think you were really locked out of doing the changes that needed to be done, because of that errror, because there is still obvious masking, fitting errors, and etc. Also one thing to point out is even the title is cut out. Now whats really important here is that we are going to have to reproduce this process for every screen, generating a reference image and then getting the screen to look like it, so all of the solutions you are coming up with that work, like example how to fit stuff, in the panels, you need to be noting this down in the process doc, and as you come with solutions do not look for quick fixes but systematic approach solutions so the problem does not occur for future screens. This is done in parallel with working on this screen, its still not anywhere near completion, so you need to work on figuring out the final solutions for the problems (or implementing if you were simply blocked by imagegen) and once the screen is finalized and perfectly matches the reference image, then compelte the process doc.

Working task:
Operator: Codex
Validator: Claude
Scope: continue the FriendslopStyle main menu fidelity pass toward a real reference match, retrying account-backed imagegen where needed, implementing systematic reusable fixes instead of one-off masks, and updating the Friendslop process documentation with solutions as they are proven.
Stop condition: produce a new verified screen pass with current capture/scorecard, or stop at a concrete blocker where the method/tooling must change before more implementation is honest.

Repo/process rules:
- Follow AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, UI/UI_AGENTS.md, UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md, UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md, and UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md.
- Codex is Operator and Claude is Validator according to `.t66/operator-state.json`.
- The task is process-governed UI reference fidelity work; use PPF/parity/mechanism discipline.
- Do not use API fallback for imagegen; use account-backed built-in imagegen by default.
- Do not accept a structural PASS count if visual fidelity fails.

Context from pass13:
- Pass13 compiled and captured but was intentionally PARTIAL.
- Verifier was `PASS=250 FAIL=1 UNSURE=0`; the single fail was the holistic visual scorecard.
- Remaining known gaps include title crop, side-panel rubber chrome, exact button silhouettes/gloss, search chrome, masking/fitting artifacts, and responsive/manual gates not yet verified.

Please provide an independent repo-grounded answer:
- What systematic fixes should Codex implement next?
- Which fixes should update the Friendslop process doc as proven reusable rules?
- Which artifacts/proofs should pass14 produce before it is allowed to be accepted?
- Call out any likely blockers or method mistakes before Codex edits further.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The user's message resolves the prior blocker, not the scope: the "too many requests" error was a transient rate-limit, not a real lockout. The correct operator action is to recycle the Codex CLI session and retry **account-backed built-in imagegen** — never switch to API fallback (explicit repo rule, restated in the task contract). With imagegen working, the pass13 component contract (`UI/FriendslopStyle/Elements/main_menu_pass13_component_contract.md`) is already written and sound, so pass14 is execution against that contract plus capturing real proof — not a re-plan.

**Systematic fixes Codex should implement next (in priority order):**
1. **Text-fit / auto-shrink + min-width rule applied to *every* label, including the screen title.** The user calls out the title being cut off — that is the same fit failure as the achievements tab, not a separate bug. The fix must be a reusable measured-fit rule (clamp font by measured text width, define min width, compact fallback), not a per-label nudge.
2. **Plate-vs-live-content ownership separation** (blank plate + live glyph/text/data on top; nothing baked underneath). This is the root cause of icon-on-icon and the "smudged/masked center" CTA look. Applies to topbar icons, CTA, friend rows.
3. **Containment for every child rect** inside its parent content area using `contained_in=<Parent> inset=...` (the verifier already supports this). The title crop and any row overflow are both containment/fit symptoms.
4. **Outline-vs-fill as a descriptor convention** (leaderboard local row = red outline + dark interior, not red fill; checkbox = uniform rounded square states).
5. **Friends section live state** (green online dot + green invite plate vs dark offline).

**Which fixes become proven reusable process-doc rules:** items 1–4 generalize to *every* future screen and should be recorded as the user asked — incrementally, as each is proven on a real capture, in a "Proven Solutions / Reusable Rules" section. Natural homes: the text-fit and ownership rules in `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`; containment/min-size in `UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`; the blocking visual-scorecard gate in `UI_FIDELITY_LOOP_INSTRUCTIONS.md`. Per the user, do not mark the process doc *complete* until the screen perfectly matches the reference — but accrete the proven rules along the way.

**Artifacts/proofs pass14 must produce before acceptance:**
- Fresh capture PNG via the Unreal-owned capture path (`CaptureT66UIScreen.ps1`), not a desktop screenshot, plus matching dump JSON from the same run.
- Verifier report + side-by-side contact sheet.
- A **visual scorecard with `Result:` line** and one row per component family (reference crop, current crop, verdict, reason) — and it must now include a **title text-fit/containment row** and the existing rows from the contract (`achievements_text_fit`, `cta_clean_plate`, `leaderboard_local_row_style`, `metric_checkbox_shape`, `online_header_dot`, `invite_green_state`, `row_containment`).
- Pass log entry for the iteration.
- Any regenerated plates: account-backed imagegen output + local contact sheet + slice spec under the contract's declared paths.
- A structural PASS count alone is explicitly **not** acceptance.

**Likely blockers / method mistakes to avoid:**
- Re-introducing inpaint/crop/mask to "fix" the title or CTA — the title crop is a fit problem to solve, not to paint over. This is the single highest-risk regression.
- Treating a recurring rate-limit as license to use API fallback or to skip imagegen — recycle the CLI session instead.
- Declaring PASS on the verifier count while the visual scorecard is missing or FAIL.
- Editing shared `ET66FriendslopChrome` descriptors in a way that regresses other screens reusing the enum — confirm blast radius before changing tokens.

## Evidence Checked
- `main_menu_pass13_component_contract.md`: ownership table, size/fit contract, and 8 verification rows already exist and are well-formed; pass14 executes against it.
- `UI_FIDELITY_LOOP_INSTRUCTIONS.md`: confirms structural PASS ≠ fidelity, blocking visual-scorecard gate, `contained_in`/inset containment support, and ESCALATE-at-5 / stuck-FAIL-set rules.
- pass13 codex draft + pass14 cross-review: confirm the freeze-inpaint / imagegen-for-blank-plates framing the user is reaffirming.
- Task contract: account-backed imagegen default, no API fallback, no structural-PASS-without-visual acceptance.

## Questions Or Blockers
None require the user. The user already chose direction (retry imagegen, systematic-not-one-off, update process doc, finish on real match) and resolved the tooling blocker (recycle the CLI). Codex/Operator can proceed.

## Caveats
- The current contract's scorecard rows do **not** include the title crop the user explicitly flagged — Codex must add a title text-fit/containment row, or the proof will miss the user's named defect.
- The user says the screen is "nowhere near completion," so iterating across multiple families this pass is acceptable — but each fix must still be the systematic rule, not a local patch, and the slice-first safety valve (stop and fix method if the same failure recurs) still applies.
- I inferred function/descriptor targets from the contract and prior reviews; I did not open the function bodies this pass. Codex should confirm exact signatures before editing.

</validator_independent_answer>

<codex_draft>
# Fresh Agent Prompt Draft: FriendslopStyle Main Menu Pass14

You are a fresh Codex agent in `C:\UE\T66`. Your task is to continue the FriendslopStyle Main Menu UI fidelity work and produce the next version of the screen without repeating the previous masking, fitting, icon ownership, panel chrome, and title-cropping errors.

## Working Task

Working task:
Operator: Codex
Validator: Claude
Scope: continue the FriendslopStyle Main Menu fidelity pass from the current pass13 partial state. Implement systematic reusable fixes, retry account-backed imagegen where needed for clean runtime chrome assets, update process documentation only with solutions proven by current capture evidence, and produce a new verified pass.
Stop condition: either produce a new current Main Menu capture/dump/verifier/scorecard showing the next pass is materially closer and no longer has the named errors, or stop at a concrete blocker where the method/tooling must change before more implementation is honest.

## Mandatory T66 Process

1. Start by reading:
   - `C:\UE\T66\AGENTS.md`
   - `C:\UE\T66\.t66\operator-state.json`
   - `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
   - `C:\UE\T66\UI\UI_AGENTS.md`
   - `C:\UE\T66\UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
   - `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`
   - `C:\UE\T66\UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`
2. Follow the Operator/Validator loop. Current known state is Codex Operator, Claude Validator, but verify from `.t66\operator-state.json`.
3. Before Claude use, verify `ANTHROPIC_API_KEY` is not set in Process/User/Machine scope.
4. This is process-governed UI reference fidelity work. Use PPF check, artifact parity gate, mechanism manifest, and a PPF/mechanism close.
5. Do not use native goal tools.
6. Do not use API imagegen fallback or `OPENAI_API_KEY`. Use account-backed built-in imagegen. If the built-in tool returns `TooManyRequests` or a bad request that appears session-related, treat that as a transient Codex session/tool issue and restart/fork to a fresh chat/CLI rather than continuing with approximate assets or API fallback.
7. Do not accept compile success or structural PASS count as visual success. The screen is accepted only when the holistic visual scorecard is `Result: PASS` and the reference/capture crop sheet actually looks correct.

## Project Context

The project is converting T66 UI screens into a new `FriendslopStyle` visual lane. The approved method is:

- Imagegen creates full-screen visual references for art direction.
- Approved references are decomposed into reusable runtime chrome assets.
- Runtime UI is built from tagged Slate widgets, live text/data/icons/state, and reusable sliced or fixed-size transparent PNG plates.
- FlatStyle's no-raster-chrome rule still applies to FlatStyle, but `UI/UI_AGENTS.md` explicitly allows FriendslopStyle generated raster chrome only through `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.

The Main Menu pilot reference is:

`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`

The current screen is not close enough. The user rejected previous attempts because buttons/panels did not match the reference, text and rows did not fit, icons were placed over baked icons, buttons looked manually masked or painted over, some rows used the wrong fill/outline style, and the title logo is visibly cut off.

## Current Pass13 State

Pass13 was a documented partial, not an accepted result.

Important pass13 artifacts:

- Capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass13_fixture_capture.png`
- Dump: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass13_fixture_dump_utf8.json`
- Component crop sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass13_component_crop_sheet.png`
- Visual scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass13_visual_scorecard.md`
- Verifier report: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass13_verify_report.md`
- Pass log: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass_log.md`
- Codex result draft: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\pass13_codex_result_draft.md`
- Claude pass13 cross-review: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T014535-CrossReview-pass16\claude_review_pass16.md`
- Claude pass14 guidance: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T015901-IndependentAnswer-pass17\claude_review_pass17.md`

Pass13 verifier result:

```text
PASS=250 FAIL=1 UNSURE=0
```

The single FAIL is intentional: `MainMenu.VisualScorecard` expected `PASS`, got `FAIL`, because the visual scorecard says `Result: FAIL` / `Overall result: PARTIAL`.

Pass13 did improve these items:

- Removed the stretched search bitmap smear by replacing the bad image dependency with bounded live UI.
- Avoided topbar live glyph over baked settings/language glyphs by repointing `TopbarIconDarkRound06` to `filter_icon_dark_round06.png`.
- Added measured topbar label fitting so `ACHIEVEMENTS` fits.
- Added online green dot.
- Preserved visible green invite button.
- Reworked leaderboard local row to red outline with dark interior.
- Reworked metric checkbox to a uniform rounded square.
- Kept CTA labels/icons live-owned instead of painted into the plate.

Pass13 still fails visually:

- Left and right panel chrome do not match the reference rubber framed panels.
- Topbar icon button silhouette is still wrong because `filter_icon_dark_round06.png` is an approximation, not the proper reference button.
- Search field chrome is thinner/flatter than the reference.
- CTA button silhouettes/gloss still differ from reference.
- The title logo is cut off. Current `title_logo_round06.png` is a fully opaque, edge-to-edge cropped title bitmap.
- Some remaining elements still look masked or smeared because the current runtime plates are not clean blank component plates at the correct size.
- Responsive Step K and manual interaction Step L were not verified.

## Existing Code/Asset Surfaces

Main code targets:

- `C:\UE\T66\Source\T66\UI\Style\T66FriendslopStyle.cpp`
- `C:\UE\T66\Source\T66\UI\Style\T66FriendslopStyle.h`
- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.h`
- `C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp`
- `C:\UE\T66\Source\T66\UI\Components\T66FlatLeaderboardPanel.cpp`

Runtime chrome assets are under:

`C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`

Current contract:

`C:\UE\T66\UI\FriendslopStyle\Elements\main_menu_pass13_component_contract.md`

Update this contract for pass14, especially to add the user-named title text-fit/containment row.

## What Went Wrong Before

Do not repeat these mistakes:

1. Using crop/inpaint/masked plates as runtime assets. This produced buttons that looked cut in half or painted over.
2. Baking text/icons into plates and then layering live text/icons on top. This caused icon-on-icon and smudged button centers.
3. Stretching a generic rubber atom into every size. This destroyed bevels, highlights, and silhouettes.
4. Treating OpenCV/skimage/Pillow as asset-generation quality tools. They are fine for measurement, contact sheets, alpha validation, crop QA, and verification. They do not create the premium runtime art by themselves.
5. Letting child rows/buttons/text exceed their panel or row bounds.
6. Accepting `PASS=250` while the visual scorecard is `FAIL`.
7. Treating imagegen `TooManyRequests` as a permanent blocker and then proceeding with known-wrong approximation assets.

## Systematic Solutions To Implement

Implement these as reusable rules, not one-off fixes:

1. **Measured text-fit/containment for every label and title.**
   - Apply a reusable measured-fit rule: preferred font size, minimum font size, available content width, reserved icon slots, and ellipsis/compact fallback.
   - This applies to `ACHIEVEMENTS`, CTA labels, invite/offline buttons, leaderboard labels, and the title/subtitle.
   - The title crop is part of this same class. Do not paint over or crop the title. Use a correct uncropped title asset or live layered title text with measured containment.

2. **Plate vs live-content ownership separation.**
   - Runtime plates own only chrome: rubber shape, bevel, gloss, shadow, outline/fill material.
   - Slate owns all labels, icons/glyphs, names, scores, counts, player data, state, and click handlers.
   - Generated plates must be blank. No fake text, no fake skulls, no fake glyphs, no partially removed content.

3. **Per-family/per-size runtime plates where generic slicing fails.**
   - If a button/panel cannot be sliced without distortion, generate a size-specific plate for that runtime size.
   - The Main Menu likely needs specific plates for: side panels, CTA primary, CTA secondary, search field, topbar icon, topbar text tab, friend row, section header, dropdown, leaderboard tabs, local leaderboard row, checkbox states.
   - For each generated asset, save source prompt, original generated image, final transparent asset, validation crop/contact sheet, and registry entry.

4. **Containment verifier rules.**
   - Add checklist rows using `contained_in=<Parent> inset=...` where supported.
   - Must cover title inside title region, rows inside panels, action buttons inside rows, CTA text/icons inside buttons, leaderboard row contents inside row.

5. **Descriptor conventions for material states.**
   - Red outline/dark interior is not the same as red fill. Leaderboard local row must remain outline + dark interior.
   - Checkbox is a uniform rounded square state.
   - Online header has green dot; offline has gray dot.
   - Invite is visibly green; offline action is dark/disabled.

6. **Process documentation updates are incremental and evidence-based.**
   - Update `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` only with rules proven by current capture/scorecard.
   - Do not mark the process doc complete until the Main Menu matches the reference.
   - Add a "Proven Solutions / Reusable Rules" section as needed.

## Imagegen Guidance

Use built-in account-backed imagegen for runtime chrome generation. Do not use API fallback.

For transparent runtime assets:

- Prompt for the component on a perfectly flat chroma-key background, usually `#00ff00`.
- Copy the generated image from `C:\Users\DoPra\.codex\generated_images\...` into a project source-art/proof location.
- Use `C:\Users\DoPra\.codex\skills\.system\imagegen\scripts\remove_chroma_key.py` to create the final alpha PNG.
- Validate alpha, transparent corners, no clipping, no key fringe, and no baked text/icons before wiring into runtime.

In the previous chat, a title-logo imagegen retry produced a candidate at:

`C:\Users\DoPra\.codex\generated_images\019e988b-c134-7eb2-828c-140d51026294\ig_0e5ed3a0d706d977016a23a9f71cd481919be5c694ec15bbd7.png`

It is not accepted or wired. Inspect it if available, but do not assume it is production-ready. It has a green background and must be alpha-processed and validated if used. If it is unavailable in the fresh session or not good enough, regenerate.

## Recommended Pass14 Implementation Order

1. Read all process docs and current artifacts listed above.
2. Add pass14 PPF/parity/mechanism preflight to `Saved/Codex/UI/FriendslopStyle/MainMenu/pass_log.md`.
3. Update `main_menu_pass13_component_contract.md` or create pass14 contract addendum:
   - add title text-fit/containment row;
   - add exact rows for clean title asset, side-panel rubber frame, CTA clean blank plates, topbar icon blank plate, search pill, and row containment.
4. Generate or replace the highest-impact bad runtime plates first:
   - title logo: uncropped transparent asset or live layered title with measured containment;
   - left/right panel rubber frames;
   - CTA primary/secondary blank plates;
   - topbar icon blank plate.
5. Validate each asset before wiring:
   - no clipping;
   - transparent background;
   - no baked labels/icons/player data;
   - no painted-over center;
   - correct runtime size or proven slice spec;
   - visual match against reference crop.
6. Wire assets through `FT66FriendslopStyle` descriptors or explicit runtime image brushes without regressing other enum users.
7. Implement reusable measured-fit/containment helpers where needed rather than per-label magic.
8. Update checklist with containment rows and title visual row.
9. Build/stage and capture current fixture.
10. Create component crop sheet comparing reference vs pass14.
11. Create visual/material-state scorecard with `Result: PASS` only if it really passes. Include at least:
    - title_not_cropped;
    - topbar_icon_ownership_and_shape;
    - achievements_text_fit;
    - search_pill_match;
    - cta_clean_blank_plate;
    - left_panel_frame_match;
    - right_panel_frame_match;
    - leaderboard_local_row_style;
    - metric_checkbox_shape;
    - online_header_dot;
    - invite_green_state;
    - row_containment.
12. Run `VerifyUIFidelity.py` with fresh capture, dump, checklist, contact sheet, and scorecard.
13. If visual scorecard fails, do not call the pass complete. Record the fail set and continue only on the failing systematic class.

## Verification Commands

Use the staged build path:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -SkipCook
```

Capture with current fixture and dump. Invoke the script directly in the current PowerShell process so `-ExtraArgs` passes both values:

```powershell
$extra = @(
  '-T66FriendslopReferenceFixture',
  '-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_dump.json'
)
& C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 `
  -Screen MainMenu `
  -Output C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_capture.png `
  -DelaySeconds 6 `
  -TimeoutSeconds 180 `
  -ExtraArgs $extra
```

Convert dump to UTF-8 if needed before verifier:

```powershell
Get-Content -LiteralPath C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_dump.json -Raw -Encoding Unicode |
  Set-Content -LiteralPath C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_dump_utf8.json -Encoding UTF8
```

Run verifier:

```powershell
python C:\UE\T66\Scripts\VerifyUIFidelity.py `
  --reference C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png `
  --capture C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_capture.png `
  --dump C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_dump_utf8.json `
  --checklist C:\UE\T66\UI\FriendslopStyle\Checklists\main_menu_checklist.md `
  --output C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_verify_report.md `
  --contact-sheet C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_verify_contact_sheet.png `
  --visual-scorecard C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_visual_scorecard.md
```

## Acceptance Bar

Pass14 is not automatically expected to be final, but it must not repeat the named errors. It must show real progress on the systematic classes:

- title no longer cropped;
- no obvious masking or painted-over button centers;
- no icon-on-icon;
- text fits and is contained;
- rows/buttons stay inside panels;
- panels and button silhouettes are visibly closer to the reference;
- visual scorecard honestly says PASS or FAIL with reasons.

Full completion of this overall work requires the Main Menu to visually match the reference, then the process doc can be completed with the proven reusable rules for future screens.

</codex_draft>
