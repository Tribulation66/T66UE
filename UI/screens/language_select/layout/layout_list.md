# Language Select Layout List

Reference gate target: generated screen-specific reference for the full Language Select screen.

Canvas: canonical `1920x1080`.

Current target screenshot:
- `C:\UE\T66\UI\screens\language_select\current\2026-04-24\current_runtime_1920x1080.png`
- The file decodes at the required `1920x1080`. Use it as the exact current runtime structure capture.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot listed above.
- This layout list.

Source audited:
- `C:\UE\T66\Source\T66\UI\Screens\T66LanguageSelectScreen.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\T66LanguageSelectScreen.h`

## Structural Contract

- Preserve the current hierarchy: shared frontend topbar, shared Back navigation button, centered page title, large centered language scroll panel, bottom-centered Confirm action.
- Restyle the screen into the canonical main-menu fantasy language: carved dark stone, bronze/gold bevels, deep slate panel interiors, green/blue/purple accent family where appropriate.
- Do not redesign the screen into a card grid, wizard, split panel, or landing page.
- Do not bake localizable language names, title text, or button labels into runtime-intended art.

## Canvas And Bounds

- `canvas`: full-screen reference canvas, `1920x1080`.
- `shared_topbar`: visible global frontend topbar at the top edge; current capture shows it across the full `1920x1080` canvas. In canonical reference, keep it as the same top band from the main-menu style anchor.
- `screen_background`: full screen below the topbar; current capture is a flat dark blue/slate surface. Restyle as a subtle fantasy menu background or dark stone field, but no foreground controls, text, language rows, or button labels may be baked into the background plate.
- `content_safe_area`: starts below the topbar with a small overlap/clearance from the shared bar. The screen content must not collide with the topbar or Back button.
- `title_region`: centered horizontally below the topbar; current capture title occupies roughly the upper center between `y 150-215`. Text reads `Select Language`; it is runtime/localizable text.
- `language_scroll_panel`: large centered panel under the title; current capture approximate bounds `x 73-1389`, `y 267-790` in the decoded screenshot. In canonical reference, preserve this as a wide central list panel occupying about 90 percent canvas width and about 55 percent canvas height.
- `bottom_action_region`: centered below the scroll panel; contains the Confirm button. Current capture shows the label near bottom center around `y 835-865`.
- `shared_back_button_region`: visible shared Back button on the left side below the topbar; current capture approximate bounds `x 27-226`, `y 239-301`. Keep the interaction position and clearance, but restyle its plate to the main-menu button family.

## Visible Regions

- `global_topbar_chrome`: shared runtime/global UI chrome, not owned by this screen. It includes settings icon, chat icon, account tab, portrait button, power-up tab, achievements tab, and minigames tab.
- `language_page_shell`: screen-owned dark panel/background surface below the shared topbar. It should look finished in the generated reference but later must separate into a UI-free plate plus foreground shells.
- `language_title_well`: runtime text well for the localized title.
- `language_list_shell`: generated foreground panel frame/backplate for the scroll list. It should have a dark readable interior, carved fantasy border, and enough padding for a vertical scrollbar.
- `language_rows`: centered vertical stack of language row buttons inside the scroll panel. Source row size is `400x60` logical units with about `5` units vertical padding on each side.
- `language_scrollbar_track`: vertical scrollbar on the right edge of the list panel. Current capture shows the thumb near the top, indicating more languages below the visible rows.
- `confirm_button_slot`: bottom-center primary/success action button, source min width `180`, height about `50`.
- `back_button_slot`: shared navigation Back button. It is visually present in the current screenshot although the non-modal screen source only owns Confirm; keep it as shared runtime navigation.

## Controls And Required States

- `language_row_button`: real row button for each language.
  - Required states: `normal`, `hover`, `pressed`, `selected`, `disabled`.
  - Selected state must be persistent after click and visibly distinct without shifting row size.
  - Source binds selected state to `PreviewedLanguage`.
- `confirm_button`: real button that applies `PreviewedLanguage`.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
  - Use success/primary fantasy treatment in the generated reference.
- `shared_back_button`: real shared navigation button.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
  - In full-screen presentation it returns to previous screen; in modal presentation it cancels/closes.
- `scrollbar`: real scroll affordance for the language list.
  - Required states: `normal`, `hover`, `dragged`, `disabled`.

## Live Runtime Content And Ownership

- `title_text`: `Select Language`; runtime/localizable.
- `language_names`: language display names from `UT66LocalizationSubsystem::GetLanguageDisplayName`; runtime/localizable. Current visible examples include English, Simplified Chinese, Traditional Chinese, Japanese, Korean, and a partially visible Russian row.
- `confirm_label`: `Confirm`; runtime/localizable.
- `back_label`: `Back`; runtime/localizable/shared navigation.
- `selected_language_state`: runtime state owned by `PreviewedLanguage`.
- `language_list_count_and_order`: runtime data from `GetAvailableLanguages`.
- `scroll_position`: runtime-owned and must not be baked into a static asset except as an offline reference indicator.

## Variants

- `full_screen_current`: shared topbar visible, shared Back button visible, title/list/Confirm layout as in the current screenshot.
- `modal_presentation`: source supports a centered scrim modal when opened as a modal. This variant uses a centered `960` logical width content shell with both Back and Confirm in a two-button row.
- `short_language_list`: scrollbar may be hidden or disabled if all language rows fit.
- `long_language_list`: scrollbar visible with thumb position reflecting current scroll offset.
- `selected_language`: one row highlighted as selected.
- `no_localization_subsystem_fallback`: fallback title and labels still show English runtime text.

## Reference Constraints

- The generated reference must preserve the current centered list structure and bottom Confirm placement.
- The generated reference must restyle current flat/plain elements to the main-menu fantasy style, but must not invent new panels, sidebars, decorative cards, avatars, flags, or explanatory copy.
- Language rows may have ornate row plates, but the words themselves must remain live text.
- The list panel must leave enough interior width for long localized language names and enough height for at least five full rows plus a partial sixth row.
- The scrollbar must stay on the right edge of the list panel and must not overlap language names.
- Do not bake the shared topbar labels or shared Back label into screen-specific runtime art.
- Later sprite/component work must use text-free button plates and row shells with stable `normal`, `hover`, `pressed`, `disabled`, and `selected` anchors.
