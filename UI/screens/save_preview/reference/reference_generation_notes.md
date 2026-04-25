# Reference Generation Notes

- Chat number: Chat 1
- Source current screenshot path: `C:\UE\T66\UI\screens\save_preview\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed at generation time
- Generation path: Codex-native `image_gen`
- Raw generated source: `C:\UE\T66\UI\screens\save_preview\reference\save_preview_raw_imagegen_1672x941.png`
- Canonical reference: `C:\UE\T66\UI\screens\save_preview\reference\save_preview_reference_1920x1080.png`
- Normalization: raw `1672x941` image was normalized to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Runtime-owned regions to preserve later: dimmed main-menu social and leaderboard content, title/tagline, modal title/body copy, button labels/states, top navigation labels/icons/currency, avatars, ranks, names, and scores
- Reference status: accepted for first reference gate
- Notes for integrator: offline comparison target only; modal shell can inform later asset work, but background live content must remain masked/runtime-owned
