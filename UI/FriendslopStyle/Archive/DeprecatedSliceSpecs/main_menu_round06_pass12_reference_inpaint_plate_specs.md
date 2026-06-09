# Main Menu Round06 Pass12 Reference-Crop Plate Specs

Source script: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_generate_reference_inpaint_plates.py`

Plate contact sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_reference_inpaint_plate_contact_sheet.png`

Reference comparison sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_reference_inpaint_plate_comparison.png`

Pass12 supersedes pass11. Pass11 proved the fixed-size surface strategy but
used deterministic generic blank plates that were too detached from Round06
material. Pass12 regenerates the runtime chrome from the corresponding Round06
reference regions, removes baked labels/player data with per-element masks, and
keeps fixed-size image drawing to avoid split centers and 9-slice damage.

Runtime copies are written to:

- `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`
- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`

Source copies are written to:

- `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\`

## Reference-Derived Fixed Plates

| Asset | Size | Draw mode | Source treatment |
|---|---:|---|---|
| `topbar_strip_round06.png` | `1888x100` | fixed image | Round06 crop, live controls removed. |
| `topbar_icon_dark_round06.png` | `96x74` | fixed image | Round06 crop, center icon area blanked for live glyph. |
| `topbar_tab_dark_round06.png` | `292x74` | fixed image | Round06 crop, label blanked. |
| `topbar_tab_red_round06.png` | `325x80` | fixed image | Round06 crop, label blanked. |
| `topbar_ticket_round06.png` | `172x74` | fixed image | Round06 crop, ticket/count area blanked for live overlay. |
| `topbar_power_red_round06.png` | `96x74` | fixed image | Round06 crop, center glyph area blanked. |
| `left_panel_round06.png` | `500x892` | fixed image | Round06 crop, child content cleared into an empty shell. |
| `profile_row_round06.png` | `460x108` | fixed image | Round06 crop, avatar/name/progress/level areas blanked. |
| `search_field_round06.png` | `460x60` | fixed image | Round06 crop, placeholder area blanked. |
| `section_header_round06.png` | `460x42` | fixed image | Round06 crop, label/count area blanked. |
| `friend_row_round06.png` | `460x58` | fixed image | Round06 crop, all row data/action content blanked. |
| `invite_button_green_round06.png` | `80x44` | fixed image | Round06 crop, label blanked. |
| `offline_button_dark_round06.png` | `80x42` | fixed image | Round06 crop, label blanked. |
| `party_slot_round06.png` | `94x94` | fixed image | Round06 crop, center plus/avatar area blanked. |
| `cta_primary_round06.png` | `680x104` | fixed image | Round06 crop, live label area blanked. |
| `cta_secondary_round06.png` | `660x94` | fixed image | Round06 crop, live label area blanked. |
| `filter_icon_red_round06.png` | `76x70` | fixed image | Round06 crop, center glyph area blanked. |
| `filter_icon_dark_round06.png` | `76x70` | fixed image | Round06 crop, center glyph area blanked. |
| `leaderboard_panel_round06.png` | `422x884` | fixed image | Round06 crop, child content cleared into an empty shell. |
| `leaderboard_tab_red_round06.png` | `190x52` | fixed image | Round06 crop, label blanked. |
| `leaderboard_tab_dark_round06.png` | `190x52` | fixed image | Round06 crop, label blanked. |
| `dropdown_dark_round06.png` | `190x52` | fixed image | Round06 crop, label/arrow area blanked. |
| `checkbox_checked_round06.png` | `28x28` | fixed image | Round06 checked state crop. |
| `checkbox_empty_round06.png` | `28x28` | fixed image | Round06 empty state crop. |
| `table_header_band_round06.png` | `390x26` | fixed image | Round06 crop, live header text blanked while retaining red divider. |
| `ranking_row_red_round06.png` | `406x46` | fixed image | Round06 crop, rank/avatar/name/score areas blanked. |

## Validation

- Fixture capture:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_capture.png`
- Fixture dump:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_dump_utf8.json`
- Fixture contact sheet:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_contact_sheet.png`
- Material crop sheet:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_crop_sheet.png`
- Material verdict:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_verdict.md`
- Fixture visual scorecard:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_visual_scorecard.md`
- Fixture verifier:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_fidelity.md`
- Pass12 verifier result: `PASS=251 FAIL=0 UNSURE=0`.
