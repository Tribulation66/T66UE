# Reference Generation Notes - pause_menu

- Chat number: 3
- Source current screenshot: `C:\UE\T66\UI\screens\pause_menu\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Layout/component list: `C:\UE\T66\UI\screens\pause_menu\layout\layout_list.md`
- Source screenshot status: existed before this chat's generation pass
- Raw generated source kept at: `C:\UE\T66\UI\screens\pause_menu\reference\pause_menu_raw_imagegen_1672x941.png`
- Normalization: raw `1672x941` was deterministically resampled to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Accepted output: `C:\UE\T66\UI\screens\pause_menu\reference\pause_menu_reference_1920x1080.png`
- Status: accepted for integrator review

## Runtime-Owned Regions To Preserve Later

- Modal title and all six button labels.
- Button states are generated control plates, but the labels stay live/localizable.
- Later runtime reconstruction must keep exactly six centered stacked buttons and no extra controls.
