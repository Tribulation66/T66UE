# Reference Generation Notes

- Chat number: Chat 1
- Source current screenshot path: `C:\UE\T66\UI\screens\achievements\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed at generation time
- Generation path: Codex-native `image_gen`
- Raw generated source: `C:\UE\T66\UI\screens\achievements\reference\achievements_raw_imagegen_1672x941.png`
- Canonical reference: `C:\UE\T66\UI\screens\achievements\reference\achievements_reference_1920x1080.png`
- Normalization: raw `1672x941` image was normalized to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Runtime-owned regions to preserve later: top navigation labels/icons/currency, category tab labels/states, progress summary count/percent/fill, achievement names/descriptions, row progress values, reward values, star state, and scroll state
- Reference status: accepted for first reference gate
- Notes for integrator: offline comparison target only; row data and reward state must remain live during runtime reconstruction
