# T66 UI Reconstruction Locked Process

This file is the only authoritative process for T66 UI reconstruction. If any older skill, handoff, prompt, note, script, or workflow conflicts with this file, this file wins.

Workers must perform only the assigned part. Do not perform earlier or later parts unless the task explicitly says to do so. Stop and report blockers instead of inventing substitute artifacts, alternate workflows, or extra diagnostics.

## Universal Rules

- Do only the assigned part: Part 0, Part 1, or Part 2.
- Do not create unrequested diagnostics, diff heatmaps, comparison grids, review boards, or extra helper images. Part 0 proof sheets are allowed because they are required library deliverables.
- Do not create per-screen sprite sheets. Reusable chrome comes only from the master UI asset library.
- Do not use any full-screen reference image as a runtime background.
- Runtime text, scores, names, values, timers, avatars, portraits, inventory, offers, selections, and live state stay runtime-owned widgets/data.
- Use the locked retro pixel font from the master UI asset library: Jersey 10 at `C:\UE\T66\RuntimeDependencies\T66\Fonts\Jersey10-Regular.ttf`. Do not choose substitute fonts.
- Button labels must use the Main Menu CTA text-fit approach instead of tiny fixed labels. The caller's explicit `FontSize` is the intended maximum size; do not force short labels larger just because the button is tall. If no font size is provided, use a height-derived fallback around `0.42 * button height`, cap the resolved size around `0.52 * button height`, use `LetterSpacing = 0..1`, center the text inside the plate with minimal content padding, and protect long localized labels with ellipsis plus a down-only `SScaleBox`. Do not solve fit by dropping button labels to 10-12px on 34-58px plates.
- Stop on missing inputs: no current screenshot, missing approved reference, missing library asset, missing route, ambiguous ownership, or deprecated target.
- Do not pull from archive folders.
- Do not manually pixel-repair generated references or runtime captures with clone, paint, erase, fill, mask, or heatmap-driven edits.
- Packaged screenshots are validation evidence only. They are not references and not source art.

## Top Bar Rule

The global top bar is allowed only on `main_menu` and the explicitly listed top-bar child screens.

Top-bar child screens:

- `account_status`
- `achievements`
- `settings`
- `language_select`
- `minigames`
- `powerup`

Do not add the global top bar to hero selection, companion selection, save/load flow, loading screen, run screens, pause menu, casino screens, overlays, modals, HUD, mini-game screens, TD screens, or any screen not listed above.

## Deprecated And Deferred Targets

Never implement these unless a later task explicitly reinstates one by slug:

- `wheel_overlay`
- `party_size_picker`
- `casino_overlay`
- `gambler_overlay`
- `vendor_overlay`
- standalone `leaderboard`
- `shop`
- `unlocks`
- legacy `Lobby`
- legacy `LobbyReadyCheck`
- legacy `LobbyBackConfirm`
- legacy `HeroLore`
- legacy `CompanionLore`
- all `mini_*` screens
- all `td_*` screens
- `whack_a_mole_arcade`
- `arcade_popup`
- HUD/runtime overlays unless explicitly assigned

Use `powerup`, not `shop`. Use `minigames`, not `unlocks`. Use `casino_gambling_tab` and `casino_vendor_tab`, not standalone gambler/vendor screens.

## Part 0: Master UI Asset Library

Purpose: create the single reusable visual system used by all later references and runtime screens.

Inputs:

- Approved main-menu reference: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Current screenshots only when needed for component inventory.
- Locked retro pixel font.

Allowed outputs:

