# Hero 1 Chad Male Raw Black Outline Recipe

## Intent

Keep the original Pixal3D output as the visual target and add only a black outline around the character.

## Source

- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Source image: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Sources\Hero_1_Chad_Male.png`
- Blender file: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RawOutlineOnly.blend`

## Material

- No material slots, textures, Principled BSDF values, colors, roughness, or geometry modifiers are changed on the character meshes.
- Both left and right characters are raw duplicates of the imported GLB.
- The right character differs only in the final proof render because a black outer-silhouette overlay is composited behind/around the raw right-side render.

## Outline

- Method: pixel-exact outer silhouette overlay generated from the right character luminance mask and composited over the raw comparison render.
- Color: black `(0, 0, 0)`.
- Alpha: `1.0`.
- Outline radius: `7 px`.
- Base raw comparison render: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_raw_vs_raw_base.png`.
- Mask render: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_outline_mask.png`.
- Overlay image: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_outer_only_outline_overlay.png`.
- Final proof render: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Renders\Hero_1_Chad_Male_raw_vs_black_outline.png`.
- Edge behavior: outside silhouette only; no clothing, fold, face, or mesh-detail lines are authored.

## Lighting Rig

- Render engine: EEVEE Next when available, EEVEE fallback.
- World color: white `(1, 1, 1)`.
- View transform: `Standard`, look `Medium High Contrast`, exposure `0`, gamma `1`.
- Shadowless soft area lights:
  - `Key_Softbox_L`: location `(-5.5, -5.2, 5.0)`, energy `470`, size `6.5`.
  - `Key_Softbox_R`: location `(5.5, -5.2, 4.6)`, energy `360`, size `7.0`.
  - `Top_Soft_Fill`: location `(0, -1.5, 7.2)`, energy `230`, size `8.0`.
  - `Front_Fill`: location `(0, -7.5, 2.1)`, energy `150`, size `9.0`.

## Non-Actioned Future UE5 Port Note

No Unreal work was done in this pass. Later, the UE5 reproduction should keep the current raw character material and add an outline as a separate object-outline pass or inverted-hull material. The base material should not be edited for this target.
