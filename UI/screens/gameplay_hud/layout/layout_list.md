# gameplay_hud Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-screen live gameplay scene remains the visible background and must not be obscured by decorative UI.
- Top-left compact score/time panel at approximately x=18, y=19, w=193, h=130.
- Top-right minimap panel at approximately x=1668, y=19, w=234, h=233, with a separate stage-number label below at approximately x=1668, y=266, w=234, h=45.
- Bottom-left HUD cluster at approximately x=18, y=826, w=380, h=237, containing idols grid, hearts, hero portrait, ability slots, keybind tags, and passive slot.
- Bottom-right inventory panel at approximately x=1223, y=871, w=678, h=208, containing header currency values and a two-row item grid.

Runtime-owned regions:
- Gameplay scene, player character, crosshair, floor/level geometry, and minimap content.
- Score, score-to-beat, timer, time-to-beat, stage number, health values, currency values, item counts, keybind labels, cooldown values, hero portrait, ability icons, passive icon, idol slots, inventory item slots.

Style target:
- Preserve the exact HUD locations, compact information density, slot counts, and open gameplay visibility.
- Replace flat gray placeholder chrome with the accepted V2 main-menu language: sleek dark charcoal panels, smooth satin surfaces, crisp uniform purple outlines, cream text treatment, restrained gold accents only where useful, and clean readable socket frames.
- Do not add ability slots, idol slots, currencies, meters, panels, icons, rows, labels, or controls beyond what is visible in the current screenshot.
- Avoid grain, cracked stone, rubble, gem/crystal surfaces, distressed metal, chipped edges, screws, rivets, bolts, heavy bevel stacks, noisy borders, and extra glow clutter.
