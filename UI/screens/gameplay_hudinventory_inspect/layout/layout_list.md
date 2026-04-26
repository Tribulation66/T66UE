# gameplay_hudinventory_inspect Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-screen live gameplay scene remains visible.
- Top-left compact score/time panel at approximately x=18, y=19, w=193, h=130.
- Top-right minimap panel at approximately x=1668, y=19, w=234, h=233, with stage-number label below at approximately x=1668, y=266, w=234, h=45.
- Bottom-left HUD cluster at approximately x=18, y=826, w=380, h=237.
- Large inventory inspect panel anchored from approximately x=544, y=661 to the bottom-right edge, w=1358, h=418.
- Inventory inspect panel contains a header row with title and currency counters, then two rows of large item sockets.

Runtime-owned regions:
- Gameplay scene, player character, crosshair, level geometry, minimap content, stage text, score/time values, health/currency values, hero portrait, ability icons, passive icon, inventory item sockets and item icons.

Style target:
- Preserve the exact inventory-panel size, header/currency positions, two-row slot count, and the open gameplay area above it.
- Replace placeholder chrome with the accepted V2 main-menu language: sleek dark charcoal panels, smooth satin surfaces, crisp uniform purple outlines, cream text treatment, restrained gold accents only where useful, and clean readable socket frames.
- Do not add inventory slots, ability slots, idol slots, currencies, meters, panels, icons, rows, labels, or controls beyond what is visible in the current screenshot.
- Avoid grain, cracked stone, rubble, gem/crystal surfaces, distressed metal, chipped edges, screws, rivets, bolts, heavy bevel stacks, noisy borders, and extra glow clutter.
