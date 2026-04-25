# Mini Main Menu Layout List

Canvas: 1920x1080.

Source evidence:
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:293` through `300` defines left/right panel size, center button dimensions, and title top padding.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:302` through `325` builds live friends and party state with local-player fallback.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:459` through `552` builds friend rows, invite/join controls, and online/offline/pending/in-party states.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:574` through `613` builds 4 party slots with live avatar/status text.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:615` through `676` builds the left friends/profile/party shell.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:678` through `740` builds the center Start/Continue button stack.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:757` through `831` places the animated background, title, Back button, left panel, center buttons, and right leaderboard.
- `Source/T66Mini/Private/UI/Components/T66MiniLeaderboardPanel.cpp:23` through `195` defines leaderboard filters, dropdowns, entry list, and status text.
- `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp:954` through `958` loads the current background and CTA source art from `SourceAssets/UI/MainMenuReference`.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\mini_main_menu\current\2026-04-24\current_1920x1080.png`.
- Layout list: `C:\UE\T66\UI\screens\mini_main_menu\layout\layout_list.md`
- Generated screen-specific reference: TODO/missing. Required path is `C:\UE\T66\UI\screens\mini_main_menu\reference\canonical_reference_1920x1080.png`.

Reference gate status: blocked. The canonical main-menu anchor and current target screenshot exist, but the generated screen-specific reference is missing. Do not proceed to sprite generation, runtime styling, or packaged comparison for this screen until the generated target reference exists.

## Regions

- Full-screen animated background: runtime animated layer using the main-menu generated scene background; generated target reference should preserve the menu composition but final scene plate must be free of panels, controls, text, and live content.
- Dark overlay tint: subtle full-screen overlay above the background.
- Top title: centered `MINI CHADPOCALYPSE` double-layer title with shadow; current source renders it as runtime text.
- Back button: top-left `BACK` navigation button.
- Left social shell: bottom-left 382x562 panel, clipped to bounds, containing profile, friend search, online/offline friend list, and party slots.
- Profile card: local avatar, local display name, Steam status, and static profile meta copy.
- Friend search field: editable text box filtering online/offline friend rows.
- Friends list: scrollable friend rows grouped by online/offline counts.
- Friend row: avatar socket, display name, presence/status, Invite button, Join button.
- Party strip: 4 party slots with avatar/fallback, display name, and status text.
- Center CTA shell: bottom-centered 392 px wide panel with Start and Continue buttons.
- Start button: green primary button with animated border glow.
- Continue button: enabled only when a local mini save exists and no remote party members are present.
- Right leaderboard shell: bottom-right 382x620 panel containing leaderboard title, description, filter buttons, dropdowns, entries, and status text.
- Leaderboard filter row: Global/Friends filter buttons.
- Leaderboard dropdown row: party size dropdown and difficulty dropdown.
- Leaderboard entry list: scroll area with rank, display name, local-player marker, and score.
- Leaderboard status footer: live status/help/error copy.

## Panels And Shells

- Full-screen scene plate: generated background only; no UI, title, labels, panels, or buttons.
- Left social panel shell: generated fantasy frame with dark interior and scrollable content aperture.
- Profile card shell: generated nested row/card frame with avatar aperture.
- Friend row shell: generated row frame with avatar aperture and two button wells.
- Search field shell: generated input frame with live editable text.
- Party slot shell: generated avatar/status card frame for 4 party members.
- Center CTA shell: generated compact frame around Start/Continue stack.
- Right leaderboard panel shell: generated fantasy frame with leaderboard chrome.
- Leaderboard row shell: generated entry row backplate with live text/value wells.
- Dropdown shell family: generated party-size and difficulty combo shells.
- Button plate families: Back neutral, Start green primary/glow, Continue enabled/disabled, Global/Friends selected/unselected, Invite/Join enabled/disabled; labels remain live.

## Controls And Required States

- Back button: real visible button; states `normal`, `hover`, `pressed`, `disabled`.
- Start button: real visible button; states `normal`, `hover`, `pressed`, `disabled`, `glow-active`.
- Continue button: real visible button; states `normal`, `hover`, `pressed`, `disabled`.
- Search field: real editable field; states `empty`, `focused`, `filled`, `disabled`.
- Invite button: states `normal`, `hover`, `pressed`, `disabled`, `in-party`, `invite-pending`.
- Join button: states `normal`, `hover`, `pressed`, `disabled`.
- Friend row: states `online`, `offline`, `in-party`, `invite-pending`.
- Party slot: states `empty`, `local`, `host`, `remote`, `ready`, `not-ready`, `screen-status`.
- Leaderboard Global/Friends filters: states `normal`, `hover`, `pressed`, `selected`, `disabled`.
- Party size dropdown: states `closed`, `open`, `hover`, `selected`, `disabled`; options Solo, Duo, Trio, Quad.
- Difficulty dropdown: states `closed`, `open`, `hover`, `selected`, `disabled`; options come from mini data with fallback Easy.
- Leaderboard entry row: states `normal`, `local-player`, `empty/status`.
- Leaderboard scroll region: scrollbar states `normal`, `hover`, `dragged`, `disabled` if visible.

## Live Runtime Content

- Live/localizable text: Back, title, Start, Continue, profile status, profile meta copy, search hint, Online/Offline headers, Invite/Invited/In Party, Join, Party, leaderboard title, leaderboard description, Global/Friends, dropdown labels, leaderboard status.
- Live values: online/offline friend counts, party member status, lobby readiness, selected frontend screen labels, selected hero label, party size, difficulty, leaderboard ranks, scores, status messages.
- Live images/media: animated background brush, local avatar, friend avatars, party avatars.
- Live state: Steam readiness, local display name, friends list, friend search query, invitation pending status, party membership, remote party member presence, mini save availability, leaderboard filter, party size, difficulty, cached leaderboard entries.
- Strict diff strategy: use fixture account/session with deterministic local display name, no remote party members, fixed friend list, known mini save availability, and cached leaderboard entries; mask avatar wells if real Steam avatars cannot be frozen.

## Variants

- Steam available versus unavailable profile state.
- No friends, mixed online/offline friends, and filtered search query.
- Invite enabled, invited/pending, in-party, offline disabled.
- Join enabled versus disabled.
- Local-only party and 4-member party.
- Party host, guest, ready, not-ready, and mini-flow screen statuses.
- Start hovered/glow active.
- Continue enabled with local save and disabled without save or with remote party members.
- Leaderboard Global and Friends selected.
- Leaderboard party-size options Solo/Duo/Trio/Quad.
- Leaderboard difficulty options from mini data plus fallback Easy.
- Empty leaderboard/status message and populated leaderboard with local-player highlighted row.

## Reference Generation Constraints

- Preserve the current three-zone layout: social panel bottom-left, CTA stack bottom-center, leaderboard bottom-right, Back top-left, title top-center.
- Match the canonical main-menu style with the same dark fantasy scene language, chunky framed shells, warm metal accents, jewel highlights, and readable dark interiors.
- Do not bake local display names, friend names, presence text, party statuses, leaderboard ranks/scores, dropdown values, button labels, or title text into runtime-intended sprites unless a future art decision explicitly approves title art.
- Leave avatar wells, friend rows, party slots, leaderboard entries, editable search text, dropdown values, and button text as runtime-owned apertures.
- The full generated reference may include representative readable text for planning, but production sprites derived later must be text-free except approved display art.
