# Mini Component Sheet Transparent Pass Prompt - 2026-04-24

## Image Generation Path

- Built-in Codex `image_gen`.
- Source output: `C:\Users\DoPra\.codex\generated_images\019dbecc-187e-7a42-8f4c-86eb596004a0\ig_081b8ea9c87cc5860169eb7d9a13cc819aa7c3f2d6410b7a61.png`
- Workspace chroma-key board: `UI/screens/mini_shared/assets/imagegen_2026-04-24/transparent_pass/mini_common_components_chromakey_board.png`
- Local alpha board: `UI/screens/mini_shared/assets/imagegen_2026-04-24/transparent_pass/mini_common_components_alpha_board.png`
- Runtime slices: `SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/*_alpha.png`

## Prompt

Use case: stylized-concept

Asset type: game UI sprite sheet board for deterministic slicing

Primary request: Create a clean text-free UI component sprite sheet for a dark Chadpocalypse mini-game frontend, matching clean charcoal panels, restrained bronze-gold trim, green enamel success buttons, blue back buttons, and purple action buttons. Components must be empty plates with no letters, no numbers, no icons, no avatars, no labels, no values, no UI text.

Scene/backdrop: perfectly flat solid `#00ff00` chroma-key background across the entire canvas, one uniform color with no shadow, gradient, texture, reflections, floor plane, or lighting variation.

Composition/framing: arrange 16 separated components in a precise 4 columns x 4 rows grid with generous `#00ff00` gaps between all sprites and around the border. Keep every sprite fully inside its grid cell and centered. Row 1: wide title plaque, large rectangular content panel, medium rectangular content panel, long narrow row plaque. Row 2: compact stat chip, square portrait frame, normal character card, selected character card with brighter gold edge. Row 3: disabled/dim character card, green button plate, blue button plate, purple button plate. Row 4: idol offer row plate, idol offer row with right action socket, small slot badge frame, summary result row plate.

Style/medium: polished 2D game UI sprites, sleek modern minimalist surfaces, crisp high contrast edges, same red/black/charcoal/gold visual language as the approved main menu.

Materials/textures: dark charcoal planar interiors, satin bronze/gold metal borders, clean flat/satin enamel accents, subtle inner vignette inside panels only. Avoid grain, cracked stone, gemstone/crystal surfaces, noisy distressed panels, rubble texture, carved stone, chipped edges, micro-detail borders, and fantasy jewel ornaments.

Constraints: no text, no glyphs, no numbers, no symbols, no white placeholder bars, no baked data, no characters, no portraits, no drop shadows onto the green background, no glow bleeding into the green background. Do not use `#00ff00` anywhere in the sprites. Transparent-output preparation: background must be exactly flat `#00ff00` for local chroma-key removal.

## Local Processing

The generated board was converted to alpha using the installed Codex imagegen chroma-key helper. Deterministic connected-component slicing produced the promoted runtime sprite sheet family; no manual pixel repair, painting, erasing, masking, or screenshot cleanup was performed.
