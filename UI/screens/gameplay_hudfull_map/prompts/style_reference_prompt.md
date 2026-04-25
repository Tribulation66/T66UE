# Style Reference Prompt

Generate one full-screen 1920x1080 canonical reference image for the T66 screen "gameplay_hudfull_map".

Input images:
1. Main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
2. Current target screenshot: `C:\UE\T66\UI\screens\gameplay_hudfull_map\current\current_state_1920x1080.png`

Goal:
Preserve the current target screenshot's layout, major regions, spacing, control positions, modal proportions, and screen hierarchy. Reinterpret the visual design in the same Chadpocalypse UI language as the main-menu anchor: dark fantasy metal frames, purple neon bevels, pixel-art/painted game UI treatment, punchy readable panels, sharp stateful controls, and high-contrast arcade-fantasy styling.

Screen-specific structure:
- Keep the gameplay HUD dimmed behind the map overlay.
- Preserve the large centered map modal bounds, header placement, title area, close affordance area, and giant map well.
- Preserve the central player-marker/map-content region as a live-data well.
- Keep the underlying HUD visible only as muted context.

Runtime-owned content:
Treat map contents, route/fog data, player marker, close keybind/label, underlying HUD values, minimap content, stage label, and gameplay scene as live runtime-owned content. Represent these regions clearly as sockets or placeholders, but do not treat them as final baked runtime art.

Do not make a sprite sheet, asset board, layout diagram, callout sheet, multi-screen comparison, or variant grid. Do not redesign the structure. Do not bake runtime/localizable text or live values as final runtime art. This is an offline comparison target only.
