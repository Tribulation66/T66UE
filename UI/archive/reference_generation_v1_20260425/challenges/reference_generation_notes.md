# Reference Generation Notes - challenges

- Chat number: 3
- Source current screenshot: `C:\UE\T66\UI\screens\challenges\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Layout/component list: `C:\UE\T66\UI\screens\challenges\layout\layout_list.md`
- Source screenshot status: existed before this chat's generation pass
- Raw generated source kept at: `C:\UE\T66\UI\screens\challenges\reference\challenges_raw_imagegen_1672x941.png`
- Normalization: raw `1672x941` was deterministically resampled to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Accepted output: `C:\UE\T66\UI\screens\challenges\reference\challenges_reference_1920x1080.png`
- Status: accepted for integrator review

## Runtime-Owned Regions To Preserve Later

- Challenge tab labels, category tabs, status text, row names, rewards, category badges, author/source text, selected-challenge detail text, rule text, reward values, and confirm label.
- Challenge row icon/checkbox sockets and selected state.
- Later runtime reconstruction must keep the two-column shell and mask/replace text and icon wells with live Slate data.

## Notes

- An earlier generated draft added unowned sidebar and extra tabs; that draft was rejected and not copied into this screen pack.
- The accepted reference was generated only after the exact component list was written.
