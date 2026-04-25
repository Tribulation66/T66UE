# Run Summary Layout List

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\run_summary\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

## Fixed Structure

- Full-screen framed summary surface with a thick outer border and right-side vertical scrollbar.
- Top-left page title `Run Summary`.
- Summary stat line below the title containing stage reached, score, and time.
- Top-right `Event Log` button.
- Two wide horizontal rank panels in the upper half:
  - weekly rank panel
  - all-time rank panel
  - each panel has score and speed-run value regions.
- Left lower column:
  - seed luck panel with value
  - integrity panel with status text
  - `Go Again!` button
  - `Main Menu` button
- Center column:
  - runtime 3D preview viewport
  - four selected-buff slots below the preview
  - bottom row of item/inventory/reward slots across the lower center
- Right column:
  - stats panel with a vertical list of player stat rows
  - damage-by-source panel with table header and ranked rows

## Runtime-Owned Regions

- All titles, stat names, values, rank values, time, score, stage, button labels, integrity text, table labels, and table rows are runtime-owned text/values.
- Center character/scene preview is a runtime 3D preview/open aperture.
- Buff/item slots are runtime-owned icon/image wells.
- Keep preview and slot interiors readable as placeholders, but do not bake final player model, icons, stat rows, or table values into production art.

## Explicit Bans

- Do not add tabs, powerup cards, challenge controls, leaderboard filters, or extra CTAs.
- Do not remove the 3D preview aperture, lower item slots, or the two rank panels.
