# Reference Generation Notes

- Chat number: Chat 1
- Source current screenshot path: `C:\UE\T66\UI\screens\account_status\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed at generation time
- Generation path: Codex-native `image_gen`
- Raw generated source: `C:\UE\T66\UI\screens\account_status\reference\account_status_raw_imagegen_1672x941.png`
- Canonical reference: `C:\UE\T66\UI\screens\account_status\reference\account_status_reference_1920x1080.png`
- Normalization: raw `1672x941` image was normalized to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Runtime-owned regions to preserve later: top navigation labels/icons/currency, account standing text, appeal status, progress labels/values/fills, dropdown values, table headers, table rows, ranks, scores, times, empty-state text, and scroll state
- Reference status: accepted for first reference gate
- Notes for integrator: offline comparison target only; do not use the full-screen reference as runtime background art
