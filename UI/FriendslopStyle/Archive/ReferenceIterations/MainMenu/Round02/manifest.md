# FriendslopStyle Main Menu Reference Round 02

Generated: 2026-06-05

Purpose: regenerated FriendslopStyle Main Menu references that preserve the current T66 Main Menu layout and visible content roles. These are design references only, not runtime UI assets.

## Layout Basis

- Baseline capture copied from `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`.
- Local layout reference: `layout_reference_current_main_menu.png`.
- Structural source: `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`.
- Checklist source: `C:\UE\T66\UI\Checklists\main_menu_checklist.md`.

## Title Finding

The center title is live Slate text, not part of the background image.

Evidence:

- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp:470` builds the title with `FT66FlatStyle::MakeFlatLabel`.
- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp:471` uses `NSLOCTEXT(..., "TRIBULATION 66")`.
- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp:1896` loads the background image separately.

For Round02 mockups, the center title region was changed to `Chadpocalypse`.

## Shared Prompt Contract

- Preserve the current baseline screen composition:
  - Top navigation bar.
  - Left profile/friends/party panel.
  - Center title/subtitle/three-CTA stack.
  - Right leaderboard/filter panel.
- Preserve visible content roles:
  - `ACCOUNT`, `POWER UP`, `ACHIEVEMENTS`, `MINIGAMES`, ticket value `10`.
  - `Local Player`, `Level 1/100`, `Search friends...`, `ONLINE (0)`, `OFFLINE (0)`, `PARTY`.
  - `Chadpocalypse`, `If you're not Chad it's over`.
  - `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`.
  - `GLOBAL CHAD RANKING`, `WEEKLY`, `ALL TIME`, `Solo`, `Easy`, `High Score`, `Speed Run`, leaderboard rows.
- Vary only FriendslopStyle UI-element language.
- Do not use source-game logos, exact typography, mascots, signature layouts, copied palettes, or theme transfer.
- Outputs are full-screen references only, not runtime UI assets.

## Outputs

| # | File | Style Vocabulary | Inspection Notes |
|---|---|---|---|
| 01 | `main_menu_reference_01_peak_cozy_chunky_layout.png` | cozy chunky outdoor co-op, rounded hand-cut plates, thick friendly outlines | Preserves all major regions. Strong warm chunky plate language. Slightly more outdoor/camp mood than strict current background. |
| 02 | `main_menu_reference_02_schedule_scrappy_utility_layout.png` | scrappy utility, laminated/worn plates, taped/scuffed panel edges | Preserves layout well. Strong physical UI-plate system and readable hierarchy. |
| 03 | `main_menu_reference_03_lethal_lofi_terminal_layout.png` | lo-fi terminal/control panel, green monitor texture, dense readable labels | Strongest exact layout preservation and cleanest contrast from FlatStyle. Very implementation-minded. |
| 04 | `main_menu_reference_04_gwyf_social_game_night_layout.png` | social game-night, colorful sticker plates, playful readable controls | Preserves layout and contents well. Most playful/social, with some decorative sticker energy. |
| 05 | `main_menu_reference_05_repo_weird_toy_utility_layout.png` | weird toy utility, chunky rubber/plastic panels, goofy device controls | Preserves layout well. Strong candidate for FriendslopStyle element language. |

## Next Suggested Step

Rank the five by:

1. Layout fidelity to current Main Menu.
2. Friendslop atmosphere.
3. Reusable UI element language.
4. Readability.
5. Implementation risk.

Then generate one hybrid `Chadpocalypse` FriendslopStyle direction from the best traits before writing process docs or runtime element specs.
