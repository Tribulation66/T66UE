# Temporary Buff Shop Reference Generation Notes - V2

- Chat number: Chat 3
- Status: accepted
- Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_shop\current\current_state_1920x1080.png`
- Accepted reference: `C:\UE\T66\UI\screens\temporary_buff_shop\reference\temporary_buff_shop_reference_1920x1080.png`
- Raw imagegen output: `C:\UE\T66\UI\screens\temporary_buff_shop\reference\temporary_buff_shop_raw_imagegen_1672x941_v2.png`
- Normalization: deterministic resample to 1920x1080 with `C:\UE\T66\Scripts\InvokeDeterministicResample.py`

## Style Sources

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- 4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
- Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
- Imagegen chrome sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Layout And Rule Handling

- Current screenshot existed: yes
- Top bar: not forced; this screen preserves the current shop overlay structure.
- Back button handling: preserved `Back To Buffs` because it is an in-flow shop control present in the current screenshot.
- Replaced back affordance: no
- Preserved components: title, currency line, description, `Back To Buffs`, one currency icon, five columns of cards, three full visible rows, clipped fourth row, buy buttons, and right scrollbar.
- Explicitly not added: tabs, filters, weekly/expert/legacy labels, party slots, sidebars, extra currencies, extra rows, or footer navigation.

## Runtime-Owned Regions To Preserve Later

- Currency amount, shop item data, owned counts, buy button state, icon art, and scroll position remain runtime-owned.
- The clipped fourth row should remain a runtime scroll/list artifact, not a separate static footer.
