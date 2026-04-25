# Pause Menu Layout List

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\pause_menu\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

## Fixed Structure

- Full-screen dimmed main-menu/game frontend remains visible as the background only.
- One centered vertical pause modal with a clean dark frame, crisp border, and restrained metallic accents.
- Large title `Paused` centered near the top of the modal.
- Exactly five stacked full-width menu buttons inside the modal:
  - Resume Game, highlighted/selected green
  - Save And Quit
  - Restart, destructive/red state
  - Settings
  - Achievements

## Runtime-Owned Regions

- Modal title and all button labels are runtime-owned text.
- Button state colors and plates are generated shell/control art, but labels stay live later.

## Explicit Bans

- Do not add close buttons, tabs, sidebars, extra menu options, icons, inventory slots, currency, or confirmation panels.
- Do not change the centered single-column modal hierarchy.
