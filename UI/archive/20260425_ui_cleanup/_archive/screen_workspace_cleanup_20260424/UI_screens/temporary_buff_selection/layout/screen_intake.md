# Temporary Buff Selection Screen Intake

## Identity

- Screen token: `TemporaryBuffSelection`
- Display name: `Temporary Buff Selection Modal`
- Owner: TODO
- Target platform: PC/controller frontend modal

## Canonical Canvas

- Offline comparison target path: TODO/missing `C:\UE\T66\UI\screens\temporary_buff_selection\reference\canonical_reference_1920x1080.png`
- Offline comparison target resolution: `1920x1080`
- Hi-res helper/reference path: TODO/missing
- Hi-res helper/reference resolution: TODO
- UI-free scene/background plate path: not applicable for this modal; underlying screen remains runtime-owned and dimmed by scrim
- Runtime target viewport: `1920x1080`
- Packaged acceptance capture resolution: `1920x1080`
- Calibration anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`

## Blocking Style-Reference Gate

- Canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target runtime screenshot path: `C:\UE\T66\UI\screens\temporary_buff_selection\current\2026-04-24\current_1920x1080.png`.
- Layout list path: `C:\UE\T66\UI\screens\temporary_buff_selection\layout\layout_list.md`
- Image generation used all three required inputs: no
- Generated screen-specific reference path: TODO/missing `C:\UE\T66\UI\screens\temporary_buff_selection\reference\canonical_reference_1920x1080.png`
- Generated reference preserves target layout: no, missing
- Generated reference matches canonical main-menu style: no, missing
- Gate verdict: blocked

## Source Inventory

- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66TemporaryBuffSelectionScreen.h`
- `Source/T66/Core/T66BuffSubsystem.h`
- `Source/T66/Core/T66BuffSubsystem.cpp`
- `Source/T66/UI/T66TemporaryBuffUIUtils.cpp`
- `Source/T66/UI/Style/T66Style.h`

## Reference Variants

- Primary comparison frame: TODO/missing `C:\UE\T66\UI\screens\temporary_buff_selection\reference\canonical_reference_1920x1080.png`
- No-buttons variant for analysis/prompting only: TODO
- No-text variant for analysis/prompting only: TODO
- No-dynamic variant for analysis/prompting only: TODO

## Regions

- `fullscreen_scrim` - stateful backdrop dim over prior screen.
- `modal_content_shell` - static generated-shell frame for the full modal.
- `header_title_block` - live text.
- `coupon_balance_well` - generated-shell with live label and live value.
- `loadout_slot_row` - stateful 4-slot control row with live icons.
- `buff_grid_scroll` - live scroll well containing buff cards.
- `buff_card_grid` - stateful repeated card family.
- `footer_party_panel` - live avatar sockets and live fallback initials.
- `footer_done_button` - stateful real button.
- `footer_difficulty_dropdown` - stateful real dropdown.
- `footer_enter_button` - stateful real CTA.

## Element Checklist

- `modal_content_shell` - family: `Center/ModalShell` - ownership: static generated-shell - states required: `normal` - status: TODO.
- `row_shell` - family: `Center/RowShell` - ownership: static generated-shell - states required: `normal` - status: TODO.
- `buff_card_shell` - family: `Center/Card` - ownership: stateful shell with live apertures - states required: `normal`, `focused-match`, `disabled` - status: TODO.
- `loadout_slot_shell` - family: `Center/Slot` - ownership: stateful shell with runtime-icon aperture - states required: `empty`, `filled`, `focused`, `unowned` - status: TODO.
- `avatar_slot_frame` - family: `Footer/Party` - ownership: runtime-avatar aperture - states required: `empty`, `filled`, `host/local` - status: TODO.
- `compact_button_plate` - family: `Button` - ownership: stateful real button plate - states required: `normal`, `hover`, `pressed`, `disabled` - status: TODO.
- `toggle_button_plate` - family: `Button` - ownership: stateful real button plate - states required: `normal`, `hover`, `pressed`, `selected`, `disabled` - status: TODO.
- `green_cta_button_plate` - family: `Button` - ownership: stateful real button plate - states required: `normal`, `hover`, `pressed`, `disabled` - status: TODO.
- `dropdown_shell` - family: `Control` - ownership: stateful real dropdown shell - states required: `closed`, `open`, `hover`, `selected`, `disabled` - status: TODO.
- `scrollbar` - family: `Control` - ownership: stateful scroll control - states required: `normal`, `hover`, `dragged`, `disabled` - status: TODO if visible in current capture.

