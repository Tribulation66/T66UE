# Run Summary Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\run_summary\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\run_summary\reference\run_summary_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\run_summary\reference\run_summary_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: not forced; this is an end-of-run summary surface.
- Back button handling: no standalone back button was generated.
- Replaced back affordance: no
- Preserved components: title, stage/score/time line, `Event Log`, `Weekly Rank`, `All Time Rank`, seed luck, integrity, `Go Again!`, `Main Menu`, center preview, four buff slots, ten bottom item slots, stats panel, damage table, and right scrollbar.
- Explicitly not added: charts, medals, badges, extra ranking tabs, expert/legacy labels, filters, sidebars, extra buttons, extra currencies, or footer navigation.

## Runtime-Owned Regions To Preserve Later

- Rank values, stage/score/time, seed luck, integrity text, live preview, buff slots, item slots, stats, damage rows, event log action, and action buttons remain runtime-owned.
- The center preview should remain a live/runtime preview region rather than baked static character art.
