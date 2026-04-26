# Style Reference Prompt

Generate one full-screen 1920x1080 canonical reference image for the T66 screen "gameplay_hud".

Input images:
1. Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
2. 4K detail helper: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_3840x2160_realesrgan_x4plus_anime.png`
3. Global top-bar reference sprite: `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar\topbar_global_reference_sprite_1920x140.png`
4. Preferred imagegen chrome helper sheet: `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets\mainmenu_chrome_sprite_sheet_imagegen_20260425_v1_4096.png`
5. Current target screenshot: `C:\UE\T66\UI\screens\gameplay_hud\current\current_state_1920x1080.png`

Goal:
Preserve the current target screenshot's exact layout, hierarchy, content count, slot count, spacing, control positions, and screen structure. Use the main-menu anchor, 4K helper, global top-bar sprite, and chrome helper sheet only as visual style references. Reinterpret the existing HUD chrome in the accepted V2 Chadpocalypse language: smooth dark charcoal panels, crisp uniform purple borders, rounded clean sockets, clean planar surfaces, cream text treatment, restrained gold accents, and low-grain modern polish.

Screen-specific structure:
- Keep the live 3D gameplay view dominant and unobscured.
- Preserve the top-left score/time panel position and size.
- Preserve the top-right minimap box and stage-number label position and size.
- Preserve the bottom-left hearts/idols/portrait/ability/passive HUD cluster position and size.
- Preserve the bottom-right compact inventory panel position and item-grid rhythm.

Runtime-owned content:
Treat score values, timers, stage number, minimap contents, hearts, hero portrait, ability icons, passive icon, idol slots, inventory item slots, item icons, keybinds, cooldowns, and gameplay scene as live runtime-owned content. Represent these regions clearly as sockets or placeholders, but do not treat them as final baked runtime art.

Global top-bar handling:
This is an in-run HUD target, not a frontend/menu screen. Do not add the global top bar. Use the top-bar sprite only as a style reference for clean shared chrome.

Do not make a sprite sheet, asset board, layout diagram, callout sheet, multi-screen comparison, or variant grid. Do not redesign the structure. Do not bake runtime/localizable text or live values as final runtime art. This is an offline comparison target only.

Negative style guardrails:
No graininess, cracked stone, rubble, gemstone/crystal panels, bevel-heavy fantasy frames, noisy distressed borders, screws, rivets, bolts, chipped edges, micro-detail borders, extra glow clutter, or invented content.
