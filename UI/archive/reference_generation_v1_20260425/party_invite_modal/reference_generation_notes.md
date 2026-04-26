# Reference Generation Notes

- Chat number: 2
- Screen slug: `party_invite_modal`
- Source current screenshot: `C:\UE\T66\UI\screens\party_invite_modal\current\current_state_1920x1080.png`
- Anchor path: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Source screenshot state: existed before generation
- Raw native image output: `C:\Users\DoPra\.codex\generated_images\019dc448-af5d-72b1-8abf-524ea1622a8b\ig_0de50fc64c2798b60169eca6319a3081989d059972b19af350.png`
- Normalization: deterministic resample from `1672x941` to `1920x1080`
- Accepted reference: `C:\UE\T66\UI\screens\party_invite_modal\reference\party_invite_modal_reference_1920x1080.png`
- Status: accepted for first reference-generation pass

## Runtime-Owned Regions To Preserve Later

- invite status text and player names
- accept/reject labels and enabled/disabled states
- dimmed main-menu background live UI
- any future invite row/avatar/party data

## Integrator Notes

- Modal chrome is the static target; invite content remains runtime-owned.
- Full-screen reference is an offline comparison target only.
