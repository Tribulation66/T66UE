# Challenges Pass 01 Notes

Authority:

- `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md`
- `C:\UE\T66\Docs\UI\UI_GENERATION.md`

Reference:

- `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Challenges.png`

Current runtime captures inspected:

- `C:\UE\T66\UI\screens\all_screens\current\2026-05-03\reference12_button_state_sweep\Challenges.png`
- `C:\UE\T66\UI\generation\ReferenceComponentSheets_2026-05-03\Batch03_FullscreenChrome\Proof\Challenges_V16_staged_1672x941.png`

## Visible UI Families

- Fullscreen/top-bar child screen shell and dark wood background.
- Center title, divider line treatment, source tabs, status bar, and list/detail body split.
- Left challenge browser panel with two row cards, state/icon socket, live title, live reward, live author/source, and live tag pill.
- Right detail panel with live title, live author/source, live description, rules heading, live rule rows, live reward/selection copy, confirm button, and scrollbar.
- Button plates, row plates, parchment panels, tag pill, state socket, status strip, and scrollbar.

## First Difference List

- `layout`: Current runtime placed the title and source tabs too far left/down compared with the centered reference.
- `asset`: Current list rows used generic dark wood row/button chrome instead of parchment challenge browser rows.
- `asset`: Current detail description and rules areas were mostly live text over dark wood/generic row shells instead of parchment paper panels.
- `asset`: Current origin badge was a primitive flat rectangle instead of a generated tag pill.
- `asset`: Current row state socket reused button art instead of a fixed square socket.
- `asset`: Current status message had no generated status bar chrome.
- `asset`: Current buttons still routed to `SourceAssets/UI/Reference/Shared/Buttons/Pill` instead of Challenges-owned button assets.
- `text-fit`: Current detail title, rule text, and reward text were too large and wrapped/clipped compared with the reference.
- `resize-contract`: Current row/detail/status component usage was not all declared with target-specific resize contracts.

## Accepted Assets

Generated candidate source:

- `C:\Users\DoPra\.codex\generated_images\019df090-e767-7642-b9c8-086137e4da66\ig_0fcb5631103e35fb0169f7f48a8b748198a4e17ea648d50910.png`

Workspace copies:

- `C:\UE\T66\UI\Reference\Screens\Challenges\RawSources\challenges_component_board_v1_chromakey.png`
- `C:\UE\T66\UI\Reference\Screens\Challenges\Candidates\challenges_component_board_v1_alpha.png`
- `C:\UE\T66\UI\Reference\Screens\Challenges\challenges_component_assets_manifest_v1.json`

Runtime assets promoted under `C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges`:

- `Panels\challenges_panels_challenge_row_plate_v1.png` - 9-slice row plate.
- `Panels\challenges_panels_challenge_detail_paper_v1.png` - 9-slice parchment detail/description panel.
- `Panels\challenges_panels_challenge_rules_paper_v1.png` - 9-slice rules panel.
- `Panels\challenges_panels_challenge_status_bar_v1.png` - horizontal sliced status bar.
- `Controls\challenges_controls_tag_pill_v1.png` - horizontal sliced tag pill.
- `Controls\challenges_controls_state_socket_v1.png` - fixed square socket.
- `Controls\challenges_controls_header_divider_v1.png` - horizontal sliced divider candidate.
- `Buttons\challenges_buttons_pill_normal.png`
- `Buttons\challenges_buttons_pill_hover.png`
- `Buttons\challenges_buttons_pill_pressed.png`
- `Buttons\challenges_buttons_pill_disabled.png`
- `Buttons\challenges_buttons_pill_selected.png`

## Runtime Changes

- Routed Challenges button brushes to `SourceAssets/UI/Reference/Screens/Challenges/Buttons`.
- Replaced challenge rows with generated row plates and live Slate text/data.
- Replaced flat origin badges with generated tag pill chrome and live label text.
- Replaced row check/selection art with a generated fixed socket and live confirmed marker.
- Added generated status-bar chrome with live status text.
- Added generated parchment panels for the list backing, description, and rules list.
- Adjusted Challenges layout toward the reference: centered title, centered source tab row, status strip, and lower list/detail panels.
- Reduced detail/list text sizes and removed purple accent colors from the reference-facing live text.

## Verification

Completed:

- `git diff --check -- Source/T66/UI/Screens/T66ChallengesScreen.cpp SourceAssets/UI/Reference/Screens/Challenges UI/Reference/Screens/Challenges`
- `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -SingleFile=C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp`

Blocked:

- Full stage/capture could not complete. `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development` first hit an UnrealBuildTool mutex conflict.
- After waiting, full target builds were blocked by unrelated compile errors outside this target:
  - `C:\UE\T66\Source\T66TD\Private\UI\Screens\T66TDBattleScreen.cpp(724)` - `SLATE_ARGUMENT(TMap<FName, float>, BattleTuningValues)` macro parse failure.
  - `C:\UE\T66\Source\T66\UI\Screens\T66CompanionSelectionScreen.cpp(1502)` - `TSharedRef<SBox>` assignment from `SNew(SVerticalBox)`.

Because the standalone `T66.exe` was not rebuilt, no packaged Challenges proof capture was produced for this pass.
