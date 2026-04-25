# Style Reference Prompt

Generate one full-screen 1920x1080 canonical reference image for the T66 screen "gameplay_hudinventory_inspect".

Input images:
1. Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
2. Current target screenshot: `C:\UE\T66\UI\screens\gameplay_hudinventory_inspect\current\current_state_1920x1080.png`

Goal:
Preserve the current target screenshot's layout, major regions, spacing, control positions, and screen hierarchy. Reinterpret the visual design in the same Chadpocalypse UI language as the main-menu anchor: dark fantasy metal frames, purple neon bevels, pixel-art/painted game UI treatment, punchy readable panels, sharp stateful controls, and high-contrast arcade-fantasy styling.

Screen-specific structure:
- Keep the live 3D gameplay view dominant and unobscured above the inspect panel.
- Preserve the top-left score/time panel position and size.
- Preserve the top-right minimap box and stage-number label position and size.
- Preserve the bottom-left hearts/idols/portrait/ability/passive HUD cluster position and size.
- Preserve the large bottom-right inventory inspect panel bounds, header row, currency counter positions, and two-row item grid.

Runtime-owned content:
Treat score values, timers, stage number, minimap contents, hearts, hero portrait, ability icons, passive icon, idol slots, inventory item slots, item icons, keybinds, cooldowns, currency values, and gameplay scene as live runtime-owned content. Represent these regions clearly as sockets or placeholders, but do not treat them as final baked runtime art.

Do not make a sprite sheet, asset board, layout diagram, callout sheet, multi-screen comparison, or variant grid. Do not redesign the structure. Do not bake runtime/localizable text or live values as final runtime art. This is an offline comparison target only.
