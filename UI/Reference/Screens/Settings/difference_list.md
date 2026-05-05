# Settings First-Pass Difference List

Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Settings.png`

Baseline packaged capture: `C:\UE\T66\UI\Reference\Screens\Settings\Proof\settings_baseline_packaged_1920x1080.png`

## Baseline Inventory

- Panels: runtime used the dark V16 fullscreen panel; reference uses a warm parchment/gold shell.
- Rows: runtime used dark wood row shells; reference rows read as light parchment plates with thin warm outlines.
- Buttons and tabs: runtime button states were partly shared/shared-derived; reference needs Settings-owned pill plates with live text.
- Dropdowns and fields: runtime had a Settings-owned dropdown field but surrounding chrome still read dark.
- Sliders/meters: runtime slider progress routed through shared reference helpers; reference needs target-owned meter art.
- Scrollbars: runtime used the default gray scrollbar; reference uses an ornate warm scrollbar.
- Text: runtime body labels were mostly light-on-dark; reference body labels are dark-on-parchment while button labels stay light over brown/gold plates.

## First Pass Changes

- Promoted accepted generated/shared chrome into the Settings runtime folder before routing:
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Panels\settings_panels_reference_scroll_paper_frame.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_normal.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_hover.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_pressed.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_selected.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Buttons\settings_buttons_pill_disabled.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_controls_sheet.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings\Controls\settings_controls_reference_progress_meter_sheet.png`
- Updated Settings chrome routing so buttons, shell panels, row shells, dropdown fields, slider thumbs, progress meters, and scrollbars resolve from Settings-owned assets.
- Applied explicit resize contracts:
  - fullscreen shell: 9-slice box
  - row shell: 9-slice box
  - non-square pill buttons: sliced plate, zero brush margin, live text, normalized minimum width
  - dropdown field: horizontal sliced plate
  - progress meter: horizontal 3-slice
  - slider thumb: fixed image region
  - scrollbar: fixed image brushes from the Settings controls sheet
- Updated Settings page text colors toward the reference: dark body/muted text on parchment, light button text over brown/gold plates.

## Verification

- Focused Settings compile succeeded:
  - `Build.bat T66 Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -Module=T66 -File=...T66SettingsScreen_RetroFX.cpp -File=...T66SettingsScreen_Gameplay.cpp -File=...T66SettingsScreen_HUD.cpp -File=...T66SettingsScreen_Controls.cpp -File=...T66SettingsScreen_Graphics.cpp`
- Full target build failed before packaged staging/capture due an unrelated TD compile blocker:
  - `C:\UE\T66\Source\T66TD\Private\UI\Screens\T66TDBattleScreen.cpp(724,4): error C4002: too many arguments for function-like macro invocation 'SLATE_ARGUMENT'`
- Because the full target does not build, there is no valid post-change packaged Settings capture for this pass. The baseline packaged capture remains the only packaged visual proof in this workspace.

## Residual Differences To Check After TD Build Is Unblocked

- Confirm the parchment shell now visually replaces the old dark V16 panel in packaged runtime.
- Confirm row height, row spacing, and right-side column width against the reference after the new row shell is visible.
- Confirm the themed scrollbar appears in all Settings tabs and does not crowd row content.
- Confirm slider/meter fill alignment and thumb scale against the reference.
- Confirm selected/unselected tab and button contrast against the reference.
- Confirm body text stays dark on parchment while button/tab text remains readable over chrome.
