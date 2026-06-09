# FriendslopStyle Shared Primitive Runtime Specs

These specs describe the current runtime overlay geometry for the shared modal,
modal checkbox variant, and tooltip. They are not permission to author new
pixels by cropping, masking, repainting, or inpainting.

## Standard Modal

Asset:

`RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_panel_textless.png`

Source copy:

`SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_modal_panel_textless.png`

Runtime helper:

`T66ScreenSlateHelpers::MakeFriendslopStandardModal`

Panel size:

- Width: 1120
- Height: 400
- Brush margin: `FMargin(0.14, 0.20, 0.14, 0.16)`
- Fallback fill: `FLinearColor(0.035, 0.038, 0.047, 1.0)`
- Scrim: black at 0.62 alpha

Live overlay slots:

- Title: `X=190, Y=59, W=740, H=60`, font size 30, bold, scale-down one-line text.
- Body: `X=200, Y=156, W=720, H=88`, font size 16, wrapping at 650.
- Status: `X=170, Y=244, W=780, H=32`, font size 16, wrapping at 730, collapsed when empty.
- Left button: `X=230, Y=274, W=300, H=58`.
- Right button: `X=590, Y=274, W=300, H=58`.

Opt-in checkbox variant panel size:

- Width: 1120
- Height: 490
- Brush margin: `FMargin(0.14, 0.20, 0.14, 0.16)`
- Fallback fill: `FLinearColor(0.035, 0.038, 0.047, 1.0)`

Opt-in checkbox variant slots:

- Title: `X=190, Y=59, W=740, H=60`, font size 30, bold, scale-down one-line text.
- Body: `X=200, Y=156, W=720, H=88`, font size 16, wrapping at 650.
- Status: `X=170, Y=250, W=780, H=32`, font size 16, wrapping at 730, collapsed when empty.
- Checkbox row: `X=340, Y=296, W=440, H=52`.
- Left button: `X=230, Y=360, W=300, H=58`.
- Right button: `X=590, Y=360, W=300, H=58`.

Button chrome options:

- `Dark` -> `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_long_dark.png`, 300 x 58 source canvas.
- `Red` -> `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_primary_red.png`, 300 x 58 source canvas.
- `Green` -> `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_action_green.png`, 300 x 58 source canvas.

Button source copies:

- `SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_long_dark.png`
- `SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_primary_red.png`
- `SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_modal_button_action_green.png`

Button generation record:

`Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass04_workers/standard_modal_buttons_exact_300x58/record.md`

Checkbox chrome options:

- `Unchecked` -> `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_checkbox_unchecked.png`, 44 x 44 source canvas.
- `Checked` -> `RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_modal_checkbox_checked.png`, 44 x 44 source canvas.

Checkbox source copies:

- `SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_modal_checkbox_unchecked.png`
- `SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_modal_checkbox_checked.png`

Checkbox generation record:

`Saved/Codex/UI/FriendslopStyle/SharedPrimitives/pass05_workers/standard_modal_checkbox/record.md`

Checkbox text fit:

- Live label font size: 16, bold.
- Label box: `W=360, H=38`.
- Label behavior: scale down only, ellipsize if the localized string still
  cannot fit.

Checkbox row capture route:

- Standard unchecked preview:
  `Scripts/CaptureT66UIScreen.ps1 -Screen MainMenu -Modal QuitConfirmation -ExtraArgs @('-T66FriendslopPreviewDoNotAskAgain')`
- Standard checked preview:
  `Scripts/CaptureT66UIScreen.ps1 -Screen MainMenu -Modal QuitConfirmation -ExtraArgs @('-T66FriendslopPreviewDoNotAskAgain','-T66FriendslopPreviewDoNotAskAgainChecked')`
- Click-toggle preview:
  `Scripts/CaptureT66UIScreen.ps1 -Screen MainMenu -Modal QuitConfirmation -ClickTag 'QuitConfirmation.DoNotAskAgain' -ClickDelaySeconds 0.8 -ExtraArgs @('-T66FriendslopPreviewDoNotAskAgain')`
- Long-label fit preview:
  `Scripts/CaptureT66UIScreen.ps1 -Screen MainMenu -Modal QuitConfirmation -ExtraArgs @('-T66FriendslopPreviewDoNotAskAgain','-T66FriendslopPreviewDoNotAskAgainLongLabel')`

Checkbox row non-overlap table:

- Body ends at `Y=244`.
- Status row starts at `Y=250` and ends at `Y=282` when visible.
- Checkbox row starts at `Y=296` and ends at `Y=348`.
- Buttons start at `Y=360`.
- Minimum vertical gap in the occupied stack: 6 px between body/status, 14 px
  between status/checkbox, and 12 px between checkbox/buttons. With empty
  status text, the visible body-to-checkbox gap is 52 px.

Checkbox variant slice/fit evidence:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass05_captures_20260609\standard_modal_checkbox_variant_contact_sheet.png`

Button states:

- `Default`
- `Selected`
- `Ready`
- `Disabled`

Button text fit:

- Live label font size: 16, bold.
- Inner label box: button width minus 74, button height minus 22.
- Label behavior: scale down only, ellipsize if the localized string still cannot fit.

Button slice integrity:

- Current draw mode: Slate `Image`.
- Current margin: `FMargin(0, 0, 0, 0)`.
- Red, green, and dark source cap budget: none. These are size-specific
  300 x 58 plates and do not use protected caps or center stretching.
- Current runtime slot: 300 x 58.
- Runtime cap budget at 300 x 58: not applicable because the plates render as
  fixed images.
- Current status: `SLICE INTEGRITY RESOLVED BY FIXED IMAGE`. Pass 04 replaced
  the red, green, and dark button plates with exact-size 300 x 58 generated
  PNGs and switched the standard modal helper to zero-margin image rendering.
- Preserved layout result: the 300 x 58 slots, `X=230,Y=274` and
  `X=590,Y=274`, and 16px live labels remain the accepted sizing/positioning
  correction.
- Future variable-width button variants must not reuse this fixed-image spec.
  If modal button width becomes variable again, rerun the slice/contact-sheet
  gate and use horizontal-3-slice or fresh size-specific generated plates.

Current users:

- Quit confirmation
- Party invite
- Save preview

## Standard Tooltip

Asset:

`RuntimeDependencies/T66/UI/FriendslopStyle/SharedPrimitives/standard_tooltip_panel_textless.png`

Source copy:

`SourceAssets/UI/FriendslopStyle/SharedPrimitives/standard_tooltip_panel_textless.png`

Runtime helper:

`T66TooltipSlate::MakeTooltipContent`

Panel size:

- Width override: 560
- Minimum desired height: 310
- Source image target size: 560 x 260
- Brush margin: `FMargin(0.12, 0.20, 0.12, 0.26)`
- Content padding: `FMargin(50, 40, 50, 80)`
- Text wrap width: 460
- Fallback fill: `FLinearColor(0.035, 0.038, 0.047, 1.0)`

Live content fields:

- Title: font size 16, bold
- Subtitle: font size 12
- Body: font size 13
- Rows: label/value pairs
- Warnings: font size 12, bold

Current capture path:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass03_captures_20260608\tooltip_item.png`