- Master UI asset library under `C:\UE\T66\SourceAssets\UI\MasterLibrary\`
- Font file and font usage notes.
- Component catalog with asset names, states, dimensions, nine-slice margins, and intended use.
- Preview/proof sheets for the master library only. These are inspection artifacts, never runtime art.

Required asset construction:

- New or replacement UI chrome for the master asset library must originate from Codex-native image generation using the approved reference image as the visual source.
- Hard ban: do not use Pillow/PIL for master UI asset-library chrome. This includes scripts, Python helpers, alpha cleanup, matte removal, resizing, cropping, slicing, proofing, or any "minor" repair.
- Do not use procedural generators, Slate primitives, fill rectangles, crop-patch repainting, or scripted wood/metal/paper synthesis to invent or repaint buttons, panels, tabs, slots, bars, paper, dropdowns, select controls, top bars, or any other library UI element.
- Local scripts may only run after an approved imagegen board exists, and only for rectangular slicing, deterministic resizing that preserves pixels, copying, naming, manifest writing, alpha validation, and replacing already-background pixels outside the component silhouette with transparent black.
- Local scripts must not create the visual design, despill, edge-clean, crop into the silhouette, recolor, sharpen, blur, repaint, repair corners, synthesize texture, or locally pixel-repair master-library UI chrome. If generated component pixels look bad, regenerate with imagegen from the approved reference.
- Chroma/matte safety is mandatory: any removed background pixels must be alpha 0 with RGB 0,0,0. Do not leave green, magenta, checkerboard gray, white, paper-colored, or any other visible RGB in transparent pixels because Unreal texture filtering and mips can bleed that color around organic button edges.
- Chrome assets must be reference-derived from the approved main-menu reference wherever possible.
- Buttons, panels, top-bar pieces, tabs, fields, slots, rows, and dividers must be runtime-safe 9-slice PNGs with transparent outside pixels and catalogued margins. Rounded, oval, chamfered, or otherwise non-rectangular controls must validate as alpha-bearing PNGs with transparent corners; opaque rectangular crops are a hard rejection.
- The outer rim, bevel, glow, corner language, and palette must come from the approved main-menu reference. Do not ship procedural-placeholder chrome.
- Visual chrome and decorative art must be bitmap assets generated or approved in Part 0. Do not approximate art with C++/Slate primitive drawing, manual fill rectangles, ad hoc borders, or coded glow/shape effects.
- If direct crops produce uneven borders or baked content contamination, reconstruct the chrome from the sampled reference palette and shape language, then prove it with 9-slice resize sheets.
- Repaint or reconstruct live-content interiors so labels, values, portraits, icons, names, scores, and inventory content are not baked into chrome.
- Button hover, pressed, selected, and disabled fills must follow the exact rounded/chamfered component shape. Do not place rectangular bars inside rounded buttons.
- Button text renders directly on the generated button surface. Do not add a separate rectangular text plate, label backing, or brown square under text.
- Non-square controls must be their actual organic silhouette, not a rounded control inside a visible square backing. Verify transparent corners and outside pixels in-game against both dark and bright backgrounds before accepting the slice.
- Reference style is locked for this main-menu wood family: basic buttons are rectangular dark-mahogany planks with thin antique-gold bevels and black pixel outlines, not gold capsules, pointy chevrons, or arrow-ended controls. Invite and offline use the small rounded pill style from the reference with centered text. Hero-selection buttons must reuse the same library family unless a new approved imagegen family exists.
- Panel semantics are locked: basic panels include a dark mahogany fill plus border, not a border-only frame. Paper background is an inner content material only; it starts where the reference paper starts and must not replace the whole left/right shell. Profile/avatar slots sit on the left of friend rows; party slots are dark centered square slots with centered plus/avatar content.
- Panel borders must be uniform in thickness and brightness around all edges unless the approved reference clearly requires an intentional asymmetry.
- Icons may use image generation, but they must live in their own `IconGeneration\` atlas and `Slices\IconsGenerated\` transparent slices.
- Image-generated icon sheets must match the actual assigned screen icon inventory. Do not generate generic extras that do not appear in the target screen or assigned shared chrome.
- Image-generated icon sheets must contain no labels, no button frames, and must be converted to transparent per-icon runtime slices. Icon-internal symbols that are part of the actual UI, such as the translation `A` plus glyph mark, are allowed.

Required component families:

- Buttons: `normal`, `hover`, `pressed`, `disabled`, and `selected` where applicable.
- Panels and modal frames.
- Borders, dividers, and line accents.
- Tabs and segmented controls.
- Slot frames and icon wells.
- Scroll rails and thumbs.
- Top-bar assets only for `main_menu` and the allowed top-bar child screens.

Forbidden in Part 0:

- Runtime code changes.
- Screen-specific sprite sheets.
- Screen references for non-main-menu screens.
- Diff overlays, heatmaps, review boards, or packaged comparisons.
- Purely procedural approximations of the reference style.
- Any Pillow/PIL use in the master asset-library chrome pipeline, including cleanup, slicing, resizing, alpha work, proofing, or small repairs.
- Script-authored UI chrome for the master asset library. Scripts may only perform the limited mechanical operations listed above after imagegen produces an approved board.
- Despill, edge cleanup, color mutation, repainting, or local pixel repair on master-library UI chrome. Alpha work is allowed only outside the component silhouette, must preserve generated component pixels unchanged, and must write transparent pixels as alpha 0 with RGB 0,0,0.
- Rounded or organic buttons exported as visible square/rectangular sprites, including hidden backing layers that appear at the corners during runtime scaling.

Part 0 is done only when the repo has a reusable UI asset library that future references and runtime screens must use.

## Part 1: Reference Image Generation

Purpose: generate one approved visual target per screen using the master UI asset library.

Inputs:

- Active, non-deprecated screen slug.
- Current screenshot for that screen.
- Part 0 master UI asset library.
- Screen ownership notes.

Allowed output:

`C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png`

Optional output:

`C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_notes.md`

Forbidden in Part 1:

- Sprite sheets.
- Runtime code changes.
- Diff overlays.
- Heatmaps.
- Alternate style variants.
- Extra review images.
- New component styles not present in the master UI asset library.

Part 1 is done only when the screen has exactly one canonical 1920x1080 reference image generated from the locked library, with runtime-owned content still identified.

## Part 2: Screen Building

Purpose: implement the real Unreal UI to match the approved reference.

Inputs:

- Approved Part 1 screen reference.
- Part 0 master UI asset library.
- Existing Unreal screen/widget code.
- Runtime ownership notes.

Rules:

- Build real Slate/UMG widgets.
- Use live `FText`, runtime values, real buttons, real anchors, and responsive layout.
- Use only the locked master UI asset library for chrome.
- Production buttons, custom clickable areas, panels, tabs, slots, rows, and top-bar controls must use the project shared UI construction helpers whenever an appropriate helper exists (`FT66Style`, `T66OverlayChromeStyle`, `T66MiniUI`, `T66MiniGeneratedChrome`, or an approved screen-family helper). Do not add raw `SButton` or production chrome `SBorder` directly in screen code unless the task first adds or extends a shared helper. Helpers must preserve the locked master UI asset library, live text/data, text-fit policy, enabled/disabled behavior, and cursor policy.
- Do not create new visual art in code. Slate/UMG may place, scale, clip, tint-disabled, or nine-slice locked assets, but must not draw substitute chrome, decorative borders, filled patches, rays, glows, or icon art.
- Do not generate new art unless the task explicitly returns to Part 0.
- Do not add top bar outside the allowed top-bar list.
- Do not implement deprecated or deferred targets.

Measured sizing gate:

- Before manual tuning, measure the approved reference for parent shells, major controls, text boxes, icon boxes, and repeated gaps.
- Convert measured boxes into constraints: parent rects, child rects, padding, width, height, and spacing. Reconcile rows by adding all widths, gaps, and margins; do not guess by visually nudging one widget at a time.
- For text and icons, measure the reference content box as a percentage of its control box. Set initial font and icon sizes from that ratio.
- After a runtime capture, compute correction factors only from measured deltas: `next_size = current_size * reference_content_px / current_content_px`.
- Manual fine tuning is allowed only after parent/child box sizes and positions are already in the same range as the reference.
- Text or JSON measurement tables are allowed Part 2 evidence. Do not create image diff overlays, heatmaps, comparison boards, or extra visual diagnostics unless explicitly requested.

Validation:

- Compile or focused-build where feasible.
- Capture packaged runtime when feasible.
- Treat captures as validation evidence only.
- Do not create diff overlays or review boards unless the task explicitly asks for a review pass.

Part 2 is done only when the packaged runtime screen visually matches the approved reference using live widgets/data and the locked UI asset library.

## Worker Prompt Shape

Use short worker prompts:

```text
You own these screens: <screen list>.
We are on Part <0/1/2> of C:\UE\T66\Docs\UI\UI_RECONSTRUCTION_LOCKED_PROCESS.md.
Follow that file exactly.
Do not perform any other part.
Do not create diagnostics, diff heatmaps, extra references, or per-screen sprite sheets.
Stop and report blockers instead of improvising.
At completion, describe step by step what you did and cite each output path.
```
