Operator draft:

There is no contradiction between "it is an image," "it is a bitmap UI texture," and "it is a transparent PNG." A transparent PNG is one common source/runtime image file for the button plate. The plate is the art: bevel, border, fill, rounded corner, shadow, paper/card/wood texture, etc. The label text is separate and drawn on top by the UI.

"Slice" means the UI renderer treats the source image as separate regions instead of stretching the whole image as one rectangle. The artist/engine defines cut margins. Those cuts are not necessarily visible in the PNG; they are metadata/rules telling the engine which pixels are protected and which pixels may stretch.

3-slice means the image is cut along one axis into three regions. For a horizontal button or bar, that is left cap, middle, right cap. The left and right caps do not stretch; the middle stretches horizontally. It is used when height stays fixed and only width changes.

9-slice means the image is cut along both axes into a 3 by 3 grid, which creates nine regions. The four corners do not stretch. The four edges stretch only along their length. The center stretches or tiles. It is used for panels/buttons that need to grow in both width and height while preserving corners and bevels.

The numbers are simply region counts: 3 regions for a one-axis slice, 9 regions for a two-axis slice.

Example: a 200 by 80 transparent PNG button plate might have 18 pixels protected on the left, 18 on the right, 14 on top, and 16 on bottom. If the UI draws it at 420 by 80, the corners and caps stay crisp and only the middle/edges expand. If the UI draws it as a simple stretched image, all pixels scale, including the bevel and corners, which is what causes distortion.

So the foundation is: the artwork is still a bitmap image, usually transparent PNG or a texture atlas region. Slicing is the rule for how that image is allowed to scale. Live text is then layered above it.

For future FriendslopUI thinking, the important distinction is not "image or no image." It is "raw stretched full bitmap" versus "modular texture with fixed caps/corners and explicit slice margins."
