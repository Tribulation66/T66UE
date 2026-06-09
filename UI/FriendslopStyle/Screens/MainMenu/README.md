# FriendslopStyle Main Menu

This folder contains the active Main Menu implementation docs for the
FriendslopStyle lane.

## Current Files

| File | Purpose |
|---|---|
| `checklist.md` | Verifier checklist for the current Friendslop Main Menu pass. |
| `component_contract_current.md` | Current ownership, provenance, and containment contract. |
| `element_manifest.md` | Runtime widget and chrome decomposition. |
| `geometry.md` | Reference geometry table normalized for verifier use. |
| `geometry_overlay.png` | Visual overlay for the current Main Menu geometry table. |
| `production_plate_plan.md` | Current production plate plan and PPF/parity notes. |
| `round06_production_slice_specs.md` | Current Round06 production slice records. |
| `slice_specs.md` | Screen-level slice spec index. |
| `slice_artifacts/` | Main Menu slice/contact proof images moved under this screen folder. |
| `visual_scorecard_template.md` | Visual review evidence template; no Codex-owned visual Result gate. |

## Reference

Current visual reference:

`UI/FriendslopStyle/Reference/MainMenu/Current/main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

The reference is a visual target and comparison artifact. It is never a runtime
plate source.

## Archive Boundary

Previous pass contracts, inpaint/clean-sheet specs, and Round01-Round05 prompt
iterations are under `UI/FriendslopStyle/Archive/`. They are historical evidence,
not current implementation rules.

## Pass Inventory Rule

Every new Main Menu implementation pass must update `element_manifest.md` as a
full-screen ledger. Evaluate all five visual families, not only the element
discussed most recently:

- `TopBar`
- `LeftSocialPanel`
- `RightLeaderboardPanel`
- `CenterButtonStack`
- `Background`

Each family must be visual `PASS` or visual `FAIL`. Visual `PASS` means it does
not need image regeneration; visual `FAIL` means it does. For each visual `FAIL`
family, classify all elements inside that family as visual `PASS` or visual
`FAIL`, then launch one Codex CLI imagegen worker for that family. The worker
receives the cached textless family crop for the approved reference and must
generate both a family contact sheet and individualized backgroundless PNGs for
all failed elements in that family. Manual cropping from the family sheet is not
the runtime asset path. All generated elements must be implemented onto the
screen before the pass reports.

Main Menu family worker prompts must not describe the desired look in words.
See `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
Section 2.2.1 for the full Allowed/Forbidden definition. Do not use
descriptive/adjectival material, shape, color, vibe, polish, or game-comparison
language. The cached textless family crop is the visual authority; the request
can list element names, output paths, alpha/canvas requirements, and
no-baked-content rules only.

For each approved Main Menu reference, create the textless family breakdown once
through a separate local Codex CLI worker, then reuse it for future iterations
against that same reference. The five textless family crops are `TopBar`,
`LeftSocialPanel`, `RightLeaderboardPanel`, `CenterButtonStack`, and
`Background`.

Before importing individualized worker PNGs, run an alpha-bounds check against
the expected visible extent of each asset. If the plate occupies only a narrow
part of an otherwise correct-sized transparent canvas, fork a bounded local CLI
retry worker. Do not fix this by manual cropping.

After regenerated assets are implemented, run the sizing/fitting correction
section across the same five families and record what changed or blocked
correction. Then run wiring/functionality `PASS`/`FAIL` and fix failures until
they pass or are blocked.

Main Menu passes no longer report `FULL` or `PARTIAL`. The final report gives
objective process coverage, generated asset implementation paths,
sizing/fitting notes, the wiring/functionality `PASS`/`FAIL` gate, and current
reference/capture/contact evidence for user visual review. Every final evidence
packet must include both `reference_vs_current` and `previous_vs_current`
comparison sheets.

