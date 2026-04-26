# Challenges Layout List - V2

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\challenges\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
4K helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
Global top-bar sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
Chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`

## Top-Bar And Back Handling

- Include the shared global top bar exactly as fixed chrome at the top of the full-screen reference.
- The challenges content begins below the global top bar.
- Remove the standalone red close/back affordance from the generated reference because `challenges` is a main-menu/top-bar child screen.
- Do not replace it with a new back arrow, footer, or alternate return control.

## Fixed Structure

- One full-width challenges content surface below the top bar.
- Header/title region at top left.
- Top-right primary content controls:
  - selected `Challenges` tab/button
  - `Mods` tab/button
- The current red close button is intentionally omitted in the V2 reference.
- Secondary tab/action row under the title:
  - selected `Official` tab
  - `Community` tab
  - `Create Challenge` action button
  - small status line below the tabs
- Body split into two columns:
  - left challenge list panel
  - right selected-challenge detail panel
- Left list panel:
  - exactly two visible challenge rows
  - empty lower list space
  - each row has a small left state/icon socket, title line, reward line, category badge, and author/source line.
- Right detail panel:
  - selected challenge title
  - author/source line
  - short description line
  - `Rules And Requirements` heading
  - bordered rules text box with exactly two bullet rows
  - reward heading/value region
  - explanatory body text region
  - lower-right confirm button

## Runtime-Owned Regions

- All labels, tab text, status copy, row names, rewards, badges, author names, detail copy, rule text, reward values, and button labels are runtime-owned text.
- Challenge row sockets are runtime-owned state/icon wells.

## Explicit Bans

- Do not add sidebar navigation, weekly/expert/legacy tabs, reward cards, rank markers, extra challenge rows, Start Challenge buttons, filters, currencies, icons, or meters.
- Do not add cracked stone, gemstones, crystal panels, screws, rivets, chipped edges, rubble, distressed frames, or noisy bevel-heavy borders.
