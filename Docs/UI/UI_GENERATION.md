# T66 UI Generation

This is the authoritative process for generating and implementing T66 UI from reference images.

The current UI approach is intentionally screen-owned. Each screen or modal gets its own runtime asset folder so separate Codex chats can work independently without sharing mutable component art.

## Core Goal

Use image generation to create text-free raster UI chrome that matches the target reference image, then implement the screen with live Slate/UMG widgets and explicit resize contracts.

Generated images are for UI presentation only: panels, buttons, fields, slots, tabs, scrollbars, icons, dividers, meters, and decorative chrome. They must not bake live player data, names, stats, scores, dates, balances, save metadata, selection state, or localized labels into runtime art.

## Active Folder Layout

Runtime assets:

```text
C:\UE\T66\SourceAssets\UI\Reference\Screens\<ScreenName>\
C:\UE\T66\SourceAssets\UI\Reference\Modals\<ModalName>\
C:\UE\T66\SourceAssets\UI\Reference\Shared\
```

Handoff prompts:

```text
C:\UE\T66\UI\Reference\Screens\<ScreenName>\PROMPT.md
C:\UE\T66\UI\Reference\Modals\<ModalName>\PROMPT.md
C:\UE\T66\UI\Reference\PROMPT_INDEX.md
```

Reference images:

```text
C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\
C:\UE\T66\UI\screens\<screen_slug>\reference\
```

`SourceAssets\UI\Reference\Shared` is a bootstrap and compatibility source, not the destination for newly accepted screen-specific work. Before a target is considered individualized, any asset it uses from Shared or from another screen/modal must be duplicated into that target's folder and renamed for that target.

## Chat Output Rules

- Do not embed generated imagegen outputs directly in chat unless the user explicitly asks to see inline previews.
- When reporting generated images, proof captures, or component candidates, provide local file links/paths instead of Markdown image embeds.
- If the user asks to see images, send links first. Embed inline only when explicitly requested.
- Keep imagegen originals in the generator output folder and copy accepted project-bound assets into the target runtime asset folder.

## Phase 1: Inventory The Target

For each screen/modal chat:

1. Open the target `PROMPT.md` from `UI\Reference`.
2. Inspect the reference image and current packaged capture.
3. Inventory visible UI families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, modal shells, and preview frames.
4. Identify which families are missing or visibly mismatched.
5. Confirm which labels/data must remain live.

Do not work across multiple screens unless the user explicitly requests a combined pass.

## Phase 2: Generate Sprite Sheets And Components

Use image generation for missing or corrected component art.

Rules:

- Sprite sheets must be text-free.
- Direct per-component PNGs are preferred when a component needs transparency, clean alpha, or separate states.
- Tight transparent component PNGs must not include screenshot borders, checkerboards, matte colors, or neighboring UI.
- Transparent outside pixels should be alpha 0 and RGB 0,0,0 to avoid filtering bleed.
- Export required control states: `normal`, `hover`, `pressed`, `disabled`, and `selected` where applicable.
- Bad generated art routes back to image generation, not local pixel repair.
- Do not manually paint, clone, mask, repair, sharpen, blur, or recolor generated pixels with Pillow/PIL, Photoshop-style scripts, or local image operations.

## Phase 3: Runtime Resize Contracts

Every runtime component must declare how it scales.

Common contracts:

- Fixed image: icons, badges, portraits, small ornaments, and square slot frames that should preserve authored aspect.
- Horizontal 3-slice: long buttons, tabs, pills, and compact selectors. Preserve caps and stretch only the center strip.
- Vertical 3-slice: vertical scroll thumbs and rails where top/bottom caps must stay fixed.
- 9-slice: rectangular panels, fields, dropdown shells, and frames.
- Tiled or fill interior: only for assets authored as repeatable paper, wood, metal, or panel interiors.

Button contract:

- Non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, and live text over the plate.
- Use `T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth` or an equivalent minimum-width clamp so labels do not squash caps or collapse button centers.
- Do not draw generated button plates as ordinary stretched `SImage`, `SBorder`, or default `FButtonStyle` box art.

Panel contract:

- Panels and dropdown shells must use 9-slice or an equivalent custom renderer.
- If corners, borders, or caps stretch, the resize contract is wrong.

## Phase 4: Unreal Implementation

Implementation must use real Slate/UMG widgets and live data.

Rules:

- Route accepted target assets from that target's own folder under `SourceAssets\UI\Reference\Screens` or `SourceAssets\UI\Reference\Modals`.
- If another screen/modal has the closest current asset, duplicate it into the current target folder and rename it before use.
- Shared helpers are allowed for behavior and resize contracts. Shared mutable art is not the final state for a screen-owned pass.
- Runtime code may place, scale, slice, tint disabled states, clip, and draw generated assets according to their contract.
- Runtime code must not replace missing chrome with ad hoc fill rectangles, primitive borders, or procedural substitutes unless it is a temporary fallback clearly recorded in the difference list.
- Stage any new runtime assets so packaged builds load the real files.
- If a packaged capture shows a white rectangle or missing texture, first check staging/runtime dependency paths.

Current helper paths:

- `T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton` is the shared sliced-button renderer.
- `T66ScreenSlateHelpers::MakeReferenceChromeButtonAssetPath` is the compatibility button path helper.
- `T66ScreenSlateHelpers::MakeReferenceSharedBorder` and `GetReferenceSharedBrush` are compatibility bootstrap helpers; screen chats should migrate target-owned assets out of Shared when touching that UI.
- `Source/T66/T66.Build.cs` stages `SourceAssets/UI/Reference/...` as loose runtime UI assets.

## Phase 5: Visual Iteration Loop

After every meaningful runtime change:

1. Build or stage the relevant target.
2. Capture the packaged runtime screen.
3. Compare the packaged capture against the reference image.
4. Write a concrete difference list.
5. Classify each difference as `asset`, `resize-contract`, `layout`, `text-fit`, `staging`, or `live-data`.
6. Fix the highest-impact difference first.
7. Repeat until the visible difference list is empty or a concrete blocker is recorded.

Do not say the screen is done while visible differences remain. Do not rely on compile success as visual proof.

## Hard Bans

- No manual pixel repair with Pillow/PIL or equivalent tools.
- No local repainting, despill, clone, blur, sharpen, matte replacement, or silhouette repair.
- No full-screen reference image as runtime UI.
- No invisible hotspots replacing real buttons.
- No completion claim without packaged screenshot comparison.
- No inline chat image output unless the user explicitly asks for it.
- No new active generated-version runtime folders. Promote accepted art into the target's `Reference\Screens` or `Reference\Modals` folder.

## Next-Agent Prompt Shape

Use the generated files in `C:\UE\T66\UI\Reference\PROMPT_INDEX.md`. Each prompt begins with the target name and includes the reference image, source files, runtime asset folder, output rules, and iteration checklist.
