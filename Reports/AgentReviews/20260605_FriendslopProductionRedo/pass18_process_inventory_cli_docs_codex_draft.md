# Codex Draft - Pass18 Process Inventory / CLI Worker Docs

## Work Completed

Updated active FriendslopStyle process docs so future Main Menu/Friendslop passes must:

- maintain a full element manifest ledger;
- evaluate every element every implementation pass;
- classify each row as `PASS`, `REGENERATE`, `REBUILD`, or `ACCEPTED_DELTA`;
- report total/pass/regenerate/rebuild/accepted-delta counts;
- keep the screen `PARTIAL` while any row is unreviewed, `REGENERATE`, or `REBUILD`;
- run Friendslop image generation in separate local Codex CLI workers using account-backed built-in imagegen, not the main Codex app chat and not `OPENAI_API_KEY`/API scripts.

Changed files:

- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`
- `UI/FriendslopStyle/README.md`
- `UI/FriendslopStyle/friendslop_asset_registry.md`
- `UI/FriendslopStyle/Screens/MainMenu/README.md`
- `UI/FriendslopStyle/Screens/MainMenu/element_manifest.md`
- `UI/FriendslopStyle/Screens/MainMenu/checklist.md`
- `UI/FriendslopStyle/Screens/MainMenu/visual_scorecard_template.md`
- `UI/FriendslopStyle/Screens/MainMenu/component_contract_current.md`
- `UI/FriendslopStyle/Screens/MainMenu/production_plate_plan.md`

Verification:

- Read live T66 root/UI/Friendslop process docs before editing.
- Verified `.t66/operator-state.json`: Codex operator, Claude validator.
- Verified `ANTHROPIC_API_KEY` absent in Process/User/Machine scopes.
- Ran Claude independent answer: `Reports/AgentReviews/20260605_FriendslopProductionRedo/20260606T083934-IndependentAnswer-pass36/claude_review_pass36.md`, `Result: OK`, `ClaudeTokensSpent: 235,525`.
- Ran narrow `rg` checks for `Codex CLI worker`, `main Codex app chat`, `REGENERATE`, `REBUILD`, `ACCEPTED_DELTA`, and active rule text.
- Ran `git diff --check` on edited paths; no whitespace errors, only the existing line-ending warning for `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`.

## Fresh Chat Prompt To Provide User

```text
You are a fresh Codex agent in C:\UE\T66.

Read and follow these files before doing any work:
- C:\UE\T66\AGENTS.md
- C:\UE\T66\.t66\operator-state.json
- C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md
- C:\UE\T66\UI\UI_AGENTS.md
- C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md
- C:\UE\T66\UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md
- C:\UE\T66\UI\FriendslopStyle\README.md
- C:\UE\T66\UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md
- C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\README.md
- C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\element_manifest.md
- C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\component_contract_current.md
- C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\checklist.md
- C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\visual_scorecard_template.md

Do not use native goal tools.
Verify the Operator/Validator state from .t66/operator-state.json. Codex is expected to be Operator and Claude Validator.
Before invoking Claude, verify ANTHROPIC_API_KEY is absent in Process/User/Machine scopes.

Task:
Continue the FriendslopStyle Main Menu implementation from the current partial state and produce the next implementation pass. The goal is not a narrow title-only fix. The goal is a full-screen iteration against every element in the Main Menu manifest.

Approved reference:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_01_current_capture_stronger_rubber_cli.png

Active screen docs:
C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\

Hard process rules:
1. Do a full inventory pass using element_manifest.md. Every row must be evaluated in this pass.
2. Assign each row exactly one status: PASS, REGENERATE, REBUILD, or ACCEPTED_DELTA.
3. Do not focus only on the title or the latest discussed component. Every non-matching element must be acted on or explicitly reported as still failing.
4. Use REGENERATE when the visual pixels are wrong: wrong rubber material, silhouette, bevel, gloss, shadow, smears, masks, pillow centers, baked text/icons, wrong title/background style, or a blank plate that is clean but visually different from the reference.
5. Use REBUILD when the asset can remain but Slate layout, containment, slicing, text/icon placement, state wiring, import path, or interaction is wrong.
6. Use ACCEPTED_DELTA only for an explicit user-approved delta already documented in the pass.
7. The screen is PARTIAL while any row is unreviewed, REGENERATE, or REBUILD. Do not claim FULL unless every row is PASS or ACCEPTED_DELTA and the visual scorecard says Result: PASS.

