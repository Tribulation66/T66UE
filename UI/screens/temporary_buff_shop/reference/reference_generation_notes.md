# Reference Generation Notes - temporary_buff_shop

- Chat number: 3
- Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_shop\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Layout/component list: `C:\UE\T66\UI\screens\temporary_buff_shop\layout\layout_list.md`
- Source screenshot status: captured during this chat with `Scripts\CaptureT66UIScreen.ps1 -Screen TemporaryBuffShop`
- Raw generated source kept at: `C:\UE\T66\UI\screens\temporary_buff_shop\reference\temporary_buff_shop_raw_imagegen_1672x941.png`
- Normalization: raw `1672x941` was deterministically resampled to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Accepted output: `C:\UE\T66\UI\screens\temporary_buff_shop\reference\temporary_buff_shop_reference_1920x1080.png`
- Status: accepted for integrator review

## Runtime-Owned Regions To Preserve Later

- Title, currency value, explanatory description, Back To Buffs label, buff card names, effect text, owned counts, prices, and buy labels.
- Buff icons are runtime-owned image wells.
- Later runtime reconstruction must preserve this as a temporary-buff-shop-only screen with no party strip, difficulty dropdown, Done button, or Enter Tribulation CTA.
