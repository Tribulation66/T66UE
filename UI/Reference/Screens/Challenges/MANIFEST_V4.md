# Challenges MANIFEST V4

## Status

- Status: TRUE_BLOCKED_CAPTURE_FAILURE
- Target: Challenges
- Pass count: 1
- Built-in imagegen used: yes
- Reference-derived sheet generated: yes
- Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Challenges.png`
- Workspace: `C:\UE\T66\UI\Reference\Screens\Challenges`
- Runtime asset folder: `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges`

## Preflight

- `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md`: exists and read.
- `C:\UE\T66\Docs\UI\UI_GENERATION.md`: exists and read.
- `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1`: exists.
- `C:\UE\T66\Binaries\Win64\T66.exe`: exists.
- `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Challenges.png`: exists.
- `C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp`: exists.

## Geometry Map

1920x1080 reference-derived owned-area map:

- Fullscreen wood/chrome shell: x=0, y=0, w=1920, h=1080, role=9-slice/fullscreen frame, family=dark carved wood with restrained gold trim.
- Shared top bar/header/nav/currency/avatar/back/settings: x=0, y=0, w=1920, h=132, role=shared frozen top bar, family=out of scope.
- Challenges title and divider: x=66, y=142, w=1789, h=82, role=live title over thin divider/ornaments, family=live text plus small generated divider.
- Source tabs/create row: x=373, y=242, w=1215, h=60, spacing=44/50, role=horizontal 3-slice buttons, family=plain dark/brown and selected amber button plates.
- Status strip: x=43, y=314, w=1830, h=52, role=horizontal 3-slice strip, family=dark wood status bar.
- List panel: x=39, y=387, w=968, h=659, role=9-slice panel, family=parchment list well with gold/wood border.
- Challenge rows: x=66, y=407 and y=524, w=916, h=115, role=9-slice row plates with live text, family=parchment row plus simple dark socket.
- Detail panel: x=1028, y=387, w=845, h=659, role=9-slice panel, family=dark wood detail frame with parchment cards.
- Detail description card: x=1049, y=500, w=751, h=167, role=9-slice parchment card, family=quiet parchment.
- Rules card: x=1049, y=720, w=751, h=281, role=9-slice parchment card, family=quiet parchment.
- Scrollbar: x=1817, y=430, w=45, h=572, role=vertical controls, family=narrow dark rail with amber arrows/thumb.

## Pass 1 Imagegen

- Generated source kept by Codex: `C:\Users\DoPra\.codex\generated_images\019df98a-e356-7d53-97ed-9492c6b7c26b\ig_002ace75137168250169fa40a62c288198996ebf15a31d7de6.png`
- Workspace candidate: `C:\UE\T66\UI\Reference\Screens\Challenges\Working\Pass_01\Candidates\Challenges_textfree_sheet_pass01.png`
- Sprite sheet quality gate: PASS.
- Accepted reasons: text-free; preserves the reference's restrained dark wood, parchment, amber trim, simple button, socket, status bar, and scrollbar families; no live labels, numbers, title text, author/source names, CC values, rule text, player names, portraits, screenshots, or runtime text; geometry and component families correspond to the reference screen.
- Rejected sheets: none.

## Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Buttons\challenges_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_header_divider_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_state_socket_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Controls\challenges_controls_tag_pill_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_detail_frame_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_detail_paper_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_row_plate_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_rules_paper_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_challenge_status_bar_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_fullscreen_list_panel_v2.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges\Panels\challenges_panels_fullscreen_row_shell_quiet.png`

## Source And Build

- Source files changed: none.
- Build attempts: 0. Asset-only pass, so build skipped by V4 policy.
- Archived/reset asset paths: `C:\UE\T66\UI\Reference\Archive\PreV4_Reset_20260505_154740`

## Capture Attempts

- Attempt 1: delay=3.5, command=`powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen Challenges -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\Challenges\Proof\Challenges_pass00_working_1920x1080.png`, result=timeout, output exists=false.
- Attempt 2: delay=6, command=`powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen Challenges -ResX 1920 -ResY 1080 -DelaySeconds 6 -Output C:\UE\T66\UI\Reference\Screens\Challenges\Proof\Challenges_pass01_working_1920x1080.png`, result=timeout, output exists=false.
- Attempt 3: delay=10, command=`powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen Challenges -ResX 1920 -ResY 1080 -DelaySeconds 10 -Output C:\UE\T66\UI\Reference\Screens\Challenges\Proof\Challenges_pass01_working_1920x1080_delay10.png`, result=timeout, output exists=false.

## Remaining Differences

- Current implementation screenshot unavailable because all three required working capture attempts timed out without creating an output file.
- Visual comparison against runtime cannot be completed until capture works.

## Approved Differences

- Shared top bar/header/nav/currency/avatar/back/settings differences are out of scope for this Challenges target.
- Challenge labels, values, title, author/source, reward values, rule text, and runtime status copy remain live by target note.

## Next Action

- Resolve the working capture timeout or capture automation startup issue, then rerun the Pass 1 capture and compare against `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Challenges.png`.
