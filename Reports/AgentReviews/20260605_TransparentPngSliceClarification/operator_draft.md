Operator draft:

"Transparent PNG" does not mean the whole image is invisible. It means the image has an alpha channel, so every pixel can have its own opacity.

For a button plate, most of the pixels are visible: the fill, border, bevel, shadow, scratches, paper/wood/plastic surface, etc. Some pixels are transparent, usually outside the rounded/irregular shape or in padding around it. The visible button is the opaque or semi-opaque part of the PNG. The transparent parts just let the game/menu background show through.

So nothing needs to sit on top of the transparent area to make the plate visible. The PNG itself contains both visible colored pixels and invisible alpha pixels.

9-slice is not a new image format and it is not usually a destructive conversion. The PNG stays a PNG. The UI system stores slice rules next to it: left margin, right margin, top margin, bottom margin. Those margins tell the renderer which parts are fixed and which parts may stretch.

The practical mental model is:

1. Create or author a blank button/panel plate image.
2. Export it as PNG with alpha.
3. Import/use it as a UI texture.
4. Assign 3-slice or 9-slice margins in the engine/style/brush settings.
5. Draw live text and icons on top in the UI.

So "use imagegen to generate PNGs, then convert them to 9-slice" is close, but the exact wording should be "generate or author blank UI plates as PNGs, then configure those PNGs as sliced UI brushes." The slicing is metadata/settings, not a magical cleanup step. If the generated plate has messy corners, baked highlights in the wrong place, distorted borders, or text baked in, 9-slice will not fix that; it only protects correctly authored regions during scaling.

For T66, this is just conceptual for now. The current flat pipeline does not allow generated raster chrome, so adopting imagegen PNG plates for FriendslopUI would need to be an explicit new alternative style contract rather than an accidental continuation of the old imagegen chrome approach.