Image generation rule:
Every asset generation/regeneration in this iteration must be done through a separate local Codex CLI worker using account-backed built-in imagegen. Do not generate Friendslop assets in the main Codex app chat. Do not use OPENAI_API_KEY, OpenAI API scripts, web image URLs, browser screenshots, or old generated-image folders. Reference crops are measurement/comparison targets only, not runtime asset sources.

For each imagegen worker:
- write a request/prompt markdown file;
- record start time or run id;
- attach or reference required visual context;
- require a new image produced by that worker's own built-in imagegen call;
- ban copying old generated outputs;
- save stdout/stderr logs or equivalent transcript;
- save last_message.txt or equivalent final status;
- copy the output PNG to the required SourceAssets/RuntimeDependencies path only after verifying it came from this worker;
- record token count and SHA-256 when available.

If built-in imagegen returns TooManyRequests or a session/auth-like transient failure, restart or fork a fresh Codex CLI worker. Do not approximate manually, do not use API fallback, and do not salvage a bad asset with Pillow/OpenCV masking or local pixel repair.

Implementation expectations:
- Start with a fresh Main Menu capture and dump through Unreal-owned tooling.
- Determine the next pass number from existing Saved/Codex/UI/FriendslopStyle/MainMenu artifacts.
- For every REGENERATE row, generate a new candidate through CLI worker imagegen, then do only mechanical alpha/chroma removal, crop/padding, resize, slice setup, and validation. No manual visual editing.
- For every REBUILD row, update Slate/layout/containment/state/import wiring as needed.
- Keep all labels, titles when live, icons, names, scores, counts, online/offline state, localization, hover/pressed/selected/disabled state, and interactions live in Slate unless the current contract explicitly allows a baked static icon.
- Do not paste the full reference into runtime UI.
- Do not bake player names, scores, labels, localized text, row data, or action labels into runtime plates.

Required final artifacts:
- Fresh capture PNG.
- Fresh dump JSON.
- VerifyUIFidelity report using the active Main Menu checklist.
- Contact sheet / side-by-side sheet.
- Visual scorecard with Result: PASS or Result: FAIL.
- Updated element manifest ledger summary with total rows, PASS count, REGENERATE count, REBUILD count, ACCEPTED_DELTA count, and unreviewed count.
- Worker records for every regenerated asset.
- List of rebuilt elements and files changed.
- Responsive gate result or explicit skipped reason.
- Manual interaction gate result or explicit skipped reason.

Use this verifier shape, adjusted only if the current script help differs:
python C:\UE\T66\Scripts\VerifyUIFidelity.py --reference <reference.png> --capture <capture.png> --dump <dump.json> --checklist C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\checklist.md --output <report.md> --contact-sheet <contact_sheet.png> --visual-scorecard <visual_scorecard.md>

Final response requirements:
- State whether the pass is FULL or PARTIAL.
- Report total manifest rows and counts for PASS, REGENERATE, REBUILD, ACCEPTED_DELTA, and unreviewed.
- Report exactly how many elements were regenerated and which CLI worker records prove them.
- Report exactly how many elements were rebuilt and which files changed.
- Show the reference image and produced capture side by side in Markdown using absolute image paths:
  ![Reference](C:/UE/T66/UI/FriendslopStyle/Reference/MainMenu/Current/main_menu_reference_01_current_capture_stronger_rubber_cli.png)
  ![Produced capture](<absolute path to fresh capture>)
- Also show or link the contact sheet.
- Do not accept compile success, structural PASS count, or PASS=250-style verifier output as visual success. The visual scorecard controls acceptance.
```
