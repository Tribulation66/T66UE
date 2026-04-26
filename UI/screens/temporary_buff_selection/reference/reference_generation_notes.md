# Temporary Buff Selection Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_selection\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\temporary_buff_selection\reference\temporary_buff_selection_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\temporary_buff_selection\reference\temporary_buff_selection_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: not forced; this is a run-flow selection surface rather than a main-menu/top-bar child screen.
- Back button handling: no standalone back button was generated.
- Replaced back affordance: no
- Preserved components: title and instruction, four top selection slots, Chad Coupons count, buff card grid with right scrollbar, bottom party panel with four party slots, `Done`, difficulty dropdown, and `Enter Tribulation`.
- Explicitly not added: portraits, hero cards, extra party controls, tabs, filters, extra currencies, extra grid rows, or footer navigation.

## Runtime-Owned Regions To Preserve Later

- Buff card data, owned/slotted counts, selection slots, party slot state, difficulty dropdown, and enter action remain runtime-owned.
- The card icon boxes are placeholders for runtime buff icons.
- Scroll position and clipped offscreen cards should be driven by the runtime list.
