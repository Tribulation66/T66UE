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
- Preserve the exact inventory-panel size and the open gameplay area above it.
- Replace placeholder chrome with the Chadpocalypse main-menu visual language: dark metal slabs, purple neon bevels, pixel-art highlights, readable socket frames, gold/red/green currency sockets, and restrained glow that does not cover gameplay.
