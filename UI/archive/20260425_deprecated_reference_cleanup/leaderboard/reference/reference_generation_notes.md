# Reference Generation Notes

- Chat number: Chat 1
- Source current screenshot path: `C:\UE\T66\UI\screens\leaderboard\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot status: existed at generation time
- Generation path: Codex-native `image_gen`
- Raw generated source: `C:\UE\T66\UI\screens\leaderboard\reference\leaderboard_raw_imagegen_1672x941.png`
- Canonical reference: `C:\UE\T66\UI\screens\leaderboard\reference\leaderboard_reference_1920x1080.png`
- Normalization: raw `1672x941` image was normalized to `1920x1080` with `Scripts\InvokeDeterministicResample.py`
- Runtime-owned regions to preserve later: title, back label, icon tab states, Weekly/All Time tab labels/states, dropdown values/options, Score/Speed Run labels/states, ranks, names, avatars, scores, favorite state, and row count
- Reference status: accepted for first reference gate
- Notes for integrator: offline comparison target only; leaderboard rows and avatar sockets must be masked/runtime-owned during later reconstruction
