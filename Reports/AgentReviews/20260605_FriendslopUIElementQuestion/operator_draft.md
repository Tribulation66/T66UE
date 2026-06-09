Working task:
Operator: Codex
Validator: Claude
Scope: Answer the UI-production question for the planned Friendslop UI direction: what a game button background usually is, what formats/workflows are used, how those elements scale, and why the prior imagegen-chrome approach distorted. No repo edits.
Stop condition: Give a clear technical answer grounded in current T66 UI process context, with Claude review and token reporting.

Operator draft:

The thing under the PLAY text in a normal game UI is usually a runtime UI brush: live text rendered on top of a texture-backed button plate. Artists may call it a sprite, button plate, panel, frame, or slice. In Unreal/Slate terms it becomes an FSlateBrush or style resource, usually backed by a Texture2D imported from a transparent PNG, often packed into an atlas by the engine/runtime pipeline. The text should generally stay live text, not be baked into the image, unless the label is a logo or special title treatment.

For authored UI with bevels, corners, outlines, and small edge details, the standard scaling method is not raw image stretching. It is fixed-size, 3-slice, or 9-slice/nine-patch scaling. The corner pixels stay unscaled, horizontal edges stretch only horizontally, vertical edges stretch only vertically, and the center stretches or tiles. In Unreal Slate this maps to box/border-style brushes with margins, rather than an Image brush stretched across arbitrary dimensions.

Megabonk-like menu buttons are most likely a shared button-family asset reused at several widths through slicing, with separate variants only where shape, decoration, or importance differs. The central Play/Unlocks/Quests/Shop stack could be one sliced button plate with different layout sizes. If a button has unique silhouette art, asymmetric bevels, stickers, cracks, bolts, or a very particular width-dependent composition, it should be a separate authored variant or a fixed-size element.

The old imagegen-chrome approach likely failed because it treated UI chrome like a screen-resolution illustration: generated at one display size, then stretched or resampled into different widget sizes/resolutions. That damages bevels, outlines, corners, noise, shadows, and any baked text. The fix is not just generating at the exact target resolution; the fix is modular UI assets with declared scaling rules.

For a future UI/FriendslopUI pipeline, the safe model is:

1. Author source files in layered art tools, then export transparent PNG runtime assets.
2. Define each element as fixed, 3-slice, 9-slice, tiled, or bespoke.
3. Keep live Slate/UMG text on top of the art.
4. Store normal/hover/pressed/disabled variants or use controlled material/tint overlays.
5. Record intended display size, minimum size, slice margins, max resize range, and DPI behavior in a manifest.
6. Test every element at common target resolutions before accepting it.

For T66 specifically, FriendslopUI should not revive the old "generate whole screen chrome" pipeline. It should be an alternative style system to FlatStyle where the chrome can be texture-authored, but each texture has objective rules for how it may be used.
