# Reference Generation Notes - run_summary

- Chat number: 3
- Source current screenshot: `C:\UE\T66\UI\screens\run_summary\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Layout/component list: `C:\UE\T66\UI\screens\run_summary\layout\layout_list.md`
- Source screenshot status: existed before this chat's generation pass
- Raw generated source kept at: `C:\UE\T66\UI\screens\run_summary\reference\run_summary_raw_imagegen_1672x941.png`
- Normalization: raw `1672x941` was deterministically resampled to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Accepted output: `C:\UE\T66\UI\screens\run_summary\reference\run_summary_reference_1920x1080.png`
- Status: accepted for integrator review

## Runtime-Owned Regions To Preserve Later

- Stage reached, score, time, weekly/all-time rank values, seed luck value, integrity status, button labels, stats rows, damage table headers and rows.
- Center 3D preview is a runtime open aperture.
- Buff slots and item/reward slots are runtime-owned image/state wells.
- Later runtime reconstruction must keep the preview, slot rows, rank panels, stats panel, and damage table as separate masked/live regions.
