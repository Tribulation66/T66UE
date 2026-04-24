# Mini Title Footer Component Sheet Prompt - 2026-04-24

## Image Generation Path

- Built-in Codex `image_gen`.
- Workspace chroma-key board: `UI/screens/mini_shared/assets/imagegen_2026-04-24/title_footer_pass/mini_title_footer_chromakey_board.png`
- Local alpha board: `UI/screens/mini_shared/assets/imagegen_2026-04-24/title_footer_pass/mini_title_footer_alpha_board.png`
- Runtime slices: `SourceAssets/UI/SettingsReference/MiniScreens/Common/slices/*_heavy.png`

## Prompt

Use case: stylized-concept

Asset type: game UI sprite sheet board for deterministic slicing

Primary request: Create a clean text-free fantasy UI component sprite sheet for a dark cathedral mini-game frontend. This pass focuses on heavier, more readable title plaques, footer bars, buttons, metric chips, and summary rows that match the ornate main-menu material language.

Scene/backdrop: perfectly flat solid `#00ff00` chroma-key background across the entire canvas, one uniform color with no shadow, gradient, texture, reflections, floor plane, or lighting variation.

Composition/framing: arrange 9 separated components in a precise 3 columns x 3 rows grid with generous `#00ff00` gaps between all sprites and around the border. Keep every sprite fully inside its grid cell and centered. Row 1: wide title plaque, medium title plaque, compact title plaque. Row 2: long footer/action bar, green button plate, blue button plate. Row 3: purple button plate, summary result row plate, compact metric chip.

Style/medium: polished 2D fantasy game UI sprites, hand-painted but crisp, high contrast edges, readable silhouettes, same visual language as an ornate fantasy main menu.

Materials/textures: dark charcoal slate interiors, beveled bronze-gold metal borders, carved stone, purple crystal accents, subtle emerald enamel on the green button, cool blue enamel on the blue button, violet arcane enamel on the purple button, restrained inner vignette inside plates only.

Constraints: no text, no glyphs, no numbers, no symbols, no icons, no white placeholder bars, no baked data, no characters, no portraits, no drop shadows onto the green background, no glow bleeding into the green background. Do not use `#00ff00` anywhere in the sprites. Transparent-output preparation: background must be exactly flat `#00ff00` for local chroma-key removal.

## Local Processing

The generated board was converted to alpha using the installed Codex imagegen chroma-key helper. Deterministic grid slicing produced the promoted runtime title, footer, button, chip, and summary-row family; no manual pixel repair, painting, erasing, masking, or screenshot cleanup was performed.
