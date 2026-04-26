Drop future minimap marker icons in this folder as individual PNG files.

The HUD now attempts to load these loose files first and falls back to the old atlas if a file is missing.

Current supported filenames:
- npc.png
- vendor.png
- support_vendor.png
- gambler.png
- saint.png
- ouroboros.png
- collector.png
- trickster.png
- gate.png
- miasma.png
- chest.png
- crate.png
- loot_bag.png
- catch_up_loot.png

Guidelines:
- Transparent PNG only.
- Keep the silhouette bold and readable at small size.
- Avoid text, borders, or background plates.
- Default target size is about 20x20 to 22x18 in the minimap widget.
- Use a centered composition with enough padding so icons do not touch the edges.
- Current files are temporary in-repo icons intended to be replaced later by generated final art.

Integration note:
- Not every marker type is emitted by the HUD yet.
- This folder and loader exist now so the next icon pass can be asset-first instead of another atlas refactor.
