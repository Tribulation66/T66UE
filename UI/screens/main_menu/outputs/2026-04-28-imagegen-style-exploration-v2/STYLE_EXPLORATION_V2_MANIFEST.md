# Main Menu Style Exploration V2

Source screenshot:
`C:\UE\T66\UI\screens\main_menu\current\main_menu_runtime_capture_20260428_1920x1080.png`

Contact sheet:
`C:\UE\T66\UI\screens\main_menu\outputs\2026-04-28-imagegen-style-exploration-v2\main_menu_style_exploration_v2_contact_sheet_20260428.png`

Final normalized outputs:

1. `normalized_1920x1080\main_menu_style_exploration_v2_01_cute_retro_toy_arcade.png`
2. `normalized_1920x1080\main_menu_style_exploration_v2_02_modern_brutalist_glass.png`
3. `normalized_1920x1080\main_menu_style_exploration_v2_03_art_nouveau_fantasy.png`
4. `normalized_1920x1080\main_menu_style_exploration_v2_04_cyberpunk_holographic_neon.png`
5. `normalized_1920x1080\main_menu_style_exploration_v2_05_illuminated_manuscript.png`
6. `normalized_1920x1080\main_menu_style_exploration_v2_06_organic_biomechanical_alien.png`
7. `normalized_1920x1080\main_menu_style_exploration_v2_07_bauhaus_memphis_geometric.png`
8. `normalized_1920x1080\main_menu_style_exploration_v2_08_gothic_stained_glass.png`
9. `normalized_1920x1080\main_menu_style_exploration_v2_09_papercraft_collage.png`
10. `normalized_1920x1080\main_menu_style_exploration_v2_10_y2k_chrome_bubble_futurism.png`

Rejected exploratory outputs are kept with `_rejected_` filenames in this folder.

## Prompt Correction Notes

This set intentionally loosened the previous constraints:

- Font style may change.
- Title lettering style may change.
- Background art treatment may change.
- Panel geometry, button geometry, icon style, border language, color palette, and material rendering may change strongly.

The only hard locks are:

- Content inventory: same visible text, same control count, same menu item meanings, same panel inventory.
- Placement: same major zones and approximate bounding boxes.

The first loose prompt still generated extra generic UI content, including social/leaderboard additions. The corrected prompt added exact control counts and explicit forbidden content:

- Right panel has exactly two tabs: `WEEKLY` and `ALL TIME`.
- No standalone `SOCIAL` label.
- No `FRIENDS` tab.
- No `SEASON`, `BATTLE PASS`, `STORE`, `QUESTS`, `HEROES`, `HOME`, `PLAY`, `FIND MATCH`, `VIEW REWARDS`, `FOLLOW US`, extra currency, gem counter, bottom party bar, or new leaderboard names.

## Style Recipes

- Cute Retro Toy-Arcade: rounded chunky plastic panels, candy cyan, strawberry red, banana yellow, mint, warm cream, bubble tabs, sticker shadows, toy-diorama idol scene.
- Modern Brutalist Glass: hard chamfered panes, smoky glass, off-white grid lines, cobalt and safety orange, concrete texture, Swiss-modern type, cinematic low-poly idol.
- Art Nouveau Fantasy: flowing vine frames, gold filigree, emerald enamel, parchment-dark panels, jewel tabs, hand-lettered fantasy type, celestial garden-temple idol.
- Cyberpunk Holographic Neon: translucent panels, scanlines, RGB split glow, hex borders, circuit traces, neon brackets, holographic idol projection.
- Illuminated Manuscript: parchment, gilded borders, blackletter-inspired type, red/blue/gold flourishes, embossed leather, codex-style idol illustration.
- Organic Biomechanical Alien: membrane panels, chitin borders, bioluminescent veins, cellular buttons, alien glyphs, living energy halo.
- Bauhaus Memphis Geometric: bold circles, triangles, stripes, flat colors, offset blocks, primary blue, yellow, red, mint, high-contrast poster idol.
- Gothic Stained Glass: pointed arches, black iron tracery, ruby/sapphire glass, rose-window motifs, candlelit gold, cathedral idol scene.
- Papercraft Collage: layered paper cutouts, torn cardboard edges, stickers, risograph texture, folded tabs, cut-paper idol diorama.
- Y2K Chrome Bubble Futurism: liquid chrome frames, aqua glass, iridescent gradients, pill buttons, pearl highlights, glossy plasma idol stage.