## Dynamic And Live Content

- Live text sources: title, helper copy, coupon label, slot action labels, buff names, buff descriptions, owned/slotted count labels, card action labels, party header, difficulty options, Done, Enter Tribulation.
- Live value sources: Chad Coupon balance, single-use buff cost, owned counts, assigned counts, selected difficulty, focused slot index.
- Live image/media sources: buff icons from secondary buff runtime dependencies, Steam avatars, local avatar texture.
- Freeze strategy for strict diffing: staged save/profile with known coupon balance, selected slots, owned counts, selected difficulty, and local-only party; fixed scroll at top.
- Regions excluded from strict diff: avatar image wells, translated/localized dynamic text if locale changes, buff count values if fixture is not locked.
- Manual validation required for: dropdown open state, scroll behavior, buy/use enablement, party fallback avatars.

## Family Boards Needed

- `TopBar`: no
- `Center`: yes
- `LeftPanel`: no
- `RightPanel`: no
- `LeaderboardChrome`: no
- `CTAStack`: no
- `Decor`: no
- `ModalShell`: yes
- `BuffCards`: yes
- `Buttons`: yes
- `Dropdown`: yes
- `PartyAvatarSlots`: yes

## Runtime Rules

- Localizable controls must remain text-free in runtime art: yes
- Live numerals must remain text-free in runtime art: yes
- Full reference/buttonless/textless master may be used as runtime background: no
- Scene plate must be UI-free: not applicable; modal uses scrim over runtime-owned previous screen
- Visible control must be the real button: yes
- Any plate expected to stretch must be authored as a true nine-slice: yes
- Baked display art allowed only for: none
- Tagline/subtitle remains live/localizable: yes

## Layout Artifacts

- `reference_layout.json`: TODO
- Generated layout header: TODO
- Coordinate authority: current source and future `reference_layout.json`
- Shell rect list: TODO
- Visible control rect list: TODO
- Live-content rect list: TODO

## Asset Validation

- Asset manifest: TODO
- Scene plate ownership validation: not applicable
- Dimension validation: TODO
- Alpha validation: TODO
- Nine-slice margin validation: TODO
- State anchor validation: TODO

## Typography Notes

- Primary display font target: `FT66Style::Tokens::FontBold`, title size 24 in current source.
- Secondary/supporting font target: `FT66Style::Tokens::FontRegular` and `FontBold`, current source ranges from 8 to 16 for card/supporting text.
- Special casing or locale rules: all labels are runtime/localizable; generated assets must not bake these strings.

## Prompt Pack

- Offline target prompt: TODO
- Scene plate prompt: not applicable for this modal
- Family board prompts: TODO
- Source images attached to family prompts: canonical main-menu anchor, current target screenshot, this layout list, plus accepted full target reference after gate completion.

## Validation

- Packaged screenshot command: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen TemporaryBuffSelection -Output C:\UE\T66\UI\screens\temporary_buff_selection\outputs\YYYY-MM-DD\packaged_capture.png`
- Packaged capture path: TODO
- Scene plate contamination check: not applicable
- Foreground component ownership check: TODO
- Strict-diff regions: modal shell, loadout shells, buff card shell positions, footer control placement.
- Diff mask paths or notes: mask avatar wells and dynamic text/value wells unless fixture-locked.
- Manual validation regions: dropdown, scroll, button states, avatar fallback states.
- Acceptance bar: blocked until current screenshot and generated screen-specific reference exist.

## Open Questions

- Which deterministic save/profile state should be used for the current screenshot fixture?
- Should the reference show local-only party or a full 4-member party for avatar socket planning?
