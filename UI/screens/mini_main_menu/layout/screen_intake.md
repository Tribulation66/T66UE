# Mini Main Menu Screen Intake

## Identity

- Screen token: `MiniMainMenu`
- Display name: `Mini Chadpocalypse Main Menu`
- Owner: TODO
- Target platform: PC/controller frontend screen

## Canonical Canvas

- Offline comparison target path: TODO/missing `C:\UE\T66\UI\screens\mini_main_menu\reference\canonical_reference_1920x1080.png`
- Offline comparison target resolution: `1920x1080`
- Hi-res helper/reference path: TODO/missing
- Hi-res helper/reference resolution: TODO
- UI-free scene/background plate path: TODO. Current source uses `SourceAssets/UI/MainMenuReference/scene_background_1920x1080.png`, but the screen-specific UI-free plate has not been produced in this screen pack.
- UI-free scene/background plate resolution: `1920x1080`
- Runtime target viewport: `1920x1080`
- Packaged acceptance capture resolution: `1920x1080`
- Calibration anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`

## Blocking Style-Reference Gate

- Canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target runtime screenshot path: `C:\UE\T66\UI\screens\mini_main_menu\current\2026-04-24\current_1920x1080.png`.
- Layout list path: `C:\UE\T66\UI\screens\mini_main_menu\layout\layout_list.md`
- Image generation used all three required inputs: no
- Generated screen-specific reference path: TODO/missing `C:\UE\T66\UI\screens\mini_main_menu\reference\canonical_reference_1920x1080.png`
- Generated reference preserves target layout: no, missing
- Generated reference matches canonical main-menu style: no, missing
- Gate verdict: blocked

## Source Inventory

- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp`
- `Source/T66Mini/Public/UI/Screens/T66MiniMainMenuScreen.h`
- `Source/T66Mini/Private/UI/Components/T66MiniLeaderboardPanel.cpp`
- `Source/T66Mini/Public/UI/Components/T66MiniLeaderboardPanel.h`
- `Source/T66Mini/Private/UI/T66MiniUIStyle.h`
- `Source/T66/UI/Style/T66Style.h`

## Reference Variants

- Primary comparison frame: TODO/missing `C:\UE\T66\UI\screens\mini_main_menu\reference\canonical_reference_1920x1080.png`
- No-buttons variant for analysis/prompting only: TODO
- No-text variant for analysis/prompting only: TODO
- No-dynamic variant for analysis/prompting only: TODO

## Regions

- `background_scene_plate` - static/generated UI-free scene plate, currently missing in this screen pack.
- `screen_tint_overlay` - stateful runtime tint overlay.
- `title_lockup` - live runtime text in current source.
- `back_button` - stateful real button.
- `left_social_shell` - live/stateful panel.
- `profile_card` - live avatar/text.
- `friend_search_field` - stateful editable control.
- `friend_list_scroll` - live scroll region.
- `friend_row` - repeated live/stateful row.
- `party_strip` - live/stateful 4-slot party region.
- `center_cta_shell` - stateful button stack.
- `start_button` - stateful real CTA with animated glow.
- `continue_button` - stateful real CTA, conditionally disabled.
- `right_leaderboard_shell` - live/stateful panel.
- `leaderboard_filter_buttons` - stateful real buttons.
- `leaderboard_dropdowns` - stateful real combo boxes.
- `leaderboard_entries` - live scroll region.
- `leaderboard_status_footer` - live text.

## Element Checklist

- `background_scene_plate` - family: `ScenePlate` - ownership: static UI-free plate - states required: `normal` - status: TODO.
- `left_social_panel_shell` - family: `LeftPanel` - ownership: generated-shell with live apertures - states required: `normal` - status: TODO.
- `profile_card_shell` - family: `LeftPanel/Profile` - ownership: runtime-avatar aperture and runtime-text wells - states required: `normal`, `steam-unavailable` - status: TODO.
- `search_field_shell` - family: `LeftPanel/Input` - ownership: real editable field - states required: `empty`, `focused`, `filled`, `disabled` - status: TODO.
- `friend_row_shell` - family: `LeftPanel/FriendRow` - ownership: runtime-avatar/text apertures - states required: `online`, `offline`, `in-party`, `invite-pending` - status: TODO.
- `party_slot_shell` - family: `LeftPanel/Party` - ownership: runtime-avatar/text apertures - states required: `empty`, `local`, `host`, `remote`, `ready` - status: TODO.
- `center_cta_shell` - family: `CTAStack` - ownership: generated-shell - states required: `normal` - status: TODO.
- `button_plate_neutral` - family: `Buttons` - ownership: stateful real button plate - states required: `normal`, `hover`, `pressed`, `disabled` - status: TODO.
- `button_plate_green` - family: `Buttons` - ownership: stateful real button plate - states required: `normal`, `hover`, `pressed`, `disabled`, `glow-active` - status: TODO.
- `right_leaderboard_shell` - family: `RightPanel/LeaderboardChrome` - ownership: generated-shell with live apertures - states required: `normal` - status: TODO.
- `leaderboard_filter_button` - family: `LeaderboardChrome` - ownership: stateful real button plate - states required: `normal`, `hover`, `pressed`, `selected`, `disabled` - status: TODO.
- `dropdown_shell` - family: `LeaderboardChrome` - ownership: stateful real dropdown shell - states required: `closed`, `open`, `hover`, `selected`, `disabled` - status: TODO.
- `leaderboard_entry_row` - family: `LeaderboardChrome` - ownership: runtime-text/value wells - states required: `normal`, `local-player`, `empty/status` - status: TODO.
- `scrollbar` - family: `Control` - ownership: stateful scroll control - states required: `normal`, `hover`, `dragged`, `disabled` - status: TODO if visible in current capture.

