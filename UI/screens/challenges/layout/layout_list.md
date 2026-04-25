# Challenges Layout List

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\challenges\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

## Fixed Structure

- Full-screen dimmed main-menu background remains visible only as a dark backdrop behind the modal.
- One large centered modal shell, inset from all canvas edges, with a clean dark frame, crisp border, and restrained metallic accents.
- Header/title region inside the modal at top left.
- Top-right primary modal controls:
  - selected `Challenges` tab/button
  - `Mods` tab/button
  - red close button
- Secondary tab/action row under the title:
  - selected `Official` tab
  - `Community` tab
  - `Create Challenge` action button
  - small status line below the tabs
- Body split into two columns:
  - left challenge list panel
  - right selected-challenge detail panel
- Left list panel:
  - selected first challenge row
  - second challenge row
  - empty lower list space
  - each row has a small left socket/icon/checkbox area, title line, reward line, small category badge, and author/source line.
- Right detail panel:
  - selected challenge title at upper left
  - author/source line
  - short description line
  - `Rules And Requirements` heading
  - bordered rules text box with two bullet rows
  - reward heading/value region
  - explanatory body text region
  - lower-right confirm button

## Runtime-Owned Regions

- All title labels, tab labels, status copy, row names, rewards, category badges, author names, detail copy, rule text, reward values, and button labels are runtime-owned text.
- Challenge row icon/checkbox sockets are runtime-owned state/icon wells.
- Preserve these as readable placeholders and live-content wells in the reference; do not treat them as final baked art.

## Explicit Bans

- Do not add sidebar navigation, weekly/expert/legacy tabs, reward cards, rank markers, extra challenge rows beyond the visible current layout, or a `Start Challenge` button.
- Do not change the two-column modal hierarchy.
