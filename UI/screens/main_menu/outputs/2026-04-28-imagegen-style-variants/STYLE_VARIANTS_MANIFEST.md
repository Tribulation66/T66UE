# Main Menu Imagegen Style Variants

Source screenshot:
`C:\UE\T66\UI\screens\main_menu\current\main_menu_runtime_capture_20260428_1920x1080.png`

Contact sheet:
`C:\UE\T66\UI\screens\main_menu\outputs\2026-04-28-imagegen-style-variants\main_menu_style_variants_contact_sheet_20260428.png`

Final normalized outputs:

1. `normalized_1920x1080\main_menu_style_variant_01_cyan_tech.png`
2. `normalized_1920x1080\main_menu_style_variant_02_crimson_iron.png`
3. `normalized_1920x1080\main_menu_style_variant_03_emerald_copper.png`
4. `normalized_1920x1080\main_menu_style_variant_04_sapphire_silver.png`
5. `normalized_1920x1080\main_menu_style_variant_05_amber_brass.png`
6. `normalized_1920x1080\main_menu_style_variant_06_toxic_lime_graphite.png`
7. `normalized_1920x1080\main_menu_style_variant_07_ivory_rose_gold.png`
8. `normalized_1920x1080\main_menu_style_variant_08_molten_obsidian.png`
9. `normalized_1920x1080\main_menu_style_variant_09_monochrome_chrome.png`
10. `normalized_1920x1080\main_menu_style_variant_10_ocean_teal_coral.png`

Rejected exploratory outputs are kept with `_rejected_` filenames in this folder.

## Prompt Correction Notes

The first broad style-transfer prompt caused structural drift: new panels, extra currencies, renamed tabs/buttons, season filters, and a replaced background scene.

The working prompt shape was changed to a conservative pixel-template edit:

- Treat the original magenta screenshot as a locked pixel template.
- Make only targeted hue/material replacement of existing purple/magenta UI chrome.
- Preserve the central background, title, tagline, text, row counts, names, scores, avatars, panel positions, and button labels.
- Explicitly forbid Follow Us panels, social icons, Friends/Requests tabs, bottom party panels, extra currencies, profile modules, Season filters, Battle Pass, Store, Quests, Heroes, Play/Find Match labels, View Rewards, and new rows.
- Keep every edit inside original component bounding boxes.

## Style Recipes

- Cyan Tech: icy cyan/silver bevels, frosted dark glass, cool steel button plates.
- Crimson Iron: red-black gunmetal, crimson enamel bevels, warm smoky panels.
- Emerald Copper: oxidized copper, emerald edge glow, jade active tabs.
- Sapphire Silver: polished silver, royal blue active surfaces, cold charcoal glass.
- Amber Brass: aged brass, honey-gold active surfaces, warmer smoked charcoal.
- Toxic Lime Graphite: matte graphite, lime plasma edges, carbon-black plates.
- Ivory Rose Gold: porcelain-white and rose-gold bevels, pearl highlights.
- Molten Obsidian: black basalt, lava-orange seam highlights, ember trim.
- Monochrome Chrome: dark polished chrome, white edge highlights, graphite glass.
- Ocean Teal Coral: teal lacquer, coral rim lights, cool deep-sea charcoal.
