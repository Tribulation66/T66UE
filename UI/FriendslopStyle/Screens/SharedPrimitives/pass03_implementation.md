# Shared Primitives Pass 03 Implementation Record

Date: 2026-06-08

## Scope

Pass 03 corrected the reusable FriendslopStyle standard modal and standard tooltip after the initial generated-shell integration exposed sizing and tint issues.

## Process

- Operator: Codex
- Validator: Claude through `Scripts/Invoke-ClaudePlanReview.ps1`
- Proven process: `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
- Image generation path: account-backed local Codex CLI worker using the built-in imagegen workflow
- Button worker: `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/`

## Implemented Changes

- Replaced locally drawn/tinted modal button chrome with generated textless PNG buttons:
  - `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_primary_red.png`
  - `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_action_green.png`
  - `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_long_dark.png`
- Updated matching source copies under `SourceAssets/UI/FriendslopStyle/SharedPrimitives/`.
- Reset loaded custom Friendslop brushes to white tint so loaded PNGs are not darkened by fallback tint.
- Constrained modal button sizing to 300 x 58 with 16px live labels and scale-down text fit.
- Raised and centered modal button slots inside the shared modal shell.
- Expanded tooltip safe content padding and minimum height so item rows clear the chrome border and pointer notch.
- Updated the implementation instructions to explicitly ban local drawing, tinting, recoloring, vector construction, and fallback brush substitutions for red/green/dark/state button chrome.
- Updated `slice_specs.md` with the final modal and tooltip layout values.

## Evidence

Generated button worker outputs:

- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/standard_modal_button_primary_red.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/standard_modal_button_action_green.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/standard_modal_button_long_dark.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/standard_modal_button_family_contact_sheet.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/validation.json`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_workers/standard_modal_buttons/record.md`

Final captures:

- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/quit_confirmation.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/quit_confirmation.json`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/party_invite.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/party_invite.json`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/save_preview.png`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/save_preview.json`
- `Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass03_captures_20260608/tooltip_item.png`

Tooltip caveat:

- Gameplay tooltip screenshot capture passed and the log marker confirms `[TooltipCapture] opened tooltip 'Item'`.
- `T66AutoDumpWidget` could not dump the tooltip because the current dump resolver only finds active viewport `UUserWidget` roots; the tooltip capture is a transient Slate widget added directly to the viewport.

## Verification

- `T66Editor Win64 Development` build: PASS.
- `Scripts/StageStandaloneBuild.ps1`: PASS.
- Modal captures through `Scripts/CaptureT66UIScreen.ps1`: PASS.
- Tooltip capture through `Scripts/CaptureT66UIWidget.ps1`: screenshot PASS; structured dump FAIL due transient Slate dump-target limitation.

## Geometry Correction Notes

The values now recorded in `slice_specs.md` are the result of this pass, not the pre-existing acceptance target. The prior modal button slots and tooltip padding/min-height failed visual fit checks:

- Modal buttons at 330 x 68 sat too low and made live labels crowd the generated chrome at the runtime viewport scale.
- Tooltip padding/min-height allowed item rows to crowd the shell border and pointer notch.

The final values were selected after recapture:

- Modal buttons: 300 x 58 at `X=230,Y=274` and `X=590,Y=274`, with 16px live labels.
- Tooltip: 560 width, 310 minimum height, 460 wrap width, and `FMargin(50,40,50,80)` content padding.

The modal button PNG source canvases do not match the final slot aspect exactly,
and the current 9-slice margins are too large for the 300 x 58 slots. The
sizing/positioning result is still preserved, but the red, green, and dark
button chrome has a separate slice-integrity failure: protected caps and
highlights collapse toward the center and read as a cut or pillow seam. A future
visual fix should change slice mode/margins or use a size-specific generated
plate through the imagegen worker process, not manual image edits.

## Primitive Fit Gate

Standard modal:

- Chrome coverage: PASS.
- Texture load and tint: PASS.
- Slice integrity: FAIL. The red, green, and dark generated button assets render
  with a visible center seam/pillow artifact at 300 x 58 because the current
  9-slice cap budget is too large for the runtime slot.
- Containment: PASS.
- Minimum padding: PASS.
- No overlap: PASS.
- Centering/alignment: PASS.
- Text fit: PASS.
- State fit: PASS for visible captured states:
  - enabled red/green states in quit confirmation and save preview;
  - disabled red/green states in the party invite empty state.
- Overall primitive fit gate: PARTIAL. Layout, live text fit, tint, and
  positioning pass; button slice integrity fails and needs a separate visual
  fix.

Standard tooltip:

- Chrome coverage: PASS.
- Texture load and tint: PASS.
- Screenshot containment: PASS by visual review.
- Screenshot minimum padding: PASS by visual review.
- Screenshot no overlap: PASS.
- Screenshot centering/alignment: PASS.
- Screenshot text fit: PASS.
- Structured dump: FAIL, unresolved.
- Overall primitive fit gate: PARTIAL because the structured dump could not inspect the transient Slate tooltip widget.

## PPF Close

Process used: FriendslopStyle shared primitive implementation with account-backed imagegen worker for new button chrome and Slate-only sizing correction.

Matches declared process: YES.

Evidence: worker artifacts, source/runtime PNGs, final captures, build/stage logs, and updated slice specs.

## Mechanism Close

Mechanism: generated visual chrome.
Status: PRESENT.
Evidence: red, green, and dark modal button PNGs generated by worker and copied to source/runtime paths.
Discriminator test: no local Slate fill, tint, vector construction, or manually repaired image is used for button color variants.
Reported status: FULL.

Mechanism: live localizable text.
Status: PRESENT.
Evidence: modal and tooltip labels/text are `STextBlock` content over textless shells.
Discriminator test: captured labels can change without editing the generated PNGs.
Reported status: FULL.

Mechanism: sizing and positioning fit gate.
Status: PARTIAL.
Evidence: final modal dumps and tooltip screenshot show content contained inside
generated primitives; tooltip structured dump failed because the transient Slate
tooltip widget is not visible to the current dump-target resolver. The modal
button geometry and labels fit, but button slice integrity fails at 300 x 58.
Discriminator test: initial tooltip row crowding into the pointer was corrected
through Slate padding/min-height, not image edits. The modal button seam is now
tracked as a slice packaging failure rather than accepted as a layout result.
Reported status: PARTIAL.
