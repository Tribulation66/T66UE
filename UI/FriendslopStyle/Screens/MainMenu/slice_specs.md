# FriendslopStyle Main Menu Slice Specs

Reference visual: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Source sheet: `C:\UE\T66\SourceAssets\UI\FriendslopStyle\MainMenu\friendslop_mainmenu_runtime_chrome_sheet_alpha.png`

Runtime root: `C:\UE\T66\RuntimeDependencies\T66\UI\FriendslopStyle\MainMenu\`

Contact sheet: `C:\UE\T66\UI\FriendslopStyle\Screens\MainMenu\slice_artifacts\main_menu_asset_contact_sheet.png`

Status note: this slice spec records the first Main Menu pilot asset set. It is
not a final acceptance record after the 2026-06-05 process correction. Future
FriendslopStyle passes must re-test each plate against the corresponding
reference region and may need per-element/per-size plates instead of these
generic reusable slices.

## Runtime Assets

| Asset | Source Size | Slate DrawAs | Margin | Intended Runtime Use | Notes |
|---|---:|---|---:|---|---|
| `panel_large_dark.png` | `662x464` | `Box` | `0.20,0.22,0.20,0.22` | Left panel, leaderboard panel, top bar strip | Large rubber panel with preserved corner bevels and middle stretch. |
| `button_long_dark.png` | `666x133` | `Box` | `0.22,0.30,0.22,0.30` | Default CTA and navigation buttons | Primary reusable horizontal button body. |
| `button_primary_red.png` | `667x145` | `Box` | `0.22,0.31,0.22,0.31` | Selected/primary buttons | Red selected state for main CTA and selected toggles. |
| `button_action_green.png` | `667x145` | `Box` | `0.22,0.31,0.22,0.31` | Positive action buttons | Recolored from the clean red source to avoid chroma-key spill from the generated green button. |
| `pill_dark.png` | `231x113` | `Box` | `0.28,0.32,0.28,0.32` | Search fields, compact pills, badges | Keep height above 44 px when possible. |
| `row_dark.png` | `858x100` | `Box` | `0.12,0.30,0.12,0.30` | Friend rows and leaderboard rows | Wide row source for stable horizontal stretching. |
| `row_selected_red.png` | `858x105` | `Box` | `0.12,0.30,0.12,0.30` | Local/selected leaderboard rows | Red row variant. |
| `party_slot_dark.png` | `276x236` | `Box` | `0.24,0.24,0.24,0.24` | Party slot / portrait slot frame | Use at square-ish sizes. |
| `icon_button_dark.png` | `157x147` | `Image` | `0,0,0,0` | Small icon buttons only | Do not 9-slice; contact sheet shows distortion at small sizes. |
| `small_square_dark.png` | `169x167` | `Image` | `0,0,0,0` | Fixed square placeholders | Do not 9-slice. |
| `checkbox_checked_red.png` | `149x140` | `Image` | `0,0,0,0` | Checked metric boxes | Fixed image; scaling only. |
| `checkbox_empty_dark.png` | `153x141` | `Image` | `0,0,0,0` | Empty metric boxes | Fixed image; scaling only. |

## Slice Decision

Use `DrawAs=Box` only for components whose center area is intentionally flat enough to stretch. Use `DrawAs=Image` for compact square controls where the whole silhouette is load-bearing. The first contact sheet proved that forcing 9-slice behavior onto the small square/icon/checkbox family destroys the rubber silhouette, so those assets are fixed-image carriers.

## Anti-Lookalike Gate

Cheapest wrong result: generic rubber-looking controls in approximately the
right locations, or flat colored Slate rectangles with the new red/green colors
but no reference-matching bevel, shadow padding, glossy edge, or scale
relationship.

Discriminator: every major panel/button/row surface must resolve to a runtime
Friendslop PNG plate that matches its approved reference region at runtime size.
The final capture must show reference-matching inflated rubber edges at the
panel, CTA, friend-list, leaderboard, and top-bar surfaces, not just changed
colors or a stretched generic source.

