# Reference Generation Notes - temporary_buff_selection

- Chat number: 3
- Source current screenshot: `C:\UE\T66\UI\screens\temporary_buff_selection\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Layout/component list: `C:\UE\T66\UI\screens\temporary_buff_selection\layout\layout_list.md`
- Source screenshot status: existed before this chat's generation pass
- Raw generated source kept at: `C:\UE\T66\UI\screens\temporary_buff_selection\reference\temporary_buff_selection_raw_imagegen_1672x941.png`
- Normalization: raw `1672x941` was deterministically resampled to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Accepted output: `C:\UE\T66\UI\screens\temporary_buff_selection\reference\temporary_buff_selection_reference_1920x1080.png`
- Status: accepted for integrator review

## Runtime-Owned Regions To Preserve Later

- Title, instruction text, currency value, selected-buff slot states, buff card names, descriptions, owned/slotted counts, prices, buy labels, party labels and slot values, Done label, difficulty dropdown value, and Enter Tribulation label.
- Buff icons and party/selection slots are runtime-owned icon/state wells.
- Later runtime reconstruction must preserve the bottom command strip and scrollable card density; do not replace this with the temporary buff shop screen layout.