## Dynamic And Live Content

- Live text sources: title, Back, Start, Continue, profile status/meta, friend search hint, friend display names, presence text, Invite/Invited/In Party, Join, Party, party status strings, leaderboard headings/help copy, filter labels, dropdown labels, status text.
- Live value sources: online/offline counts, party member status, selected frontend screen labels, selected hero label, party size, difficulty, leaderboard rank, score, cache/status messages.
- Live image/media sources: animated background brush, local Steam avatar, friend Steam avatars, party Steam avatars.
- Freeze strategy for strict diffing: deterministic local profile, fixed friend list, fixed party state, fixed mini save presence, cached leaderboard fixture, fixed background animation time or still capture.
- Regions excluded from strict diff: avatar wells, leaderboard values if live services update, friend presence copy if not fixture-locked, animated background if not captured at deterministic time.
- Manual validation required for: search filtering, invite/join enablement, Continue enablement, dropdown option menus, leaderboard filter refresh, animated glow.

## Family Boards Needed

- `TopBar`: no
- `Center`: yes
- `LeftPanel`: yes
- `RightPanel`: yes
- `LeaderboardChrome`: yes
- `CTAStack`: yes
- `Decor`: no
- `ScenePlate`: yes
- `Buttons`: yes
- `Dropdown`: yes
- `FriendRows`: yes
- `PartySlots`: yes

## Runtime Rules

- Localizable controls must remain text-free in runtime art: yes
- Live numerals must remain text-free in runtime art: yes
- Full reference/buttonless/textless master may be used as runtime background: no
- Scene plate must be UI-free: yes
- Visible control must be the real button: yes
- Any plate expected to stretch must be authored as a true nine-slice: yes
- Baked display art allowed only for: none currently; title is runtime text in source
- Tagline/subtitle remains live/localizable: yes where present

## Layout Artifacts

- `reference_layout.json`: TODO
- Generated layout header: TODO
- Coordinate authority: current source and future `reference_layout.json`
- Shell rect list: TODO
- Visible control rect list: TODO
- Live-content rect list: TODO

## Asset Validation

- Asset manifest: TODO
- Scene plate ownership validation: TODO
- Dimension validation: TODO
- Alpha validation: TODO
- Nine-slice margin validation: TODO
- State anchor validation: TODO

## Typography Notes

- Primary display font target: current source uses `FT66Style::MakeFont("Black", 40)` for the title and `T66MiniUI::TitleFont(24)` in leaderboard.
- Secondary/supporting font target: current source uses `FT66Style::MakeFont("Regular"/"Bold", 9-16)` and `T66MiniUI::BodyFont/BoldFont`.
- Special casing or locale rules: labels and values remain runtime-owned; generated assets must not bake localizable strings or live names.

## Prompt Pack

- Offline target prompt: TODO
- Scene plate prompt: TODO
- Family board prompts: TODO
- Source images attached to family prompts: canonical main-menu anchor, current target screenshot, this layout list, plus accepted full target reference after gate completion.

## Validation

- Packaged screenshot command: `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen MiniMainMenu -Output C:\UE\T66\UI\screens\mini_main_menu\outputs\YYYY-MM-DD\packaged_capture.png`
- Packaged capture path: TODO
- Scene plate contamination check: TODO
- Foreground component ownership check: TODO
- Strict-diff regions: panel shells, button plate locations, title/back/CTA/leaderboard placement, non-live background composition if animation is frozen.
- Diff mask paths or notes: mask avatar wells, live friend/presence text, leaderboard values, and animated background if not deterministic.
- Manual validation regions: search field, friend row controls, party status variants, leaderboard filters/dropdowns, Continue enablement.
- Acceptance bar: blocked until current screenshot and generated screen-specific reference exist.

## Open Questions

- Should the approved target reference keep the title as runtime text or convert it to approved display art later?
- Which fixture state should represent friends, party members, mini save availability, and leaderboard rows in the current screenshot?
