# Prompt For Next Agent: FriendslopStyle Main Menu Full-Screen Redo

You are working in `C:\UE\T66`.

## Task Contract

```text
Working task:
Operator: follow the current `.t66/operator-state.json` routing.
Scope: redo the full FriendslopStyle Main Menu so the assembled Unreal runtime screen visually matches the approved Round06 reference. This is not a narrow leaderboard-width fix. The prior pass failed visually across the whole screen.
Stop condition: either produce a current verified 1920x1080 runtime capture that matches the reference and extracted element sheet in appearance, size, and position, or document a hard blocker in the approach.
```

Do not declare done from a structural `VerifyUIFidelity.py` PASS count. The prior pass did that with `PASS=94 FAIL=0`, and it was wrong.

## Read First

1. `C:\UE\T66\AGENTS.md`
2. `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
3. `C:\UE\T66\UI\UI_AGENTS.md`
4. `C:\UE\T66\UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
5. `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`
6. `C:\UE\T66\UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`

Follow the Codex/Claude Operator/Validator protocol. Before using Claude, verify `ANTHROPIC_API_KEY` is not set in Process/User/Machine scope.

## Visual Targets

Approved whole-screen reference:

`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`

Extracted elements/material target image:

`C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_alpha.png`

Original chroma-key source sheet:

`C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_source_chromakey.png`

The runtime screen must look like the approved whole-screen reference, and each assembled UI element must look like the matching element/material from the extracted elements sheet. A correctly placed element that does not look like the extracted rubber element is a FAIL.

## Current Failed Pass To Study

Final bad capture:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass08_capture.png`

Bad contact sheet:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass08_contact_sheet.png`

Bad fidelity report before gate correction:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass08_fidelity.md`

Gate-corrected report proving the old capture is no longer accepted:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\ProcessFix\main_menu_amended_report.md`

Do not focus only on the leaderboard row width. The user’s correction is that the full screen failed visually everywhere. The new visual gates should expose broad appearance/scale/position failures. If the old capture only fails a handful of checks, the scorecard is still too weak.

## Relevant Code Surfaces

- `C:\UE\T66\Source\T66\UI\Style\T66FriendslopStyle.h`
- `C:\UE\T66\Source\T66\UI\Style\T66FriendslopStyle.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp`
- `C:\UE\T66\Source\T66\UI\Components\T66FlatLeaderboardPanel.cpp`
- `C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp`

Runtime/source asset roots:

- `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`
- `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\`

Process artifacts:

- `C:\UE\T66\UI\FriendslopStyle\Checklists\main_menu_checklist.md`
- `C:\UE\T66\UI\FriendslopStyle\Checklists\visual_scorecard_template.md`
- `C:\UE\T66\UI\FriendslopStyle\Elements\main_menu_element_manifest.md`
- `C:\UE\T66\UI\FriendslopStyle\SliceSpecs\main_menu_slice_specs.md`
- `C:\UE\T66\UI\FriendslopStyle\friendslop_asset_registry.md`

The first-pilot manifest/registry/slice specs are labeled as pilot artifacts. They are not final acceptance evidence.

## Image Generation Requirement

For all image generation, deploy Codex CLI workers. Use a fresh local Codex CLI worker per image-generation job, save the prompt and artifacts, then close that worker. Do not use ad hoc API scripts or `OPENAI_API_KEY` image-generation fallbacks.

Use the account-backed imagegen process from the `imagegen` skill. Runtime chrome outputs must be transparent PNG plates or plate families, with live text/data/icons layered in Unreal. Do not bake labels, player names, scores, friend state, ticket counts, or localization into plates.

## Required Redo Strategy

1. Rebuild the visual scorecard before touching visuals.
   - Enumerate every load-bearing UI element.
   - For each element, score at least: appearance/material match, size match, position match.
   - Include top bar buttons, ticket badge, power button, left panel, profile row, search field, online/offline headers, friend rows, invite/offline buttons, party slots, title, subtitle, Enter CTA, Load CTA, right filter buttons, leaderboard panel, dropdowns, metric buttons, table headers, and ranking rows.
   - The current bad capture should fail broadly. Treat “around 96 fails” as a sanity expectation for the old pass, not a hard-coded target.

2. Re-author plates, not manual Unreal approximations.
   - The premium rubber look must come from transparent PNG plates derived from or matching the extracted elements sheet.
   - Unreal should place plates, scale only when proven slice-safe, and overlay live Slate text/data.
   - If 9-slice or 3-slice distorts bevels/highlights/shadows, use size-specific plates.

3. Fix full-screen layout fidelity.
   - Measure the Round06 reference at native resolution.
   - Normalize to the 1920x1080 basis.
   - Use tight per-element tolerances, not broad region-only checks.
   - Add containment checks with insets wherever a child belongs inside a panel or table body.

4. Implement the full screen only after the scorecard and asset plan are ready.
   - Do not repeat the old approach of generic reusable rubber atoms stretched across everything.
   - Do not call a screen “close enough” if the assembled elements do not resemble the extracted element sheet.

5. Verify through current runtime evidence.
   - Run focused compile/build as needed.
   - Capture via Unreal-owned tooling, not desktop screenshots.
   - Use the canonical capture/dump form:
     `Scripts\CaptureT66UIScreen.ps1 -Screen MainMenu -Output <pass_N_capture.png> -DelaySeconds 6 -ExtraArgs @("-T66AutoDumpScreen=<pass_N_dump.json>")`
   - Run `Scripts\VerifyUIFidelity.py` with `--visual-scorecard`.
   - Both Codex and Claude must inspect the reference, capture, and contact sheet before citing PASS counts.

## Completion Bar

Done requires all of these:

- Current capture at 1920x1080.
- Current dump.
- `VerifyUIFidelity.py` report with zero structural/containment FAIL.
- Visual scorecard with `Result: PASS`.
- Contact sheet showing the runtime screen reads like Round06 at a glance.
- Per-element scorecard rows showing the assembled elements look like the extracted elements sheet.
- No broad visual mismatch hidden behind a green numeric report.

If the visual scorecard fails, the pass is not done even if the structured verifier is green.

## Expected Output Locations

Use next free pass numbers under:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\`

Keep prompts and imagegen worker records under:

`C:\UE\T66\UI\FriendslopStyle\SourcePrompts\MainMenu\`

Update source assets under:

`C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\`

Update runtime assets under:

`C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`

## Final Reporting

Report:

- whether the result is DONE or BLOCKED;
- exact capture/dump/report/contact-sheet/scorecard paths;
- what imagegen Codex CLI workers generated;
- what plates were replaced or added;
- structural verifier result;
- visual scorecard result;
- Claude validation result and token usage;
- Codex token usage from `Scripts\Get-CodexTokenUsage.ps1`.
