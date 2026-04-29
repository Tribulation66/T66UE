# CHADPOCALYPSE Main Menu - 80s Arcade Retro Variants

Date: 2026-04-28

Source screenshot:

- `C:\UE\T66\UI\screens\main_menu\current\main_menu_runtime_capture_20260428_1920x1080.png`

Output folder:

- `C:\UE\T66\UI\screens\main_menu\outputs\2026-04-28-imagegen-80s-arcade-retro`

Contact sheet:

- `C:\UE\T66\UI\screens\main_menu\outputs\2026-04-28-imagegen-80s-arcade-retro\main_menu_80s_arcade_contact_sheet_20260428.png`

Normalized 1920x1080 outputs:

- `normalized_1920x1080\main_menu_80s_arcade_01_maze_cabinet_neon.png`
- `normalized_1920x1080\main_menu_80s_arcade_02_platform_girder_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_03_space_shooter_high_score.png`
- `normalized_1920x1080\main_menu_80s_arcade_04_vector_grid_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_05_neon_racing_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_06_williams_chaos_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_07_cute_bubble_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_08_dungeon_fantasy_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_09_urban_brawler_cabinet.png`
- `normalized_1920x1080\main_menu_80s_arcade_10_amber_vector_monitor.png`

## Prompt Lock

Each image used the original screenshot as the strict layout and content wireframe. The prompt explicitly locked the following:

- Top navigation order: gear icon, language icon, ACCOUNT, center portrait badge, POWER UP, ACHIEVEMENTS, MINIGAMES, one ticket counter reading 10, red quit icon.
- Left social panel content: Solobr'o profile, level/progress content, search field, ONLINE and OFFLINE groups, visible friend rows, and PARTY slots.
- Center content: CHADPOCALYPSE title, subtitle, golden idol composition, NEW GAME and LOAD GAME buttons.
- Right panel content: GLOBAL LEADERBOARD, WEEKLY and ALL TIME tabs, Solo and Easy dropdowns, High Score and Speed Run options, rank/name/score table with the same visible rows.
- Spatial placement: top bar stays top, left panel stays left, center title/art/buttons stay center, right leaderboard stays right.
- Avoid list: no extra tabs, no extra currency, no new store/quest/social/follow panels, no added bottom bar, no new readable text outside source content, no copied game logos, characters, sprites, or exact UI elements.

## Style Recipes

1. `main_menu_80s_arcade_01_maze_cabinet_neon.png`
   - Reference: Pac-Man-era maze arcade cabinet.
   - Direction: black enamel, neon yellow maze-line trim, cyan and hot-pink accents, dotted border lights, CRT scanline glow.

2. `main_menu_80s_arcade_02_platform_girder_cabinet.png`
   - Reference: Donkey Kong-era platform arcade cabinet.
   - Direction: red-orange girder frames, cobalt shadows, rivet pixels, diagonal ladder-like trim, saturated marquee glow.

3. `main_menu_80s_arcade_03_space_shooter_high_score.png`
   - Reference: Galaga and Galaxian-era space shooter cabinets.
   - Direction: deep starfield, cyan/magenta/yellow phosphor glow, formation-grid dots, compact high-score typography.

4. `main_menu_80s_arcade_04_vector_grid_cabinet.png`
   - Reference: Tron and Tempest-era vector cabinets.
   - Direction: black-glass CRT, cyan/magenta/amber wireframe outlines, angular grid geometry, heavy vector bloom.

5. `main_menu_80s_arcade_05_neon_racing_cabinet.png`
   - Reference: Out Run and Super Hang-On-era racing cabinets.
   - Direction: magenta-orange sunset glow, teal chrome borders, checker/grid motifs, slanted aerodynamic button cuts.

6. `main_menu_80s_arcade_06_williams_chaos_cabinet.png`
   - Reference: Defender and Robotron 2084-era Williams cabinets.
   - Direction: black CRT field, neon warning panels, intense score-table energy, bright cabinet stripes, phosphor bloom.

7. `main_menu_80s_arcade_07_cute_bubble_cabinet.png`
   - Reference: Bubble Bobble-era cute Japanese arcade cabinets.
   - Direction: candy cyan, bubblegum pink, lemon yellow, rounded bubble-like panels, playful bitmap typography.

8. `main_menu_80s_arcade_08_dungeon_fantasy_cabinet.png`
   - Reference: Gauntlet-era fantasy dungeon cabinets.
   - Direction: dark stone panels, torch-glow edges, bronze trim, emerald/sapphire player-color accents, CRT scanlines.

9. `main_menu_80s_arcade_09_urban_brawler_cabinet.png`
   - Reference: Double Dragon and Bad Dudes-era urban brawler cabinets.
   - Direction: brick-and-neon surfaces, red/electric-blue side stripes, chrome bevels, diagonal slash borders, concrete texture.

10. `main_menu_80s_arcade_10_amber_vector_monitor.png`
    - Reference: Asteroids and Battlezone-era monochrome/vector arcade monitors.
    - Direction: amber-on-black CRT phosphor, thin vector outlines, simple wireframe geometry, barrel distortion, sparse cabinet glow.

## Iteration Notes

- A dungeon-fantasy attempt added a new bottom decorative cabinet strip, so it was rejected and regenerated with an explicit no-footer/no-bottom-strip constraint.
- An urban-brawler attempt added extra graffiti wording in the center background, so it was rejected and regenerated with an explicit no-background-lettering/no-graffiti-text constraint.
- Game references are used only for broad period, palette, geometry, CRT, cabinet, and panel-treatment language. The prompts explicitly avoid copied logos, characters, exact sprites, exact cabinet art, and exact UI elements.
