# Main Menu Round06 Pass11 Fixed-Plate Specs

Source script: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass11_generate_fixed_plates.py`

Plate contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass11_fixed_plate_contact_sheet.png`

Reference comparison sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass11_reference_plate_comparison.png`

These plates supersede the pass10 clean-sheet composited plates for the
load-bearing Round06 Main Menu surfaces. They are fixed-size blank PNG plates
authored to avoid vertical slice seams and pipe-frame side-panel artifacts.
Slate still owns all labels, player data, scores, counts, icons, handlers, and
states.

Runtime copies are written to:

- `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`
- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`

Source copies are written to:

- `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\`

## Fixed-Plate Assets

| Asset | Size | Draw mode |
|---|---:|---|
| `topbar_strip_round06.png` | `1888x100` | fixed image |
| `topbar_icon_dark_round06.png` | `96x74` | fixed image |
| `topbar_tab_dark_round06.png` | `292x74` | fixed image |
| `topbar_tab_red_round06.png` | `325x80` | fixed image |
| `topbar_ticket_round06.png` | `172x74` | fixed image |
| `topbar_power_red_round06.png` | `96x74` | fixed image |
| `left_panel_round06.png` | `500x892` | fixed image |
| `profile_row_round06.png` | `460x108` | fixed image |
| `search_field_round06.png` | `460x60` | fixed image |
| `section_header_round06.png` | `460x42` | fixed image |
| `friend_row_round06.png` | `460x58` | fixed image |
| `invite_button_green_round06.png` | `80x44` | fixed image |
| `offline_button_dark_round06.png` | `80x42` | fixed image |
| `party_slot_round06.png` | `94x94` | fixed image |
| `cta_primary_round06.png` | `680x104` | fixed image |
| `cta_secondary_round06.png` | `660x94` | fixed image |
| `filter_icon_red_round06.png` | `76x70` | fixed image |
| `filter_icon_dark_round06.png` | `76x70` | fixed image |
| `leaderboard_panel_round06.png` | `422x884` | fixed image |
| `leaderboard_tab_red_round06.png` | `190x52` | fixed image |
| `leaderboard_tab_dark_round06.png` | `190x52` | fixed image |
| `dropdown_dark_round06.png` | `190x52` | fixed image |
| `checkbox_checked_round06.png` | `28x28` | fixed image |
| `checkbox_empty_round06.png` | `28x28` | fixed image |
| `table_header_band_round06.png` | `390x26` | fixed image |
| `ranking_row_red_round06.png` | `406x46` | fixed image |

## Validation

- The generated plates are continuous fixed surfaces with no vertical slice
  reconstruction.
- The 1920x1080 fixture capture is
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass11_fixture_capture.png`.
- The verifier report is
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass11_fixture_fidelity.md`.
- Pass11 verifier result: `PASS=251 FAIL=0 UNSURE=0`.
